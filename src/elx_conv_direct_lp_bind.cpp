#include "elx_conv_direct_lp.hpp"

namespace euler {

Template_elx_conv_direct_lp_t void
Instance_elx_conv_direct_lp_t::bind_execute_functions()
{
#define BIND_GEMM_KERNEL(S, F)                                                 \
  u8s8_gemm_kernel_binder::bind<S, F>(O, T, func);

#define BIND_CONV_KERNEL(S, F, K)                                              \
  if (K == 3) {                                                                \
    u8s8_conv_kernel_binder::bind<S, F, 3>(O, T, func);                        \
  } else if (K == 5) {                                                         \
    u8s8_conv_kernel_binder::bind<S, F, 5>(O, T, func);                        \
  } else if (K == 7) {                                                         \
    u8s8_conv_kernel_binder::bind<S, F, 7>(O, T, func);                        \
  }

  auto bind_gemm_kernel = [&](int O, int T,
      u8s8_gemm_kernel_binder::kgemm<TarrayTypes, float> **func) {
    switch (xopt_) {
    case (0xd160):
      if (this->input_fmt == nhwc && this->output_fmt == nhwc) {
        if (this->ws == 1) {
          BIND_GEMM_KERNEL(1, GKF_FCF)
        } else if (this->ws == 2) {
          BIND_GEMM_KERNEL(2, GKF_FCF)
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      } else if (this->input_fmt == nhwc && this->output_fmt == nChw16c) {
        if (this->ws == 1) {
          BIND_GEMM_KERNEL(1, GKF_FCD)
        } else if (this->ws == 2) {
          BIND_GEMM_KERNEL(2, GKF_FCD)
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      } else if (this->input_fmt == nChw16c && this->output_fmt == nhwc) {
        if (this->ws == 1) {
          BIND_GEMM_KERNEL(1, GKF_DCF)
        } else if (this->ws == 2) {
          BIND_GEMM_KERNEL(2, GKF_DCF)
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      } else { // blocked
        if (this->ws == 1) {
          BIND_GEMM_KERNEL(1, GKF_DCD)
        } else if (this->ws == 2) {
          BIND_GEMM_KERNEL(2, GKF_DCD)
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      }
      break;
    default:
      el_error("Unknown direct gemm kernel xopt");
      break;
    }
  };

  auto bind_conv_kernel = [&](int O, int T,
      u8s8_conv_kernel_binder::kconv<TarrayTypes, OutputType> **func, int K) {
    switch (xopt_) {
    case (0xa160):
      if (compact_ir_weights_ && this->input_fmt == nhwc && this->output_fmt == nhwc) {
        if (this->ws == 1) {
          BIND_CONV_KERNEL(1, GKF_FBF, K);
        } else if (this->ws == 2) {
          BIND_CONV_KERNEL(2, GKF_FBF, K);
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      } else if (this->input_fmt == nhwc && this->output_fmt == nhwc) {
        if (this->ws == 1) {
          BIND_CONV_KERNEL(1, GKF_FCF, K);
        } else if (this->ws == 2) {
          BIND_CONV_KERNEL(2, GKF_FCF, K);
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      } else if (compact_ir_weights_ && this->input_fmt == nhwc &&
                 this->output_fmt == nChw16c) {
        if (this->ws == 1) {
          BIND_CONV_KERNEL(1, GKF_FBD, K);
        } else if (this->ws == 2) {
          BIND_CONV_KERNEL(2, GKF_FBD, K);
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      } else if (!compact_ir_weights_ && this->input_fmt == nChw16c &&
                 this->output_fmt == nChw16c) {
        if (this->ws == 1) {
          BIND_CONV_KERNEL(1, GKF_DCD, K);
        } else if (this->ws == 2) {
          BIND_CONV_KERNEL(2, GKF_DCD, K);
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      } else if (this->input_fmt == nhwc && this->output_fmt == nChw16c) {
        if (this->ws == 1) {
          BIND_CONV_KERNEL(1, GKF_FCD, K);
        } else if (this->ws == 2) {
          BIND_CONV_KERNEL(2, GKF_FCD, K);
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      } else if (this->input_fmt == nChw16c && this->output_fmt == nhwc) {
        if (this->ws == 1) {
          BIND_CONV_KERNEL(1, GKF_DCF, K);
        } else if (this->ws == 2) {
          BIND_CONV_KERNEL(2, GKF_DCF, K);
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      } else {
        el_error("direct:int8: kernel fmt not supported");
      }
      break;
    default:
      el_error("Unknown xopt");
      break;
    }
  };

  if (xopt_ == 0xa160 /*|| xopt_ == 0xb160*/) {
    bind_conv_kernel(this->O, this->T, &ker_conv_, this->kw);
    bind_conv_kernel(this->O, this->Tr, &ker_conv_Tr_, this->kw);
  } else if (xopt_ == 0xd160) {
    if (this->wt > 128) {
      el_error("direct: d160: wt > max-kernel-slot:128");
    }
    iter_each (_wt, this->wt) {
      int Tz = _wt == this->wt - 1 ? this->Tr : this->T;
      for (int _kw = 0; _kw < this->kw; ++_kw) {
        // _iws, _iwe
        // _iw = ws * _ow + _kw - lp
        auto ows0 = _wt * this->T;
        auto owe0 = _wt * this->T + Tz - 1;
        auto _iws = this->ws * ows0 + _kw - this->lp;
        while (_iws < 0)
          _iws += this->ws;
        auto _iwe = this->ws * owe0 + _kw - this->lp;
        while (_iwe > this->iw - 1)
          _iwe -= this->ws;
        auto _ows = (_iws + this->lp - _kw) / this->ws;
        auto _owe = (_iwe + this->lp - _kw) / this->ws;
        bind_gemm_kernel(this->O, _owe - _ows + 1, &ker_gemm_[_wt][_kw]);
      }
    }
  }

#define EXECUTE_CASE(n)                                                        \
  case 0x##n:                                                                  \
    printf("execute_opt=" #n "\n");                                            \
    execute_opt_ = &Instance_elx_conv_direct_lp_t::__execute_##n;              \
    break

  switch (xopt_) {
    EXECUTE_CASE(a160);
    //EXECUTE_CASE(b160);
    EXECUTE_CASE(d160);
  default:
    el_error("Unimplemented direct lp xopt");
    break;
  }
}

} // namespace euler
