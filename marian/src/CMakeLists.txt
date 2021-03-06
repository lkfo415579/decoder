add_subdirectory(3rd_party)

include_directories(.)
include_directories(3rd_party)

cuda_add_library(marian
  3rd_party/cnpy/cnpy.cpp
  3rd_party/exception.cpp
  3rd_party/svd/svd.cpp
  graph/expression_graph.cu
  graph/expression_operators.cu
  graph/node.cu
  graph/node_operators.cu
  tensors/tensor.cu
  tensors/device_gpu.cu
  tensors/memory_piece.cu
  kernels/tensor_operators.cu
  kernels/dropout.cu
  kernels/cudnn_wrappers.cu
#  kernels/sparse.cu
  layers/param_initializers.cu
  layers/generic.cpp
  layers/guided_alignment.cpp
  layers/convolution.cu
  models/model_factory.cpp
  rnn/attention.cu
  rnn/cells.cu
  optimizers/clippers.cu
  optimizers/optimizers.cu
  common/utils.cpp
  common/logging.cpp
  common/config.cpp
  common/config_parser.cpp
  translator/history.cpp
  translator/output_collector.cpp
  translator/nth_element.cu
  translator/helpers.cu
  translator/scorers.cpp
  training/graph_group_async.cu
  training/graph_group_sync.cu
  training/graph_group_singleton.cu
  training/validator.cpp
  data/vocab.cpp
  data/corpus.cpp
  data/text_input.cpp
  rescorer/score_collector.cpp
  $<TARGET_OBJECTS:libyaml-cpp>
  STATIC
)

set_target_properties(marian PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(marian PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_executable(marian_train command/marian.cpp)
set_target_properties(marian_train PROPERTIES OUTPUT_NAME marian)

add_executable(marian_decoder command/marian_decoder.cpp)
set_target_properties(marian_decoder PROPERTIES OUTPUT_NAME marian-decoder)

add_executable(marian_scorer command/marian_scorer.cpp)
set_target_properties(marian_scorer PROPERTIES OUTPUT_NAME marian-scorer)

add_executable(marian_vocab command/marian_vocab.cpp)
set_target_properties(marian_vocab PROPERTIES OUTPUT_NAME marian-vocab)

#test
#test EXECUTABLES
add_executable(marian_test python/marian_test.cpp)
set_target_properties(marian_test PROPERTIES OUTPUT_NAME marian-test)
#set(EXECUTABLES ${EXECUTABLES} marian_test)
#tests

set(EXECUTABLES ${EXECUTABLES} marian_train marian_decoder marian_scorer marian_vocab marian_test)

if(COMPILE_SERVER)
  add_executable(marian_server command/marian_server.cpp)
  set_target_properties(marian_server PROPERTIES OUTPUT_NAME marian-server)
  set(EXECUTABLES ${EXECUTABLES} marian_server)
endif(COMPILE_SERVER)

foreach(exec ${EXECUTABLES})
  target_link_libraries(${exec} marian ${EXT_LIBS})
  cuda_add_cublas_to_target(${exec})
  set_target_properties(${exec} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endforeach(exec)

#add_executable(
#  align2steps
#  tools/align2steps.cpp
#)

#set_target_properties(align2steps PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if(PYTHONLIBS_FOUND)
  # add_subdirectory(python)
  # cuda_add_library(pymarian SHARED
  #   python/mariannmt.cpp
  #   3rd_party/cnpy/cnpy.cpp
  #   3rd_party/exception.cpp
  #   3rd_party/svd/svd.cpp
  #   graph/expression_graph.cu
  #   graph/expression_operators.cu
  #   graph/node.cu
  #   graph/node_operators.cu
  #   tensors/tensor.cu
  #   tensors/device_gpu.cu
  #   kernels/tensor_operators.cu
  #   kernels/dropout.cu
  #   kernels/cudnn_wrappers.cu
  #   #kernels/sparse.cu
  #   #layers/param_initializers.cu
  #   rnn/attention.cu
  #   rnn/cells.cu
  #   #optimizers/clippers.cu
  #   #optimizers/optimizers.cu
  #   common/utils.cpp
  #   common/logging.cpp
  #   common/config.cpp
  #   common/config_parser.cpp
  #   translator/history.cpp
  #   translator/output_collector.cpp
  #   translator/nth_element.cu
  #   translator/helpers.cu
  #   data/vocab.cpp
  #   data/corpus.cpp
  #   data/text_input.cpp
  #   #rescorer/score_collector.cpp
  #   $<TARGET_OBJECTS:libyaml-cpp>
  # )
  # cuda_add_library(pymarian SHARED
  #   python/mariannmt.cpp
  #   3rd_party/cnpy/cnpy.cpp
  #   3rd_party/exception.cpp
  #   3rd_party/svd/svd.cpp
  #   graph/expression_graph.cu
  #   graph/expression_operators.cu
  #   graph/node.cu
  #   graph/node_operators.cu
  #   tensors/tensor.cu
  #   tensors/device_gpu.cu
  #   tensors/memory_piece.cu
  #   kernels/tensor_operators.cu
  #   kernels/dropout.cu
  #   kernels/cudnn_wrappers.cu
  # #  kernels/sparse.cu
  #   layers/param_initializers.cu
  #   layers/generic.cpp
  #   layers/guided_alignment.cpp
  #   layers/convolution.cu
  #   models/model_factory.cpp
  #   rnn/attention.cu
  #   rnn/cells.cu
  #   optimizers/clippers.cu
  #   optimizers/optimizers.cu
  #   common/utils.cpp
  #   common/logging.cpp
  #   common/config.cpp
  #   common/config_parser.cpp
  #   translator/history.cpp
  #   translator/output_collector.cpp
  #   translator/nth_element.cu
  #   translator/helpers.cu
  #   translator/scorers.cpp
  #   training/graph_group_async.cu
  #   training/graph_group_sync.cu
  #   training/graph_group_singleton.cu
  #   training/validator.cpp
  #   data/vocab.cpp
  #   data/corpus.cpp
  #   data/text_input.cpp
  #   rescorer/score_collector.cpp
  #   $<TARGET_OBJECTS:libyaml-cpp>
  # )
  add_subdirectory(python)
  # set_target_properties(pymarian PROPERTIES EXCLUDE_FROM_ALL 1)
  # set_target_properties(pymarian PROPERTIES OUTPUT_NAME mariannmt)
  # set_target_properties(pymarian PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
  # set_target_properties(pymarian PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
  #
  # target_link_libraries(pymarian ${EXT_LIBS} marian)
  #
  # cuda_add_cublas_to_target(pymarian)
endif(PYTHONLIBS_FOUND)

if(COMPILE_TESTS)
  set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party)
  add_library(Catch INTERFACE)
  target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

  add_subdirectory(tests)
endif(COMPILE_TESTS)

if(COMPILE_EXAMPLES)
  add_subdirectory(examples)
endif(COMPILE_EXAMPLES)
