#include "model.h"

namespace amunmt {
namespace GPU {

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::EncEmbeddings::EncEmbeddings(const NpzConverter& model)
: E_(model.get("Wemb", true))
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::EncForwardGRU::EncForwardGRU(const NpzConverter& model)
: W_(model.get("encoder_W", true)),
  B_(model.get("encoder_b", true, true)),
  U_(model.get("encoder_U", true)),
  Wx_(model.get("encoder_Wx", true)),
  Bx1_(model.get("encoder_bx", true, true)),
  Bx2_(new mblas::Matrix(Bx1_->dim(0), Bx1_->dim(1), Bx1_->dim(2), Bx1_->dim(3), true)),
  Ux_(model.get("encoder_Ux", true)),
  Gamma_1_(model.get("encoder_gamma1", false)),
  Gamma_2_(model.get("encoder_gamma2", false))
{ }

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::EncForwardLSTM::EncForwardLSTM(const NpzConverter& model)
: W_(model.get("encoder_W", true)),
  B_(model.get("encoder_b", true, true)),
  U_(model.get("encoder_U", true)),
  Wx_(model.get("encoder_Wx", true)),
  Bx_(model.get("encoder_bx", true, true)),
  Ux_(model.get("encoder_Ux", true)),
  Gamma_1_(model.get("encoder_gamma1", false)),
  Gamma_2_(model.get("encoder_gamma2", false))
{ }

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::EncBackwardLSTM::EncBackwardLSTM(const NpzConverter& model)
: W_(model.get("encoder_r_W", true)),
  B_(model.get("encoder_r_b", true, true)),
  U_(model.get("encoder_r_U", true)),
  Wx_(model.get("encoder_r_Wx", true)),
  Bx_(model.get("encoder_r_bx", true, true)),
  Ux_(model.get("encoder_r_Ux", true)),
  Gamma_1_(model.get("encoder_r_gamma1", false)),
  Gamma_2_(model.get("encoder_r_gamma2", false))
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::EncBackwardGRU::EncBackwardGRU(const NpzConverter& model)
: W_(model.get("encoder_r_W", true)),
  B_(model.get("encoder_r_b", true, true)),
  U_(model.get("encoder_r_U", true)),
  Wx_(model.get("encoder_r_Wx", true)),
  Bx1_(model.get("encoder_r_bx", true, true)),
  Bx2_(new mblas::Matrix( Bx1_->dim(0), Bx1_->dim(1), Bx1_->dim(2), Bx1_->dim(3), true)),
  Ux_(model.get("encoder_r_Ux", true)),
  Gamma_1_(model.get("encoder_r_gamma1", false)),
  Gamma_2_(model.get("encoder_r_gamma2", false))
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::DecEmbeddings::DecEmbeddings(const NpzConverter& model)
: E_(model.getFirstOfMany({std::make_pair("Wemb_dec", false),
                           std::make_pair("Wemb", false)}, true))
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::DecInit::DecInit(const NpzConverter& model)
: Wi_(model.get("ff_state_W", true)),
  Bi_(model.get("ff_state_b", true, true)),
  Gamma_(model.get("ff_state_gamma", false))
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::DecGRU1::DecGRU1(const NpzConverter& model)
: W_(model.get("decoder_W", true)),
  B_(model.get("decoder_b", true, true)),
  U_(model.get("decoder_U", true)),
  Wx_(model.get("decoder_Wx", true)),
  Bx1_(model.get("decoder_bx", true, true)),
  Bx2_(new mblas::Matrix(Bx1_->dim(0), Bx1_->dim(1), Bx1_->dim(2), Bx1_->dim(3), true)),
  Ux_(model.get("decoder_Ux", true)),
  Gamma_1_(model.get("decoder_cell1_gamma1", false)),
  Gamma_2_(model.get("decoder_cell1_gamma2", false))
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::DecGRU2::DecGRU2(const NpzConverter& model)
: W_(model.get("decoder_Wc", true)),
  B_(model.get("decoder_b_nl", true, true)),
  U_(model.get("decoder_U_nl", true)),
  Wx_(model.get("decoder_Wcx", true)),
  Bx2_(model.get("decoder_bx_nl", true, true)),
  Bx1_(new mblas::Matrix(Bx2_->dim(0), Bx2_->dim(1), Bx2_->dim(2), Bx2_->dim(3), true)),
  Ux_(model.get("decoder_Ux_nl", true)),
  Gamma_1_(model.get("decoder_cell2_gamma1", false)),
  Gamma_2_(model.get("decoder_cell2_gamma2", false))
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::DecLSTM1::DecLSTM1(const NpzConverter& model)
: W_(model.get("decoder_W", true)),
  B_(model.get("decoder_b", true, true)),
  U_(model.get("decoder_U", true)),
  Wx_(model.get("decoder_Wx", true)),
  Bx_(model.get("decoder_bx", true, true)),
  Ux_(model.get("decoder_Ux", true)),
  Gamma_1_(model.get("decoder_cell1_gamma1", false)),
  Gamma_2_(model.get("decoder_cell1_gamma2", false))
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::DecLSTM2::DecLSTM2(const NpzConverter& model)
: W_(model.get("decoder_Wc", true)),
  B_(model.get("decoder_b_nl", true, true)),
  U_(model.get("decoder_U_nl", true)),
  Wx_(model.get("decoder_Wcx", true)),
  Bx_(model.get("decoder_bx_nl", true, true)),
  Ux_(model.get("decoder_Ux_nl", true)),
  Gamma_1_(model.get("decoder_cell2_gamma1", false)),
  Gamma_2_(model.get("decoder_cell2_gamma2", false))
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::DecAlignment::DecAlignment(const NpzConverter& model)
: V_(model.get("decoder_U_att", true, true)),
  W_(model.get("decoder_W_comb_att", true)),
  B_(model.get("decoder_b_att", true, true)),
  U_(model.get("decoder_Wc_att", true)),
  C_(model.get("decoder_c_tt", true)), // scalar?
  Gamma_1_(model.get("decoder_att_gamma1", false)),
  Gamma_2_(model.get("decoder_att_gamma2", false))
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::DecSoftmax::DecSoftmax(const NpzConverter& model)
: W1_(model.get("ff_logit_lstm_W", true)),
  B1_(model.get("ff_logit_lstm_b", true, true)),
  W2_(model.get("ff_logit_prev_W", true)),
  B2_(model.get("ff_logit_prev_b", true, true)),
  W3_(model.get("ff_logit_ctx_W", true)),
  B3_(model.get("ff_logit_ctx_b", true, true)),
  W4_(model.getFirstOfMany({std::make_pair(std::string("ff_logit_W"), false),
                            std::make_pair(std::string("Wemb_dec"), true),
                            std::make_pair(std::string("Wemb"), true)}, true)),
  B4_(model.get("ff_logit_b", true, true)),
  Gamma_0_(model.get("ff_logit_l1_gamma0", false)),
  Gamma_1_(model.get("ff_logit_l1_gamma1", false)),
  Gamma_2_(model.get("ff_logit_l1_gamma2", false))
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
Weights::Weights(const std::string& npzFile, const YAML::Node& config,  size_t device)
: Weights(NpzConverter(npzFile), config, device)
{}

Weights::Weights(const NpzConverter& model, const YAML::Node& config, size_t device)
: encEmbeddings_(model),
  decEmbeddings_(model),
  decInit_(model),
  decAlignment_(model),
  decSoftmax_(model),
  device_(device)
{

  std::string encCell = config["enc-cell"] ? config["enc-cell"].as<std::string>() : "gru";
  std::string encCell_r = config["enc-cell-r"] ? config["enc-cell-r"].as<std::string>() : encCell;
  initEncForward(model, encCell);
  initEncBackward(model, encCell_r);

  std::string decCell = config["dec-cell"] ? config["dec-cell"].as<std::string>() : "gru";
  std::string decCell2 = config["dec-cell-2"] ? config["dec-cell-2"].as<std::string>() : decCell;
  initDec1(model, decCell);
  initDec2(model, decCell2);
}

void Weights::initEncForward(const NpzConverter& model,std::string celltype){
  if(celltype == "lstm"){
    encForwardLSTM_ = std::shared_ptr<EncForwardLSTM>(new EncForwardLSTM(model));
  } else if (celltype == "mlstm") {
    encForwardMLSTM_ = std::shared_ptr<MultWeights<EncForwardLSTM>>
      (new MultWeights<EncForwardLSTM>(model, "encoder"));
  } else if (celltype == "gru"){
    encForwardGRU_ = std::shared_ptr<EncForwardGRU>(new EncForwardGRU(model));
  }
}

void Weights::initEncBackward(const NpzConverter& model,std::string celltype) {
  if(celltype == "lstm"){
    encBackwardLSTM_ = std::shared_ptr<EncBackwardLSTM>(new EncBackwardLSTM(model));
  } else if (celltype == "mlstm") {
    encBackwardMLSTM_ = std::shared_ptr<MultWeights<EncBackwardLSTM>>
      (new MultWeights<EncBackwardLSTM>(model, "encoder_r"));
  } else if (celltype == "gru"){
    encBackwardGRU_ = std::shared_ptr<EncBackwardGRU>(new EncBackwardGRU(model));
  }
}

void Weights::initDec1(const NpzConverter& model,std::string celltype){
  if (celltype == "lstm"){
    decLSTM1_ = std::shared_ptr<DecLSTM1>(new DecLSTM1(model));
  } else if (celltype == "mlstm") {
    decMLSTM1_ = std::shared_ptr<MultWeights<DecLSTM1>>(new MultWeights<DecLSTM1>(model, "decoder"));
  } else if (celltype == "gru") {
    decGru1_ = std::shared_ptr<DecGRU1>(new DecGRU1(model));
  }
}

void Weights::initDec2(const NpzConverter& model,std::string celltype){
  if (celltype == "lstm"){
    decLSTM2_ = std::shared_ptr<DecLSTM2>(new DecLSTM2(model));
  } else if (celltype == "mlstm") {
    decMLSTM2_ = std::shared_ptr<MultWeights<DecLSTM2>>(new MultWeights<DecLSTM2>(model, "decoder_2"));
  } else if (celltype == "gru") {
    decGru2_ = std::shared_ptr<DecGRU2>(new DecGRU2(model));
  }
}

} // namespace
}

