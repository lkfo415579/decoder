# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.1] - 2017-11-30

### Added
- Option --max-length-crop to be used together with --max-length N to crop 
sentences to length N rather than omitting them.
- Experimental model with convolution over input characters

### Changed
- Fixed a number of bugs for vocabulary and directory handling

## [1.1.0] - 2017-11-21

### Added
- Batched translation for all model types, significant translation speed-up
- Batched translation during validation with translation
- `--maxi-batch-sort` option for `marian-decoder`
- Support for CUBLAS_TENSOR_OP_MATH mode for cublas in cuda 9.0
- The "marian-vocab" tool to create vocabularies

## [1.0.0] - 2017-11-13

### Added
- Multi-gpu validation, scorer and in-training translation
- summary-mode for scorer
- New "transformer" model based on [Attention is all you
  need](https://arxiv.org/abs/1706.03762)
- Options specific for the transformer model
- Linear learning rate warmup with and without initial value
- Cyclic learning rate warmup
- More options for learning rate decay, including: optimizer history reset,
  repeated warmup
- Continuous inverted square root decay of learning (`--lr-decay-inv-sqrt`) rate
  based on number of updates
- Exposed optimizer parameters (e.g. momentum etc. for Adam)
- Version of deep RNN-based models compatible with Nematus (`--type nematus`)
- Synchronous SGD training for multi-gpu (enable with `--sync-sgd`)
- Dynamic construction of complex models with different encoders and decoders,
  currently only available through the C++ API
- Option `--quiet` to suppress output to stderr
- Option to choose different variants of optimization criterion: mean
  cross-entropy, perplexity, cross-entopry sum
- In-process translation for validation, uses the same memory as training
- Label Smoothing
- CHANGELOG.md
- CONTRIBUTING.md
- Swish activation function default for Transformer
  (https://arxiv.org/pdf/1710.05941.pdf)

### Changed
- Changed shape organization to follow numpy.
- Changed option `--moving-average` to `--exponential-smoothing` and inverted
  formula to `s_t = (1 - \alpha) * s_{t-1} + \alpha * x_t`, `\alpha` is now
  `1-e4` by default
- Got rid of thrust for compile-time mathematical expressions
- Changed boolean option `--normalize` to `--normalize [arg=1] (=0)`. New
  behaviour is backwards-compatible and can also be specified as
  `--normalize=0.6`
- Renamed "s2s" binary to "marian-decoder"
- Renamed "rescorer" binary to "marian-scorer"
- Renamed "server" binary to "marian-server"
- Renamed option name `--dynamic-batching` to `--mini-batch-fit`
- Unified cross-entropy-based validation, supports now perplexity and other CE
- Changed `--normalize (bool)` to `--normalize (float)arg`, allow to change
  length normalization weight as `score / pow(length, arg)`.

### Removed
- Temporarily removed gradient dropping (`--drop-rate X`) until refactoring.
