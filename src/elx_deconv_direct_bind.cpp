#include "elx_deconv_direct.hpp"

namespace euler {

Template_elx_deconv_direct_t void
Instance_elx_deconv_direct_t::bind_execute_functions()
{
#define BIND_CONV_KERNEL(S, F, K)                                              \
  if (K == 3) {                                                                \
    conv_kernel_binder::bind<S, F, 3>(O, T, func);                             \
  } else if (K == 5) {                                                         \
    conv_kernel_binder::bind<S, F, 5>(O, T, func);                             \
  } else if (K == 7) {                                                         \
    conv_kernel_binder::bind<S, F, 7>(O, T, func);                             \
  }

  auto bind_conv_kernel = [&](int O, int T,
      conv_kernel_binder::kconv<TarrayTypes> **func, int K) {
    switch (xopt_) {
    case (0xa060):
      if (this->input_fmt == nchw) {
        if (this->ws == 1) {
          BIND_CONV_KERNEL(1, GKF_EBD, K);
        } else if (this->ws == 2) {
          BIND_CONV_KERNEL(2, GKF_EBD, K);
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      } else if (this->input_fmt == nhwc) {
        if (this->ws == 1) {
          BIND_CONV_KERNEL(1, GKF_FCF, K);
        } else if (this->ws == 2) {
          BIND_CONV_KERNEL(2, GKF_FCF, K);
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      } else {
        if (this->ws == 1) {
          BIND_CONV_KERNEL(1, GKF_DCD, K);
        } else if (this->ws == 2) {
          BIND_CONV_KERNEL(2, GKF_DCD, K);
        } else {
          el_error("Stride > 2 not yet bounded");
        }
      }
      break;
    default:
      el_error("Unknown xopt");
      break;
    }
  };

  bind_conv_kernel(this->O, this->T, &ker_conv_, this->kw);
  bind_conv_kernel(this->O, this->Tr, &ker_conv_Tr_, this->kw);

#define EXECUTE_CASE(n)                                                        \
  case 0x##n:                                                                  \
    printf("execute_opt=" #n "\n");                                            \
    execute_opt_ = &Instance_elx_deconv_direct_t::__execute_##n;                 \
    break

  switch (xopt_) {
    EXECUTE_CASE(a060);
  default:
    el_error("Unimplemented xopt");
    break;
  }
}

} // namespace euler
