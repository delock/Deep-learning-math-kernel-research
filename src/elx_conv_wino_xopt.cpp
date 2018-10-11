#include <string.h>
#include "el_intrin.hpp"
#include "el_utils.hpp"
#include "el_def.hpp"
#include "el_utils.hpp"
#include "elx_conv.hpp"
#include "elx_conv_wino.hpp"
#include "euler.hpp"

namespace euler {

//
// -------------+--------------+---------------
//  execute-opt | fusion-along | duplication
// -------------+--------------+---------------
//     A000     |      _       |    _
// -------------+--------------+---------------
//     A010     |      i       |    _
// -------------+--------------+---------------
//     A061     |    t + o     |    I
// -------------+--------------+---------------
//     A071     |  i + t + o   |    I
// -------------+--------------+---------------
//     A073     |  i + t + o   |  I + O
// -------------+--------------+---------------
//     A079     |  i + t + o   |  I + W
// -------------+--------------+---------------
//     A07b     |  i + t + o   |  I + W + O
// -------------+--------------+---------------
//     A0e0     |  t + o + wA  |    _
// -------------+--------------+---------------
//     A0e1     |  t + o + wA  |    I
// -------------+--------------+---------------
//


// tweights:     oc4 | oc3, ic3, A, A, O2, I2, V, V
// tinputs:  t2      | A, A, ic3, I2, T, V
// toutput:  t2, oc4 | A, A, oc3, O2, T, V
template <typename Type, const int A, const int K, const int V, const int I>
void elx_conv_wino_t<Type, A, K, V, I>::__execute_a061(
    Type * __restrict output, Type * __restrict input, Type * __restrict weights, Type * __restrict bias)
{
  MD2(Type, atinput2, tinput_, mthr_, A * A * this->T * this->IC);
  MD2(Type, atoutput2, toutput_, mthr_, A * A * this->T * this->oc3 * this->O2 * V);
  MD2(Type, atweights2, tweights_, this->oc4, A * A * this->IC * this->oc3 * this->O2 * V);

  MD3(Type, aoutput, output, this->n, this->oc4, this->oh * this->ow * this->oc3 * this->O2 * V);
  MD2(Type, abias, bias, this->oc4, this->oc3 * this->O2 * V);

#pragma omp parallel num_threads(mthr_) proc_bind(close)
  {
    if (is_first_run_) {
      trans_weights(tweights_, weights, this->oc4);
#pragma omp barrier
    }

    auto t2_history = -1;

#pragma omp for nowait collapse(2)
    iter_each (_t2, this->t2) {
      iter_each (_oc4, this->oc4) {
        int Tz = _t2 == (this->t2 - 1) ? this->Tr : this->T;
        size_t ithr = omp_get_thread_num();

        if (t2_history != _t2) {
          trans_input(&md2(atinput2, ithr, 0), input, _t2, Tz);
          t2_history = _t2;
        }
        gemm(&md2(atoutput2, ithr, 0), &md2(atinput2, ithr, 0),
            &md2(atweights2, _oc4, 0), _t2, Tz);
        trans_output(&md3(aoutput, 0, _oc4, 0), &md2(atoutput2, ithr, 0),
            &md2(abias, _oc4, 0), _t2, Tz);
      }
    }
  }
  if (inference_acc_)
    is_first_run_ = false;
}

// tweights:     oc4, wA | hA, oc3, ic3, O2, I2, V, V
// tinputa:  t2,      wA | hA, ic3, I2, T, V
// toutput:  t2, oc4, wA | hA, oc3, O2, T, V
// toutputa: t2, oc4, oc3, O2, T, wA, hA, V
template <typename Type, const int A, const int K, const int V, const int I>
void elx_conv_wino_t<Type, A, K, V, I>::__execute_a0e1(
    Type * __restrict output, Type * __restrict input, Type * __restrict weights, Type * __restrict bias)
{
  MD2(Type, atinputa2, tinput_, mthr_, A * this->T * this->IC);
  MD2(Type, atoutput2, toutput_, mthr_, A * this->T * this->oc3 * this->O2 * V);
  MD2(Type, atoutputa2, toutputa_, this->t2, this->OC * A * (A - K + 1) * this->T);
  MD3(Type, atweights3, tweights_, this->oc4, A, A * this->IC * this->oc3 * this->O2 * V);
  MD3(Type, aoutput, output, this->n, this->oc4, this->oh * this->ow * this->oc3 * this->O2 * V);
  MD2(Type, abias, bias, this->oc4, this->oc3 * this->O2 * V);

#pragma omp parallel num_threads(mthr_) proc_bind(close)
  {
    if (is_first_run_) {
      trans_weightsa(tweights_, weights);
#pragma omp barrier
    }
#pragma omp for nowait collapse(3)
    iter_each (_t2, this->t2) {
      iter_each (_oc4, this->oc4) {
        iter_each (_wA, A) {
          int Tz = _t2 == (this->t2 - 1) ? this->Tr : this->T;
          size_t ithr = omp_get_thread_num();

          MD6(Type, atoutputa6, &md2(atoutputa2, _t2, 0), this->oc4, this->oc3, this->O2, Tz, A, (A - K + 1) * V);
          trans_inputa(&md2(atinputa2, ithr, 0), input, _t2, _wA, Tz);
          gemma(&md2(atoutput2, ithr, 0), &md2(atinputa2, ithr, 0),
              &md3(atweights3, _oc4, _wA, 0), _t2, Tz);
          trans_outputa_th(&md6(atoutputa6, _oc4, 0, 0, 0, _wA, 0),
              &md2(atoutput2, ithr, 0), Tz);
        }
      }
    }
#pragma omp barrier
    trans_outputa_bh(output, toutputa_, bias);
  }
  if (inference_acc_)
    is_first_run_ = false;
}

// tweights:     oc4, wA | hA, oc3, ic3, O2, I2, V, V
// tinputa:  t2,      wA | hA, ic3, I2, T, V
// toutput:  t2, oc4, wA | hA, oc3, O2, T, V
// toutputa: t2, oc4, oc3, O2, T, wA, hA, V
template <typename Type, const int A, const int K, const int V, const int I>
void elx_conv_wino_t<Type, A, K, V, I>::__execute_a0e0(
    Type * __restrict output, Type * __restrict input, Type * __restrict weights, Type * __restrict bias)
{
  MD2(Type, atinput2, tinput_, this->t2, A * A * this->T * this->IC);
  MD2(Type, atoutput2, toutput_, mthr_, A * this->T * this->oc3 * this->O2 * V);
  MD2(Type, atoutputa2, toutputa_, this->t2, this->OC * A * (A - K + 1) * this->T);
  MD3(Type, atweights3, tweights_, this->oc4, A, A * this->IC * this->oc3 * this->O2 * V);
  MD3(Type, aoutput, output, this->n, this->oc4, this->oh * this->ow * this->oc3 * this->O2 * V);
  MD2(Type, abias, bias, this->oc4, this->oc3 * this->O2 * V);

#pragma omp parallel num_threads(mthr_) proc_bind(close)
  {
    if (is_first_run_) {
      trans_weightsa(tweights_, weights);
    }
    trans_input(tinput_, input);
#pragma omp barrier

#pragma omp for nowait collapse(3)
    iter_each (_t2, this->t2) {
      iter_each (_oc4, this->oc4) {
        iter_each (_wA, A) {
          int Tz = _t2 == (this->t2 - 1) ? this->Tr : this->T;
          size_t ithr = omp_get_thread_num();

          MD6(Type, atoutputa6, &md2(atoutputa2, _t2, 0), this->oc4, this->oc3, this->O2, Tz, A, (A - K + 1) * V);
          MD2(Type, atinputa2, &md2(atinput2, _t2, 0), A, A * Tz * this->IC);
          gemma(&md2(atoutput2, ithr, 0), &md2(atinputa2, _wA, 0),
              &md3(atweights3, _oc4, _wA, 0), _t2, Tz);
          trans_outputa_th(&md6(atoutputa6, _oc4, 0, 0, 0, _wA, 0),
              &md2(atoutput2, ithr, 0), Tz);
        }
      }
    }
#pragma omp barrier
    trans_outputa_bh(output, toutputa_, bias);
  }
  if (inference_acc_)
    is_first_run_ = false;
}

// tweights:     oc4, ic4 | oc3, ic3, A, A, O2, I2, V, V
// tinputs:  t2,      ic4 | A, A, ic3, I2, T, V
// toutput:  t2, oc4      | A, A, oc3, O2, T, V
template <typename Type, const int A, const int K, const int V, const int I>
void elx_conv_wino_t<Type, A, K, V, I>::__execute_a071(
    Type * __restrict output, Type * __restrict input, Type * __restrict weights, Type * __restrict bias)
{
  MD2(Type, atinput2, tinput_, mthr_, A * A * this->T * this->ic3 * this->I2 * V);
  MD2(Type, atoutput2, toutput_, this->t2, this->oc4 * A * A * this->T * this->oc3 * this->O2 * V);
  MD3(Type, atweights3, tweights_, this->oc4, this->ic4, A * A * this->ic3 * this->I2 * V * this->oc3 * this->O2 * V);

  MD3(Type, ainput, input, this->n, this->ic4, this->ih * this->iw * this->ic3 * this->I2 * V);
  MD3(Type, aoutput, output, this->n, this->oc4, this->oh * this->ow * this->oc3 * this->O2 * V);
  MD2(Type, abias, bias, this->oc4, this->oc3 * this->O2 * V);

  if (is_first_run_) {
#pragma omp parallel num_threads(mthr_) proc_bind(close)
    trans_weights(tweights_, weights, this->oc4);
  }

  iter_each(_ic4, this->ic4) {
    int last_ic4 = -1, last_t2 = -1;
#pragma omp parallel num_threads(mthr_) proc_bind(close) firstprivate(last_ic4, last_t2)
#pragma omp for nowait collapse(2)
    iter_each(_t2, this->t2) {
      iter_each(_oc4, this->oc4) {
        int Tz = _t2 == (this->t2 - 1) ? this->Tr : this->T;
        size_t ithr = omp_get_thread_num();
        MD2(Type, atoutput3, &md2(atoutput2, _t2, 0), this->oc4, A * A * Tz * this->oc3 * this->O2 * V);

        if (last_ic4 != _ic4 || last_t2 != _t2) {
          trans_input(
              &md2(atinput2, ithr, 0), &md3(ainput, 0, _ic4, 0), _t2, Tz);
          last_t2 = _t2;
          last_ic4 = _ic4;
        }
        gemm(&md2(atoutput3, _oc4, 0), &md2(atinput2, ithr, 0),
            &md3(atweights3, _oc4, _ic4, 0), _t2, Tz, _ic4);
        if (_ic4 == this->ic4 - 1)
          trans_output(&md3(aoutput, 0, _oc4, 0), &md2(atoutput3, _oc4, 0),
              &md2(abias, _oc4, 0), _t2, Tz);
      }
    }
  }

  if (inference_acc_)
    is_first_run_ = false;
}

template <typename Type, const int A, const int K, const int V, const int I>
void elx_conv_wino_t<Type, A, K, V, I>::__execute_a073(
    Type * __restrict output, Type * __restrict input, Type * __restrict weights, Type * __restrict bias)
{
  MD2(Type, atinput2, tinput_, mthr_, A * A * this->T * this->ic3 * this->I2 * V);
  MD2(Type, atoutput2, toutput_, mthr_, A * A * this->T * this->oc3 * this->O2 * V);
  MD3(Type, atweights3, tweights_, this->oc4, this->ic4, A * A * this->ic3 * this->I2 * V * this->oc3 * this->O2 * V);

  MD3(Type, ainput, input, this->n, this->ic4, this->ih * this->iw * this->ic3 * this->I2 * V);
  MD3(Type, aoutput, output, this->n, this->oc4, this->oh * this->ow * this->oc3 * this->O2 * V);
  MD2(Type, abias, bias, this->oc4, this->oc3 * this->O2 * V);

  if (is_first_run_) {
#pragma omp parallel num_threads(mthr_) proc_bind(close)
    trans_weights(tweights_, weights, this->oc4);
  }

  iter_each(_ic4, this->ic4) {
    int last_ic4 = -1, last_t2 = -1;
#pragma omp parallel num_threads(mthr_) proc_bind(close) firstprivate(last_ic4, last_t2)
#pragma omp for nowait collapse(2)
    iter_each(_t2, this->t2) {
      iter_each(_oc4, this->oc4) {
        int Tz = _t2 == (this->t2 - 1) ? this->Tr : this->T;
        size_t ithr = omp_get_thread_num();

        if (last_ic4 != _ic4 || last_t2 != _t2) {
          trans_input(
              &md2(atinput2, ithr, 0), &md3(ainput, 0, _ic4, 0), _t2, Tz);
          last_t2 = _t2;
          last_ic4 = _ic4;
        }
        gemm_non_acc(&md2(atoutput2, ithr, 0), &md2(atinput2, ithr, 0),
            &md3(atweights3, _oc4, _ic4, 0), _t2, Tz, _ic4);
        trans_output(&md3(aoutput, 0, _oc4, 0), &md2(atoutput2, ithr, 0),
            &md2(abias, _oc4, 0), _t2, Tz, _ic4);
      }
    }
  }

  if (inference_acc_)
    is_first_run_ = false;
}

template <typename Type, const int A, const int K, const int V, const int I>
void elx_conv_wino_t<Type, A, K, V, I>::__execute_a07b(
    Type * __restrict output, Type * __restrict input, Type * __restrict weights, Type * __restrict bias)
{
  MD2(Type, atinput2, tinput_, mthr_, A * A * this->T * this->ic3 * this->I2 * V);
  MD2(Type, atoutput2, toutput_, mthr_, A * A * this->T * this->oc3 * this->O2 * V);
  MD2(Type, atweights2, tweights_, mthr_, A * A * this->ic3 * this->I2 * V * this->oc3 * this->O2 * V);

  MD3(Type, ainput, input, this->n, this->ic4, this->ih * this->iw * this->ic3 * this->I2 * V);
  MD3(Type, aoutput, output, this->n, this->oc4, this->oh * this->ow * this->oc3 * this->O2 * V);
  MD2(Type, abias, bias, this->oc4, this->oc3 * this->O2 * V);

  int last_ic4 = -1, last_t2 = -1, last_oc4 = -1;
#pragma omp parallel num_threads(mthr_) proc_bind(close) firstprivate(last_t2, last_ic4, last_oc4)
  iter_each(_ic4, this->ic4) {
#pragma omp for nowait collapse(2)
    iter_each(_t2, this->t2) {
      iter_each(_oc4, this->oc4) {
        int Tz = _t2 == (this->t2 - 1) ? this->Tr : this->T;
        size_t ithr = omp_get_thread_num();

        if (last_ic4 != _ic4 || last_oc4 != _oc4) {
          trans_weightsf(&md2(atweights2, ithr, 0), weights, _ic4, _oc4);
        }
        if (last_ic4 != _ic4 || last_t2 != _t2) {
          trans_input(&md2(atinput2, ithr, 0), &md3(ainput, 0, _ic4, 0), _t2, Tz);
        }
        gemm_non_acc(&md2(atoutput2, ithr, 0), &md2(atinput2, ithr, 0),
                     &md2(atweights2, ithr, 0), _t2, Tz, _ic4);
        trans_output(&md3(aoutput, 0, _oc4, 0), &md2(atoutput2, ithr, 0),
                     &md2(abias, _oc4, 0), _t2, Tz, _ic4);

        last_oc4 = _oc4; last_ic4 = _ic4; last_t2 = _t2;
      }
    }
  }
}

template <typename Type, const int A, const int K, const int V, const int I>
void elx_conv_wino_t<Type, A, K, V, I>::__execute_a079(
    Type * __restrict output, Type * __restrict input, Type * __restrict weights, Type * __restrict bias)
{
  MD2(Type, atinput2, tinput_, mthr_, A * A * this->T * this->ic3 * this->I2 * V);
  MD2(Type, atoutput2, toutput_, this->t2, this->oc4 * A * A * this->T * this->oc3 * this->O2 * V);
  MD2(Type, atweights2, tweights_, mthr_, A * A * this->ic3 * this->I2 * V * this->oc3 * this->O2 * V);

  MD3(Type, ainput, input, this->n, this->ic4, this->ih * this->iw * this->ic3 * this->I2 * V);
  MD3(Type, aoutput, output, this->n, this->oc4, this->oh * this->ow * this->oc3 * this->O2 * V);
  MD2(Type, abias, bias, this->oc4, this->oc3 * this->O2 * V);

  int last_ic4 = -1, last_t2 = -1, last_oc4 = -1;
#pragma omp parallel num_threads(mthr_) proc_bind(close) firstprivate(last_t2, last_ic4, last_oc4)
  iter_each(_ic4, this->ic4) {
#pragma omp for nowait collapse(2)
    iter_each(_t2, this->t2) {
      iter_each(_oc4, this->oc4) {
        int Tz = _t2 == (this->t2 - 1) ? this->Tr : this->T;
        size_t ithr = omp_get_thread_num();
        MD2(Type, atoutput3, &md2(atoutput2, _t2, 0), this->oc4, A * A * Tz * this->oc3 * this->O2 * V);

        if (last_ic4 != _ic4 || last_oc4 != _oc4) {
          trans_weightsf(&md2(atweights2, ithr, 0), weights, _ic4, _oc4);
        }
        if (last_ic4 != _ic4 || last_t2 != _t2) {
          trans_input(&md2(atinput2, ithr, 0), &md3(ainput, 0, _ic4, 0), _t2, Tz);
        }
        gemm(&md2(atoutput3, _oc4, 0), &md2(atinput2, ithr, 0),
             &md2(atweights2, ithr, 0), _t2, Tz, _ic4);
        if (_ic4 == this->ic4 - 1)
          trans_output(&md3(aoutput, 0, _oc4, 0), &md2(atoutput3, _oc4, 0),
                       &md2(abias, _oc4, 0), _t2, Tz, _ic4);

        last_oc4 = _oc4; last_ic4 = _ic4; last_t2 = _t2;
      }
    }
  }
}

// Flat mode
template <typename Type, const int A, const int K, const int V, const int I>
void elx_conv_wino_t<Type, A, K, V, I>::__execute_a000(
    Type * __restrict output, Type * __restrict input, Type * __restrict weights, Type * __restrict bias)
{
#pragma omp parallel num_threads(mthr_) proc_bind(close)
  {
    if (is_first_run_)
      trans_weights(tweights_, weights);
    trans_input(tinput_, input);
#pragma omp barrier
    gemm(toutput_, tinput_, tweights_);
#pragma omp barrier
    trans_output(output, toutput_, bias);
  }
  if (inference_acc_) is_first_run_ = false;
}

template <typename Type, const int A, const int K, const int V, const int I>
void elx_conv_wino_t<Type, A, K, V, I>::__execute_a010(
    Type * __restrict output, Type * __restrict input, Type * __restrict weights, Type * __restrict bias)
{
  MD3(Type, ainput, input, this->n, this->ic4, this->ih * this->iw * this->ic3 * this->I2 * V);
  MD2(Type, atweights, tweights_, this->ic4, A * A * this->ic3 * this->I2 * V * this->oc3 * this->O2 * V);

  if (is_first_run_) {
#pragma omp parallel num_threads(mthr_) proc_bind(close)
    trans_weights(tweights_, weights);
  }

#pragma omp parallel num_threads(mthr_) proc_bind(close)
  {
    iter_each(_ic4, this->ic4) {
      trans_input(tinput_, &md3(ainput, 0, _ic4, 0));
#pragma omp barrier
      gemm(toutput_, tinput_, &md2(atweights, _ic4, 0), _ic4);
#pragma omp barrier
    }
    trans_output(output, toutput_, bias);
  }

  if (inference_acc_) is_first_run_ = false;
}

template <typename Type, const int A, const int K, const int V, const int I>
void elx_conv_wino_t<Type, A, K, V, I>::execute(
    Type * __restrict output, Type * __restrict input, Type * __restrict weights, Type * __restrict bias)
{
  if (is_bfmt_)
    return (this->*execute_opt_)(output, input, weights, bias);
  else {
    Type *in = input;
    Type *wei = weights;
    Type *out = output_as_bfmt_ ? boutput_ : output;

    if (input_as_bfmt_) {
      MD5(Type, abinput, binput_, this->n, this->ic2, this->ih, this->iw, V);
      MD4(Type, ainput, input, this->n, this->ic, this->ih, this->iw);

#pragma omp parallel for collapse(3)
      iter_each (_n, this->n) {
      iter_each (_ic2, this->ic2) {
      iter_each (_ih, this->ih) {
        int v = _ic2 == this->ic2 - 1 ? this->Ir : V;
        iter_each (_iw, this->iw) {
#pragma omp simd
          iter_each (_v, v)
            md5(abinput, _n, _ic2, _ih, _iw, _v)
                = md4(ainput, _n, _ic2 * V + _v, _ih, _iw);
        }
      }}}
      in = binput_;
    }

    if (weights_as_bfmt_) {
      MD6(Type, abweights, bweights_, this->oc2, this->ic2, this->kh, this->kw, V, V);
      MD4(Type, aweights, weights, this->oc, this->ic, this->kh, this->kw);

#pragma omp parallel for collapse(3)
      iter_each (_oc2, this->oc2) {
      iter_each (_ic2, this->ic2) {
      iter_each (_kh, this->kh) {
        int iv = _ic2 == this->ic2 - 1 ? this->Ir : V;
        int ov = _oc2 == this->oc2 - 1 ? this->Or : V;
        iter_each (_kw, this->kw) {
          iter_each (_iv, iv) {
#pragma omp simd
            iter_each (_ov, ov) {
              md6(abweights, _oc2, _ic2, _kh, _kw, _iv, _ov)
                = md4(aweights, _oc2 * V + _ov, _ic2 * V + _iv, _kh, _kw);
            }
          }
        }
      }}}
      wei = bweights_;
    }

    // TODO: padding bias

    (this->*execute_opt_)(out, in, wei, bias);

    if (output_as_bfmt_) {
      MD5(Type, aboutput, boutput_, this->n, this->oc2, this->oh, this->ow, V);
      MD4(Type, aoutput, output, this->n, this->oc, this->oh, this->ow);

#pragma omp parallel for collapse(3)
      iter_each (_n, this->n) {
      iter_each (_oc2, this->oc2) {
      iter_each (_oh, this->oh) {
        int v = _oc2 == this->oc2 - 1 ? this->Or : V;
        if (this->with_ip_sum)
          iter_each (_V, v) {
            iter_each (_ow, this->ow) {
              md4(aoutput, _n, _oc2 * V + _V, _oh, _ow)
                += md5(aboutput, _n, _oc2, _oh, _ow, _V);
            }
          }
        else
          iter_each (_V, v) {
            iter_each (_ow, this->ow) {
              md4(aoutput, _n, _oc2 * V + _V, _oh, _ow)
                = md5(aboutput, _n, _oc2, _oh, _ow, _V);
            }
          }
      }}}
    }
  }
}

} // namespace euler
