#ifndef __EULER_HPP__
#define __EULER_HPP__

#include <stddef.h>

namespace euler {

// Convolution algorithm
enum {
    CONV_AUTO = 0,
    CONV_DIRECT_1X1 = 1,
    CONV_DIRECT = 2,
    CONV_WINOGRAD = 3
};

// Desc setup error
enum {
    ELD_OK = 0,
    ELD_UNIMPLEMENTED = 1,
    ELD_GENERAL_ERROR = 2
};

// Execution error
enum {
    ELX_OK = 0,
    ELX_UNIMPLEMENTED = 1,
    ELX_GENERAL_ERROR = 2
};

// Streaming hint
enum {
  STORE_DEFAULT = 0,
  STORE_NORMAL = 1,
  STORE_STREAMING = 2
};

// Data formats
enum formats{
    nchw,
    nhwc,
    nChw16c,
    oihw,
    hwio,
    OIhw16i16o
};

// Propagation kind
enum prop_kinds {
    forward_training,
    forward_inference,
    backward_data,
    backward_weights
};

template<typename InputType, typename WeightsType,
    typename OutputType, typename BiasType>
struct elx_conv_t;

// Convolution desc
template<typename InputType, typename WeightsType,
    typename OutputType, typename BiasType>
struct eld_conv_t {
    // Conv parameters
    struct {
        struct { int n, c, h, w; } input;
        struct { int o, i, h, w; } weights;
        struct { int n, c, h, w; } output;
        struct { int c;          } bias;
    } dims;
    struct { int l, r, t, b; } pads;
    struct { int h, w; } strides;
    struct { int h, w; } dilations;

    // Data layout supported:
    // - plain: nchw, oihw, nchw
    // - blocked: nChw16c, OIhw16i16o, nChw16c
    struct { int input, weights, output; } formats;

    // propagation kind
    int prop_kind;

    // Algorithm
    int algorithm; // CONV_DIRECT | CONV_WINOGRAD
    int tile_size; // for Winograd only

    bool with_relu;
    bool with_bias;
    bool with_ip_sum;
    bool with_op_sum;
    bool is_inference;
    bool int8gemm;

    // Performance:
    // Number of thread teams, number of threads per team
    int nthreads;
    // Execution mode
    int execution_mode;
    // Flatting/Blocking/Partition
    struct { int o, t; } flatting;
    struct { int i, o; } blocking;
    struct { int i, o; } partition;
    // Streaming hint: STORE_DEFAULT | STORE_NORMAL | STORE_STREAMING
    struct { int weights, input, output; } streaming_hint;
    // Use blocked format internally for plain format
    struct { bool input, weights, output; } format_as_blocked;

    // Defaults
    eld_conv_t();
    ~eld_conv_t();
    int setup();
    void preprocess(WeightsType *weights);
    void clflush();

    // Auto computed by setup()
    struct { size_t input, weights, output, bias; } byte_sizes;
    struct { size_t input, weights, output, bias; } sizes;

    // Internal data used by elx
    elx_conv_t<InputType, WeightsType, OutputType, BiasType> *xc;
};

template struct eld_conv_t<float, float, float, float>;
//template struct eld_conv_t<short>;

// Convolution execution
template<typename InputType, typename WeightsType,
    typename OutputType, typename BiasType>
int elx_conv(eld_conv_t<InputType, WeightsType, OutputType, BiasType> &desc,
    OutputType *output, InputType *input, WeightsType *weights, BiasType *bias);

}

#endif // __EULER_HPP__
