#ifndef __ELX_CONV_DIRECT_HPP__
#define __ELX_CONV_DIRECT_HPP__

#include "el_def.hpp"
#include "el_utils.hpp"
#include "elx_conv.hpp"
#include "euler.hpp"
#include "kernel/elk_gemm_otj_binder.hxx"

namespace euler {

#define Template_elx_conv_direct_t                                         \
  template <typename UserTypes, typename TarrayType, const int V, const int I>

#define Instance_elx_conv_direct_t                                         \
    elx_conv_direct_t<UserTypes, TarrayType, V, I>

Template_elx_conv_direct_t
class elx_conv_direct_t : public elx_conv_t<UserTypes> {
  using InputType = typename UserTypes::InputType;
  using WeightsType = typename UserTypes::WeightsType;
  using OutputType = typename UserTypes::OutputType;
  using BiasType = typename UserTypes::BiasType;

  public:
  elx_conv_direct_t(eld_conv_t<UserTypes> &dc);
  virtual ~elx_conv_direct_t();

  virtual void execute(OutputType *output, InputType *input, WeightsType *weights, BiasType *bias);

  private:
  void __execute_a060(OutputType *output, InputType *input, WeightsType *weights, BiasType *bias);
  void __execute_d060(OutputType *output, InputType *input, WeightsType *weights, BiasType *bias);

  void trans_weights_blocked_to_compact(TarrayType *tweights, WeightsType *weights);

  void conv_a060(OutputType *output, InputType *input, WeightsType *weights, BiasType *bias, int _ic4, int _oc4, int _ht, int _wt);
 
  void gemm_d060_blocked_input(OutputType *toutput, InputType *tinput, WeightsType *tweights, BiasType *bias, int _ic4, int _oc4, int _ht, int _wt);
  void gemm_d060_nchw_input(OutputType *toutput, InputType *tinput, WeightsType *tweights, BiasType *bias, int _ic4, int _oc4, int _ht, int _wt);

  void set_trans_buffers();
  int prepare_execute_opt();
  void bind_execute_functions();

  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_I_O_T_;
  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_I_O_Tr_;
  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_IrO_T_;
  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_IrO_Tr_;
  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_left_I_O_T_;
  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_left_I_O_Tr_;
  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_left_IrO_T_;
  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_left_IrO_Tr_;
  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_right_I_O_T_;
  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_right_I_O_Tr_;
  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_right_IrO_T_;
  gemm_kernel_binder::kgemm<conv_impl::FP32> *ker_gemm_right_IrO_Tr_;

  gemm_kernel_binder::kconv<conv_impl::FP32> *ker_conv_;
  gemm_kernel_binder::kconv<conv_impl::FP32> *ker_conv_Tr_;

  void (elx_conv_direct_t::*execute_opt_)(OutputType *, InputType *, WeightsType *, BiasType *);

  bool no_pad_;
  bool is_first_run_;
  bool inference_acc_;

  bool stream_in_;
  bool stream_out_;
  bool stream_wei_;

  bool is_bfmt_;
  bool input_is_bfmt_;
  bool weights_is_bfmt_;
  bool output_is_bfmt_;
  bool input_as_bfmt_;
  bool weights_as_bfmt_;
  bool output_as_bfmt_;

  WeightsType *tweights_;
  InputType *tinput_;
  OutputType *toutput_;
  unsigned char *tinput_msk_;
  InputType *binput_; // blocked input
  WeightsType *bweights_;
  OutputType *boutput_;

  unsigned int xopt_;
  int attr_;
  int mthr_;
  size_t tweights_size_;
  size_t tinput_size_;
  size_t toutput_size_;
  size_t binput_size_;
  size_t bweights_size_;
  size_t boutput_size_;
  TarrayType *scratch_;
};

template class elx_conv_direct_t<conv::FP32, float, 16, ISA_SKX_AVX512>;
template class elx_conv_direct_t<conv::FP32, float, 8, ISA_SKX_AVX512>;

}  // namespace euler
#endif  // __ELX_CONV_DIRECT_HPP__
