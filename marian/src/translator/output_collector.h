#pragma once

#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <map>

#include "common/definitions.h"
#include "common/file_stream.h"

namespace marian {

class PrintingStrategy {
public:
  virtual bool shouldBePrinted(long) = 0;
};

class DefaultPrinting : public PrintingStrategy {
public:
  bool shouldBePrinted(long) { return true; }
};

class QuietPrinting : public PrintingStrategy {
public:
  bool shouldBePrinted(long) { return false; }
};

class GeometricPrinting : public PrintingStrategy {
public:
  bool shouldBePrinted(long id) {
    if(id == 0)
      next_ = start_;
    if(id <= 5)
      return true;
    if(next_ == id) {
      next_ += next_;
      return true;
    }
    return false;
  }

private:
  size_t start_{10};
  long next_{10};
};

class OutputCollector {
public:
  OutputCollector();

  template <class T>
  OutputCollector(T&& arg) : nextId_(0), outStrm_(new OutputFileStream(arg)) {}

  OutputCollector(const OutputCollector&) = delete;

  void Write(long sourceId,
             const std::string& best1,
             const std::string& bestn,
             bool nbest);

  void setPrintingStrategy(Ptr<PrintingStrategy> strategy) {
    printing_ = strategy;
  }

protected:
  UPtr<OutputFileStream> outStrm_;
  boost::mutex mutex_;
  long nextId_;

  typedef std::map<long, std::pair<std::string, std::string>> Outputs;
  Outputs outputs_;

  Ptr<PrintingStrategy> printing_;
};

class StringCollector {
public:
  StringCollector();
  StringCollector(const StringCollector&) = delete;

  void add(long sourceId, const std::string& best1, const std::string& bestn);
  std::vector<std::string> collect(bool nbest);

protected:
  long maxId_;
  boost::mutex mutex_;

  typedef std::map<long, std::pair<std::string, std::string>> Outputs;
  Outputs outputs_;
};
}
