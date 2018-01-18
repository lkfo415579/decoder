#pragma once
#include <algorithm>

#include "marian.h"
#include "translator/history.h"
#include "translator/scorers.h"

#include "translator/helpers.h"
#include "translator/nth_element.h"

namespace marian {

class BeamSearch {
private:
  Ptr<Config> options_;
  std::vector<Ptr<Scorer>> scorers_;
  size_t beamSize_;

public:
  template <class... Args>
  BeamSearch(Ptr<Config> options,
             const std::vector<Ptr<Scorer>>& scorers,
             Args... args)
      : options_(options),
        scorers_(scorers),
        beamSize_(options_->has("beam-size")
                      ? options_->get<size_t>("beam-size")
                      : 3) {}

  Beams toHyps(const std::vector<uint> keys,
               const std::vector<float> costs,
               size_t vocabSize,
               const Beams& beams,
               std::vector<Ptr<ScorerState>>& states,
               size_t beamSize,
               bool first) {

    Beams newBeams(beams.size());
    for(int i = 0; i < keys.size(); ++i) {
      int embIdx  = keys[i] % vocabSize;
      int beamIdx  = i / beamSize;

      if(newBeams[beamIdx].size() < beams[beamIdx].size()) {
        auto& beam = beams[beamIdx];
        auto& newBeam = newBeams[beamIdx];

        int hypIdx = keys[i] / vocabSize;
        float cost  = costs[i];

        int hypIdxTrans = (hypIdx / beamSize) +
                          (hypIdx % beamSize) * beams.size();
        if(first)
          hypIdxTrans = hypIdx;

        int beamHypIdx = hypIdx % beamSize;
        if(beamHypIdx >= beam.size())
          beamHypIdx = beamHypIdx % beam.size();

        if(first)
          beamHypIdx = 0;

        auto hyp = New<Hypothesis>(beam[beamHypIdx], embIdx, hypIdxTrans, cost);
        if(options_->get<bool>("n-best")) {
          std::vector<float> breakDown(states.size(), 0);
          beam[beamHypIdx]->GetCostBreakdown().resize(states.size(), 0);
          for(int j = 0; j < states.size(); ++j) {
            int key = embIdx + hypIdxTrans * vocabSize;
            breakDown[j] = states[j]->breakDown(key)
                           + beam[beamHypIdx]->GetCostBreakdown()[j];
          }
          hyp->GetCostBreakdown() = breakDown;
        }
        newBeam.push_back(hyp);
      }
    }
    return newBeams;
  }

  Beams pruneBeam(const Beams& beams) {
    Beams newBeams;
    for(auto beam: beams) {
      Beam newBeam;
      for(auto hyp : beam) {
        if(hyp->GetWord() > 0) {
          newBeam.push_back(hyp);
        }
      }
      newBeams.push_back(newBeam);
    }
    return newBeams;
  }

  Histories search(Ptr<ExpressionGraph> graph,
                   Ptr<data::CorpusBatch> batch) {

    int dimBatch = batch->size();
    Histories histories;
    for(int i = 0; i < dimBatch; ++i) {
      size_t sentId = batch->getSentenceIds()[i];
      auto history = New<History>(sentId, options_->get<float>("normalize"));
      histories.push_back(history);
    }

    size_t localBeamSize = beamSize_;
    auto nth = New<NthElement>(localBeamSize, dimBatch);

    Beams beams(dimBatch);
    for(auto& beam : beams)
      beam.resize(localBeamSize, New<Hypothesis>());

    bool first = true;
    bool final = false;

    for(int i = 0; i < dimBatch; ++i)
      histories[i]->Add(beams[i]);

    std::vector<Ptr<ScorerState>> states;

    for(auto scorer : scorers_) {
      scorer->clear(graph);
    }

    for(auto scorer : scorers_) {
      states.push_back(scorer->startState(graph, batch));
    }

    do {
      //**********************************************************************
      // create constant containing previous costs for current beam
      std::vector<size_t> hypIndices;
      std::vector<size_t> embIndices;
      Expr prevCosts;
      if(first) {
        // no cost
        prevCosts = graph->constant({1, 1, 1, 1},
                                    keywords::init = inits::from_value(0));
      } else {
        std::vector<float> beamCosts;

        int dimBatch = batch->size();

        for(int i = 0; i < localBeamSize; ++i) {
          for(int j = 0; j < beams.size(); ++j) {
            auto& beam = beams[j];
            if(i < beam.size()) {
              auto hyp = beam[i];
              hypIndices.push_back(hyp->GetPrevStateIndex());
              embIndices.push_back(hyp->GetWord());
              beamCosts.push_back(hyp->GetCost());
            }
            else {
              hypIndices.push_back(0);
              embIndices.push_back(0);
              beamCosts.push_back(-9999);
            }
          }
        }

        prevCosts
            = graph->constant({(int)localBeamSize, 1, dimBatch, 1},
                              keywords::init = inits::from_vector(beamCosts));
      }

      //**********************************************************************
      // prepare costs for beam search
      auto totalCosts = prevCosts;

      for(int i = 0; i < scorers_.size(); ++i) {
        states[i] = scorers_[i]->step(graph, states[i], hypIndices, embIndices, localBeamSize);

        if(scorers_[i]->getWeight() != 1.f)
          totalCosts = totalCosts + scorers_[i]->getWeight() * states[i]->getProbs();
        else
          totalCosts = totalCosts + states[i]->getProbs();
      }

      // make beams continuous
      if(dimBatch > 1 && localBeamSize > 1)
        totalCosts = transpose(totalCosts, {2, 1, 0, 3});

      if(first)
        graph->forward();
      else
        graph->forwardNext();

      //**********************************************************************
      // suppress specific symbols if not at right positions
      if(options_->has("allow-unk") && !options_->get<bool>("allow-unk"))
        suppressUnk(totalCosts);
      for(auto state : states)
        state->blacklist(totalCosts, batch);

      //**********************************************************************
      // perform beam search and pruning
      std::vector<unsigned> outKeys;
      std::vector<float> outCosts;

      std::vector<size_t> beamSizes(dimBatch, localBeamSize);
      nth->getNBestList(beamSizes, totalCosts->val(), outCosts, outKeys, first);

      int dimTrgVoc = totalCosts->shape()[-1];
      beams = toHyps(outKeys, outCosts, dimTrgVoc, beams, states, localBeamSize, first);

      auto prunedBeams = pruneBeam(beams);
      for(int i = 0; i < dimBatch; ++i) {
        if(!beams[i].empty()) {
          final = final || histories[i]->size() >= 3 * batch->front()->batchWidth();
          histories[i]->Add(beams[i], prunedBeams[i].empty() || final);
        }
      }
      beams = prunedBeams;

      if(!first) {
        size_t maxBeam = 0;
        for(auto& beam : beams)
          if(beam.size() > maxBeam)
            maxBeam = beam.size();
        localBeamSize = maxBeam;
      }
      first = false;

    } while(localBeamSize != 0 && !final);

    return histories;
  }
};
}
