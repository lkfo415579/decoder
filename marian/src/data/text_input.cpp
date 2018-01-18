#include "data/text_input.h"

namespace marian {
namespace data {

TextIterator::TextIterator() : pos_(-1), tup_(0) {}

TextIterator::TextIterator(TextInput& corpus)
    : corpus_(&corpus), pos_(0), tup_(corpus_->next()) {}

void TextIterator::increment() {
  tup_ = corpus_->next();
  pos_++;
}

bool TextIterator::equal(TextIterator const& other) const {
  return this->pos_ == other.pos_ || (this->tup_.empty() && other.tup_.empty());
}

const SentenceTuple& TextIterator::dereference() const {
  return tup_;
}

TextInput::TextInput(std::vector<std::string> paths,
                     std::vector<Ptr<Vocab>> vocabs,
                     Ptr<Config> options)
    : DatasetBase(paths), vocabs_(vocabs), options_(options) {

  //size_t count = 0;
  for(auto path : paths_){
    files_.emplace_back(new std::istringstream(path));
    //std::cerr << "count: " << count++ << '\n';
    // std::cerr << "path: " << path << '\n';
    // std::cerr << std::istringstream(path).str() << '\n';
  }
    //files_.emplace_back(path);
}



SentenceTuple TextInput::next() {
  bool cont = true;
  while(cont) {
    // get index of the current sentence
    size_t curId = pos_++;
    //over all sentences
    if (curId >= files_.size())
      break;

    // fill up the sentence tuple with sentences from all input files
    SentenceTuple tup(curId);
    std::string line;
    //if(std::getline((std::istream&)*files_[curId], line)) {
    if(std::getline(*files_[curId], line)) {
      //std::cerr << "LINE:" << line << '\n';
      Words words = (*vocabs_[0])(line);
      if(words.empty())
        words.push_back(0);
      tup.push_back(words);
      //readable++;
      //cont = 1;
      //std::cerr << "curId:" << curId << std::endl;
      return tup;
    }

    //
    /*size_t readable = 0;
    for(size_t i = 0; i < files_.size(); ++i) {
      std::string line;
      if(std::getline((std::istream&)*files_[i], line)) {
        //
        //std::cerr << "input.cpp,getline:" << line << std::endl;
        //Words words = (*vocabs_[i])(line);
        Words words = (*vocabs_[0])(line);
        // std::cerr << "words size:" << words.size() << std::endl;
        // for (auto word : words){
        //     std::cerr << "words:" << word << std::endl;
        // }

        if(words.empty())
          words.push_back(0);
        tup.push_back(words);
        readable++;
      }
    }*/

    // continue only if each input file has provided an example
    //cont = tup.size() == files_.size();
    //cont = tup.size() == readable;
    //std::cerr << "cont:" << cont << " " << tup.size()<<" "<< files_.size()<< '\n';
    //if(cont)
    //  return tup;
  }
  return SentenceTuple(0);
}
}
}
