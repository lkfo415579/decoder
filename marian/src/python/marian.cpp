#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/python.hpp>

#include "common/utils.h"
#include "common/version.h"
#include "translator/beam_search.h"
#include "translator/translator.h"

using namespace marian;

Ptr<TranslateServiceMultiGPU<BeamSearch>> task;
//Ptr<TranslateMultiGPU<BeamSearch>> task;

void init(const std::string& argopts) {
  auto options = New<Config>(argopts, ConfigMode::translating);
  task = New<TranslateServiceMultiGPU<BeamSearch>>(options);
  //task = New<TranslateMultiGPU<BeamSearch>>(options);
  LOG(info, "Translator initialized");
}

boost::python::list translate(boost::python::list& pyinput) {
  std::vector<std::string> input;
  std::cerr << "fuck" << std::endl;
  for(int i = 0; i < boost::python::len(pyinput); ++i) {
    input.emplace_back(
        boost::python::extract<std::string>(boost::python::object(pyinput[i])));
  }
  std::cerr << "run" << std::endl;
  auto output = task->run(input);
  std::cerr << "done" << std::endl;

  boost::python::list pyoutput;
  pyoutput.append(Join(output, "\n"));
  return pyoutput;
}

std::string version() {
  return std::string(PROJECT_VERSION);
}
