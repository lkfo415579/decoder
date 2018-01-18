#include <cstdlib>
#include <iostream>
#include <string>

//#include <boost/python.hpp>
#include "marian.h"
#include "common/utils.h"
#include "common/version.h"
#include "translator/beam_search.h"
#include "translator/translator.h"

using namespace marian;

Ptr<TranslateServiceMultiGPU<BeamSearch>> task;
//Ptr<TranslateMultiGPU<BeamSearch>> task;

// void init(const std::string& argopts) {
//   auto options = New<Config>(argopts, ConfigMode::translating);
//   task = New<TranslateServiceMultiGPU<BeamSearch>>(options);
//   //task = New<TranslateMultiGPU<BeamSearch>>(options);
//   LOG(info, "Translator initialized");
// }

void translate(const std::vector<std::string> &input) {
  // for(auto input1 : input){
  //   std::cerr << "input2" << input1 << '\n';
  // }
  //std::vector<std::string> input;
  std::cerr << "Total sentences:"<< input.size() << std::endl;
  // for(int i = 0; i < boost::python::len(pyinput); ++i) {
  //   input.emplace_back(
  //       boost::python::extract<std::string>(boost::python::object(pyinput[i])));
  // }
  //std::cerr << "run" << std::endl;
  auto output = task->run(input);
  //std::cerr << "done" << std::endl;
  std::cerr << "size" << output.size() << std::endl;
  for (auto ele : output){
    std::cerr << "output:" << ele<< std::endl;
  }
  // boost::python::list pyoutput;
  // pyoutput.append(Join(output, "\n"));
  // return pyoutput;
  return;
}

std::string version() {
  return std::string(PROJECT_VERSION);
}
int main(int argc, char** argv) {
  auto options = New<Config>(argc, argv, ConfigMode::translating);
  task = New<TranslateServiceMultiGPU<BeamSearch>>(options);
  LOG(info, "Translator initialized");

  //reading file
  std::vector<UPtr<InputFileStream>> files_;
  files_.emplace_back(new InputFileStream(std::cin));
  std::vector<std::string> lines(files_.size());
  std::string tmp_str;
  bool cont = true;
  while(cont) {
    //std::vector<std::string> lines(files_.size());
    for(size_t i = 0; i < files_.size(); ++i) {
      //cont = cont && std::getline((std::istream&)*files_[i], lines[i]);
      cont = cont && std::getline((std::istream&)*files_[i], tmp_str);
      lines.emplace_back(tmp_str);
      //std::cerr << tmp_str << std::endl;
    }
  }
  //first and last is empty line
  //lines.erase(lines.begin());
  //lines.erase(lines.end());
  LOG(info, "finished reading data from file");

  // for(size_t i = 0; i < lines.size(); ++i) {
  //   std::cerr << "line:" << lines[i] << std::endl;
  // }

  //
  translate(lines);

  return 0;
}
