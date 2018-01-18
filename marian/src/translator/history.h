#pragma once

#include <queue>

#include "hypothesis.h"

namespace marian {

class History {
private:
  struct HypothesisCoord {
    bool operator<(const HypothesisCoord& hc) const { return cost < hc.cost; }

    size_t i;
    size_t j;
    float cost;
  };

public:
  History(size_t lineNo, float alpha = 1.f);

  float LengthPenalty(size_t length) { return std::pow((float)length, alpha_); }

  void Add(const Beam& beam, bool last = false) {
    if(beam.back()->GetPrevHyp() != nullptr) {
      for(size_t j = 0; j < beam.size(); ++j)
        if(beam[j]->GetWord() == 0 || last) {
          float cost = beam[j]->GetCost() / LengthPenalty(history_.size());
          topHyps_.push({history_.size(), j, cost});
          //std::cerr << "Add " << history_.size() << " " << j << " " << cost << std::endl;
        }
    }
    history_.push_back(beam);
  }

  size_t size() const { return history_.size(); }

  NBestList NBest(size_t n) const {
    NBestList nbest;
    auto topHypsCopy = topHyps_;
    while(nbest.size() < n && !topHypsCopy.empty()) {
      auto bestHypCoord = topHypsCopy.top();
      topHypsCopy.pop();

      size_t start = bestHypCoord.i;
      size_t j = bestHypCoord.j;
      //float c = bestHypCoord.cost;
      //std::cerr << "h: " << start << " " << j << " " << c << std::endl;

      Words targetWords;
      Ptr<Hypothesis> bestHyp = history_[start][j];
      while(bestHyp->GetPrevHyp() != nullptr) {
        targetWords.push_back(bestHyp->GetWord());
        //std::cerr << bestHyp->GetWord() << " " << bestHyp << std::endl;
        bestHyp = bestHyp->GetPrevHyp();
      }

      std::reverse(targetWords.begin(), targetWords.end());
      nbest.emplace_back(targetWords,
                         history_[bestHypCoord.i][bestHypCoord.j],
                         bestHypCoord.cost);
    }
    return nbest;
  }

  Result Top() const { return NBest(1)[0]; }

  size_t GetLineNum() const { return lineNo_; }

private:
  std::vector<Beam> history_;
  std::priority_queue<HypothesisCoord> topHyps_;
  size_t lineNo_;
  float alpha_;
};

typedef std::vector<Ptr<History>> Histories;
}
