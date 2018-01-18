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
  const std::string argopts2("./decoder " + argopts);
  //std::cerr << "argops2:" << argopts2 << std::endl;
  auto options = New<Config>(argopts2, ConfigMode::translating);
  task = New<TranslateServiceMultiGPU<BeamSearch>>(options);
  //task = New<TranslateMultiGPU<BeamSearch>>(options);
  LOG(info, "Translator initialized");
}

boost::python::list translate_batch(boost::python::list& pyinput) {
  std::vector<std::string> input;
  //std::cerr << "fuck" << std::endl;
  for(int i = 0; i < boost::python::len(pyinput); ++i) {
    input.emplace_back(
        boost::python::extract<std::string>(boost::python::object(pyinput[i])));
  }
  //std::cerr << "run" << std::endl;
  auto outputs = task->run(input);
  //std::cerr << "done" << std::endl;
  //std::cerr << "output size" << outputs.size() << std::endl;
  // for (auto ele : output){
  //   std::cerr << "output:" << ele<< std::endl;
  // }

  boost::python::list pyoutput;

  for (auto& output : outputs){
    pyoutput.append(output);
  }
  //pyoutput.append(Join(output, "\n"));
  return pyoutput;
}

std::string version() {
  return std::string(PROJECT_VERSION);
}

BOOST_PYTHON_MODULE(libmariannmt) {
  boost::python::def("init", init);
  boost::python::def("translate_batch", translate_batch);
  boost::python::def("version", version);
}
