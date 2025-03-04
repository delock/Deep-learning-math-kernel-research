#pragma once

#include "euler.hpp"
#include "el_def.hpp"
#include "el_utils.hpp"
#include "elx_conv.hpp"
#include "kernel/elk_conv_wino.hpp"

#include "kernel/elk_conv_wino_2x2_3x3_input.hxx"
#include "kernel/elk_conv_wino_3x3_3x3_input.hxx"
#include "kernel/elk_conv_wino_4x4_3x3_input.hxx"
#include "kernel/elk_conv_wino_5x5_3x3_input.hxx"

namespace euler {

template <typename TinputType, typename InputType, int I, int A, int K, int V>
class elx_conv_wino_trans_input_base {
public:
  using op_type = float;

  elx_conv_wino_trans_input_base() {}
  virtual ~elx_conv_wino_trans_input_base() {}
  void setup(elx_conv_params_t *conv_xc) {
    xc = conv_xc;
    mthr_ = xc->nthreads;

    stream_in_ = xc->streaming_input
        ? (xc->streaming_input == STORE_STREAMING)
        : !(xc->execution_mode & FUS_MSK) ? true : false;

    hA_end_ = (xc->ih + xc->tp) - (xc->ht - 1) * (A - K + 1) - 1;
    wA_end_ = (xc->iw + xc->lp) - (xc->wt - 1) * (A - K + 1) - 1;

    input_is_bfmt_ = xc->input_fmt == nChw16c;
    input_as_bfmt_ = xc->input_fmt == nchw && xc->input_as_blocked;

    bind_kernel_functions();
  }

protected:
  void bind_kernel_functions() {
    if (input_is_bfmt_ || input_as_bfmt_) {
      ker_trans_input_ = elk_conv_wino_trans_input<op_type, InputType,
          TKF_BLOCKED, false, I, A, V>::execute;
      ker_trans_input0_ = elk_conv_wino_trans_input<op_type, InputType,
          TKF_BLOCKED, true, I, A, V>::execute;
    } else if (xc->input_fmt == nhwc) {
      ker_trans_input_ = elk_conv_wino_trans_input<op_type, InputType,
          TKF_NHWC, false, I, A, V>::execute;
      ker_trans_input0_ = elk_conv_wino_trans_input<op_type, InputType,
          TKF_NHWC, true, I, A, V>::execute;
    } else { // nchw
      ker_trans_input_ = elk_conv_wino_trans_input<op_type, InputType,
          TKF_COMPACT, false, I, A, V>::execute;
      ker_trans_input0_ = elk_conv_wino_trans_input<op_type, InputType,
          TKF_COMPACT, true, I, A, V>::execute;
    }
  }

  elx_conv_params_t *xc = nullptr;

  decltype(elk_conv_wino_trans_input<
      op_type, InputType, 0, false, I, A, V>::execute) *ker_trans_input_;
  decltype(elk_conv_wino_trans_input<
      op_type, InputType, 0, true, I, A, V>::execute) *ker_trans_input0_;

  bool stream_in_;
  bool input_is_bfmt_;
  bool input_as_bfmt_;

  int hA_end_;
  int wA_end_;

  int mthr_;
};

template <typename TinputType, typename InputType, int I, int A, int K, int V>
class elx_conv_wino_trans_input_t :
  public elx_conv_wino_trans_input_base<TinputType, InputType, I, A, K, V> {
public:
  using super = elx_conv_wino_trans_input_base<TinputType, InputType, I, A, K, V>;
  using op_type = typename super::op_type;

public:
  elx_conv_wino_trans_input_t() {}
  virtual ~elx_conv_wino_trans_input_t() {}

  void execute(TinputType *__restrict t_input,
      InputType *__restrict input, int Tz, int _t2, int _ic4);
  void execute(TinputType *__restrict t_input,
      InputType *__restrict input, int _ic4);

  void operator () (TinputType *__restrict t_input,
      InputType *__restrict input, int Tz, int _t2, int _ic4) {
    execute(t_input, input, Tz, _t2, _ic4);
  }
  void operator () (TinputType *__restrict t_input,
      InputType *__restrict input, int _ic4) {
    execute(t_input, input, _ic4);
  }

protected:
  inline void __execute_blocked(TinputType *__restrict tinput,
      InputType *__restrict input, int Tz, int _t2, int _ic4);

  inline void __execute_blocked(TinputType *__restrict tinput,
      InputType *__restrict input, int _ic4);

  inline void __execute_post(TinputType * __restrict tinput,
      op_type *at, int Tz, int _ic3, int _I2, int _T);

  using super::xc;
  using super::hA_end_;
  using super::wA_end_;
  using super::input_is_bfmt_;
  using super::input_as_bfmt_;
  using super::ker_trans_input_;
  using super::ker_trans_input0_;
  using super::stream_in_;
  using super::mthr_;

  inline void __execute_nhwc(TinputType *__restrict t_input,
      InputType *__restrict input, int Tz, int _t2, int _ic4);
  inline void __execute_nchw(TinputType *__restrict t_input,
      InputType *__restrict input, int Tz, int _t2, int _ic4);

  inline void __execute_nhwc(TinputType *__restrict t_input,
      InputType *__restrict input, int _ic4);
  inline void __execute_nchw(TinputType *__restrict t_input,
      InputType *__restrict input, int _ic4);
};

template <typename InputType, int I, int A, int K, int V>
class elx_conv_wino_trans_input_t<uint8_t, InputType, I, A, K, V>
 : public elx_conv_wino_trans_input_base<float, InputType, I, A, K, V> {
public:
  using TinputType = float;
  using TscaleType = float;
  using super = elx_conv_wino_trans_input_base<float, InputType, I, A, K, V>;

protected:
  using super::xc;
  using op_type = typename super::op_type;
  using super::hA_end_;
  using super::wA_end_;
  using super::input_is_bfmt_;
  using super::input_as_bfmt_;
  using super::mthr_;

public:
  elx_conv_wino_trans_input_t() {}
  virtual ~elx_conv_wino_trans_input_t() {}

  void execute(TscaleType *__restrict tinput_quant_scale,
      uint8_t *__restrict t_input_u8,
      TinputType *__restrict t_input,
      InputType *__restrict input, int _ic4);

  void execute(TscaleType *__restrict tinput_quant_scale,
      uint8_t *__restrict t_input_u8,
      TinputType *__restrict t_input,
      InputType *__restrict input,
      int _t2, int Tz);

  void operator() (TscaleType *__restrict tinput_quant_scale,
      uint8_t *__restrict t_input_u8,
      TinputType *__restrict t_input,
      InputType *__restrict input, int _ic4) {
    execute(tinput_quant_scale, t_input_u8, t_input, input, _ic4);
  }

  void operator() (TscaleType *__restrict tinput_quant_scale,
      uint8_t *__restrict t_input_u8,
      TinputType *__restrict t_input,
      InputType *__restrict input,
      int _t2, int Tz) {
    execute(tinput_quant_scale, t_input_u8, t_input, input, _t2, Tz);
  }

protected:
  inline void __execute_blocked_nhwc(TscaleType *__restrict tinput_quant_scale,
      uint8_t *__restrict t_input_u8,
      TinputType *__restrict t_input,
      InputType *__restrict input, int _ic4);

  inline void __execute_nchw(TscaleType *__restrict tinput_quant_scale,
      uint8_t *__restrict t_input_u8,
      TinputType *__restrict t_input,
      InputType *__restrict input, int _ic4);

  inline void __execute_blocked_nhwc(TscaleType *__restrict tinput_quant_scale,
      uint8_t *__restrict t_input_u8,
      TinputType *__restrict t_input,
      InputType *__restrict input,
      int _t2, int Tz);

  inline void __execute_nchw(TscaleType *__restrict tinput_quant_scale,
      uint8_t *__restrict t_input_u8,
      TinputType *__restrict t_input,
      InputType *__restrict input,
      int _t2, int Tz);

  using super::ker_trans_input_;
  using super::ker_trans_input0_;
};

// Three stage indexing, width, hight, image
template <int A, int K>
class input_tile_iter {
  constexpr static int output_line = A - K +1;
public:
  input_tile_iter(int n_init, int t_init,
      int ht, int wt, int h, int w, int tp, int lp)
    : ht_(ht), wt_(wt), tp_(tp), lp_(lp),
    hA_end_(h + tp - (ht -1) * output_line -1),
    wA_end_(w + lp - (wt -1) * output_line -1),
    tile_h_(t_init / wt),
    tile_w_(t_init % wt),
    anchor_t_(tile_h_ * output_line - tp),
    anchor_l_(tile_w_ * output_line - lp),
    n_(n_init),
    t_ (tile_h_ > 0 ? 0 : tp),
    l_ (tile_w_ > 0 ? 0 : lp),
    d_ (tile_h_ < ht -1 ? A -1 : hA_end_),
    r_ (tile_w_ < wt -1 ? A -1 : wA_end_) {}

  inline input_tile_iter &operator ++() {
    if ( ++ tile_w_ < wt_) {
      anchor_l_ += output_line;
    } else {
      tile_w_ = 0;
      anchor_l_ = -lp_;
      if ( ++ tile_h_ < ht_ ) {
        anchor_t_ += output_line;
      } else {
        n_ += 1;
        tile_h_ = 0;
        anchor_t_ = -tp_;
      }
      t_ = tile_h_ > 0 ? 0 : tp_;
      d_ = tile_h_ < ht_ - 1 ? A -1 : hA_end_;
    }

    l_ = tile_w_ > 0 ? 0 : lp_;
    r_ = tile_w_ < wt_ - 1 ? A -1 : wA_end_;
    return *this;
  }

  inline bool is_border() const {
    return !(t_ == 0 && l_ == 0 && d_ == A-1 && r_ == A -1);
  }

protected:
  int ht_, wt_, tp_, lp_;

public:
  int hA_end_, wA_end_;
  int tile_h_, tile_w_;
  int anchor_t_, anchor_l_;
  int n_, t_, l_, d_, r_;
};

template class elx_conv_wino_trans_input_t<float, float, ISA_SKX_AVX512, 4, 3, 16>;
template class elx_conv_wino_trans_input_t<float, float, ISA_SKX_AVX512, 5, 3, 16>;
template class elx_conv_wino_trans_input_t<float, float, ISA_SKX_AVX512, 6, 3, 16>;
template class elx_conv_wino_trans_input_t<float, float, ISA_SKX_AVX512, 7, 3, 16>;

template class elx_conv_wino_trans_input_t<short, float, ISA_SKX_AVX512, 4, 3, 16>;
template class elx_conv_wino_trans_input_t<short, float, ISA_SKX_AVX512, 5, 3, 16>;
template class elx_conv_wino_trans_input_t<short, float, ISA_SKX_AVX512, 6, 3, 16>;
template class elx_conv_wino_trans_input_t<short, float, ISA_SKX_AVX512, 7, 3, 16>;

template class elx_conv_wino_trans_input_t<uint8_t, float, ISA_SKX_AVX512, 4, 3, 16>;
template class elx_conv_wino_trans_input_t<uint8_t, float, ISA_SKX_AVX512, 5, 3, 16>;
template class elx_conv_wino_trans_input_t<uint8_t, float, ISA_SKX_AVX512, 6, 3, 16>;
template class elx_conv_wino_trans_input_t<uint8_t, float, ISA_SKX_AVX512, 7, 3, 16>;

template class elx_conv_wino_trans_input_t<uint8_t, uint8_t, ISA_SKX_AVX512, 4, 3, 16>;
template class elx_conv_wino_trans_input_t<uint8_t, uint8_t, ISA_SKX_AVX512, 5, 3, 16>;
template class elx_conv_wino_trans_input_t<uint8_t, uint8_t, ISA_SKX_AVX512, 6, 3, 16>;

#ifdef ENABLE_USER_FP16
template class elx_conv_wino_trans_input_t<float, short, ISA_SKX_AVX512, 4, 3, 16>;
template class elx_conv_wino_trans_input_t<float, short, ISA_SKX_AVX512, 5, 3, 16>;
template class elx_conv_wino_trans_input_t<float, short, ISA_SKX_AVX512, 6, 3, 16>;
template class elx_conv_wino_trans_input_t<float, short, ISA_SKX_AVX512, 7, 3, 16>;

template class elx_conv_wino_trans_input_t<uint8_t, short, ISA_SKX_AVX512, 4, 3, 16>;
template class elx_conv_wino_trans_input_t<uint8_t, short, ISA_SKX_AVX512, 5, 3, 16>;
template class elx_conv_wino_trans_input_t<uint8_t, short, ISA_SKX_AVX512, 6, 3, 16>;
template class elx_conv_wino_trans_input_t<uint8_t, short, ISA_SKX_AVX512, 7, 3, 16>;
#endif

}  // namespace euler
