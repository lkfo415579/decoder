#pragma once

#include "data/batch_generator.h"
#include "data/corpus.h"
#include "data/text_input.h"

#include "3rd_party/threadpool.h"
#include "translator/history.h"
#include "translator/output_collector.h"
#include "translator/printer.h"

#include "models/model_task.h"
#include "translator/scorers.h"

namespace marian {

template <class Search>
class TranslateMultiGPU : public ModelTask {
private:
  Ptr<Config> options_;
  std::vector<Ptr<ExpressionGraph>> graphs_;
  std::vector<std::vector<Ptr<Scorer>>> scorers_;

  Ptr<data::Corpus> corpus_;
  Ptr<Vocab> trgVocab_;

public:
  TranslateMultiGPU(Ptr<Config> options)
      : options_(options),
        corpus_(New<data::Corpus>(options_, true)),
        trgVocab_(New<Vocab>()) {
    auto vocabs = options_->get<std::vector<std::string>>("vocabs");
    trgVocab_->load(vocabs.back());

    auto devices = options_->get<std::vector<int>>("devices");
    ThreadPool threadPool(devices.size(), devices.size());

    scorers_.resize(devices.size());
    graphs_.resize(devices.size());
    size_t id = 0;
    for(size_t device : devices) {
      auto task = [&](size_t device, size_t id) {
        auto graph = New<ExpressionGraph>(true);
        graph->setDevice(device);
        graph->reserveWorkspaceMB(options_->get<size_t>("workspace"));
        graphs_[id] = graph;

        auto scorers = createScorers(options_);
        for(auto scorer : scorers)
          scorer->init(graph);

        scorers_[id] = scorers;
      };

      threadPool.enqueue(task, device, id++);
    }
  }

  void run() {
    data::BatchGenerator<data::Corpus> bg(corpus_, options_);

    auto devices = options_->get<std::vector<int>>("devices");
    ThreadPool threadPool(devices.size(), devices.size());

    size_t batchId = 0;
    auto collector = New<OutputCollector>();
    if(options_->get<bool>("quiet-translation"))
        collector->setPrintingStrategy(New<QuietPrinting>());

    bg.prepare(false);

    while(bg) {
      auto batch = bg.next();

      auto task = [=](size_t id) {
        thread_local Ptr<ExpressionGraph> graph;
        thread_local std::vector<Ptr<Scorer>> scorers;

        if(!graph) {
          graph = graphs_[id % devices.size()];
          graph->getBackend()->setDevice(graph->getDevice());
          scorers = scorers_[id % devices.size()];
        }

        auto search = New<Search>(options_, scorers);
        auto histories = search->search(graph, batch);

        for(auto history : histories) {
          std::stringstream best1;
          std::stringstream bestn;
          Printer(options_, trgVocab_, history, best1, bestn);
          collector->Write(history->GetLineNum(),
                           best1.str(),
                           bestn.str(),
                           options_->get<bool>("n-best"));
        }
      };

      threadPool.enqueue(task, batchId++);
    }
  }
};

template <class Search>
class TranslateServiceMultiGPU : public ModelServiceTask {
private:
  Ptr<Config> options_;
  std::vector<Ptr<ExpressionGraph>> graphs_;
  std::vector<std::vector<Ptr<Scorer>>> scorers_;

  std::vector<size_t> devices_;
  std::vector<Ptr<Vocab>> srcVocabs_;
  Ptr<Vocab> trgVocab_;

public:
  virtual ~TranslateServiceMultiGPU() {}

  TranslateServiceMultiGPU(Ptr<Config> options)
      : options_(options),
        devices_(options_->get<std::vector<size_t>>("devices")),
        trgVocab_(New<Vocab>()) {
    init();
  }

  void init() {
    // initialize vocabs
    auto vocabPaths = options_->get<std::vector<std::string>>("vocabs");
    std::vector<int> maxVocabs = options_->get<std::vector<int>>("dim-vocabs");
    for(size_t i = 0; i < vocabPaths.size() - 1; ++i) {
      Ptr<Vocab> vocab = New<Vocab>();
      vocab->load(vocabPaths[i], maxVocabs[i]);
      srcVocabs_.emplace_back(vocab);
    }
    trgVocab_->load(vocabPaths.back());

    // initialize scorers
    for(auto& device : devices_) {
      auto graph = New<ExpressionGraph>(true);
      graph->setDevice(device);
      graph->reserveWorkspaceMB(options_->get<size_t>("workspace"));
      graphs_.push_back(graph);

      auto scorers = createScorers(options_);
      for(auto scorer : scorers)
        scorer->init(graph);
      scorers_.push_back(scorers);
    }
  }

  std::vector<std::string> run(const std::vector<std::string>& inputs) {
    // for(auto input : inputs){
    //   std::cerr << "input" << input << '\n';
    // }

    //std::cerr << "before bg" << std::endl;
    auto corpus_ = New<data::TextInput>(inputs, srcVocabs_, options_);
    data::BatchGenerator<data::TextInput> bg(corpus_, options_);
    //std::cerr << "after batch" << std::endl;
    //
    auto devices_ = options_->get<std::vector<int>>("devices");
    ThreadPool threadPool_(devices_.size(), devices_.size());
    //std::cerr << "devices" << std::endl;
    //
    size_t batchId = 0;
    auto collector = New<StringCollector>();

    bg.prepare(false);

    //std::cerr << "bg" << std::endl;
    std::vector<std::future< Histories >> results;
    while(bg) {
      auto batch = bg.next();
      //std::cerr << "in bg" << std::endl;
      int dimBatch = batch->size();
      //std::cerr << "batch size " << dimBatch << std::endl;

      auto task = [=](size_t id) ->Histories {
        thread_local Ptr<ExpressionGraph> graph;
        thread_local std::vector<Ptr<Scorer>> scorers;

        if(!graph) {
          graph = graphs_[id % devices_.size()];
          graph->getBackend()->setDevice(graph->getDevice());
          scorers = scorers_[id % devices_.size()];
        }

        auto search = New<Search>(options_, scorers);
        auto histories = search->search(graph, batch);
        std::cerr << "Translating batch :" << id << '\n';
        // for(auto history : histories) {
        //   std::stringstream best1;
        //   std::stringstream bestn;
        //   Printer(options_, trgVocab_, history, best1, bestn);
        //   //std::cerr <<"hey"<<'\n';
        //   //collector->add(history->GetLineNum(), best1.str(), bestn.str());
        // }
        //std::cerr << "histories_address:" << &histories << '\n';
        //std::shared_ptr<Histories> tmp_h{&histories};
        return histories;
      };
      //std::cerr << "185,task" << std::endl;
      //auto end = threadPool_.enqueue(task, batchId);
      //auto return_value = end.get();
      //std::cerr << "END:" << end.get() << '\n';
      results.emplace_back(threadPool_.enqueue(task, batchId));
      batchId++;
    }
    //got all data
    for (auto& result : results) {
      auto histories = result.get();
      for(auto history : histories) {
        std::stringstream best1;
        std::stringstream bestn;
        Printer(options_, trgVocab_, history, best1, bestn);
        collector->add(history->GetLineNum(), best1.str(), bestn.str());
      }
    }

    //threadPool_.~ThreadPool2();
    //threadPool_.waitFinished();
    // while(1){
    //   std::cerr << " waiting:" << threadPool_.getNumTasks()<< '\n';
    //   //continue;
    //   if (!threadPool_.getNumTasks())
    //     break;
    // }
    //int count = 0;


    // do {
    //   //std::cerr << "worker_size" << threadPool_.workers.size() << "\n";
    //   //if (devices_.size() == threadPool_.workers.size()) {
    //   if (!threadPool_.getNumTasks()){
    //     break;
    //   }
    //   //std::sleep(1);
    //   std::this_thread::sleep_for(std::chrono::milliseconds(20));
    // } while (1);
    // std::cerr << "worker_size" << threadPool_.workers.size() << "\n";
    // for (std::thread &worker: threadPool_.workers) {
    //   //std::cerr << "waiting" << tasks.size() << '\n';
    //   std::cerr << " waiting:" << threadPool_.getNumTasks()<< '\n';
    //   worker.join();
    // }
    //threadPool_.workers[0].join();

    //Test_output
    //std::cerr << " waited:" << threadPool_.getNumTasks()<< '\n';
    // auto output = collector->collect(options_->get<bool>("n-best"));
    // for (auto ele : output){
    //   std::cerr << "T:output:" << ele<< std::endl;
    // }
    //threadPool_.waitFinished();
    //threadPool_.~ThreadPool();
    //while()
    //return New<std::vector<std::string>>;
    return collector->collect(options_->get<bool>("n-best"));
  }
};
}