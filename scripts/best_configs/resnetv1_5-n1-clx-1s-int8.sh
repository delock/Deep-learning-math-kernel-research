#/bin/bash

# Resnet50
# batch-size: 1
# SKX 8180 1S

source ./scripts/best_configs/common.sh $@
COMMON="$COMMON "

echo "Compute conv: 7x7"
# conv2d
NSOCKETS=1 ./scripts/run.sh -c -i3 -h224 -o64 -H112 -k7 -K7 -s2 -S2 -p3 -P3 -n1 -adirect --execution-mode=0xa160 --blk-i=1 --blk-o=2 --flt-o=2 --flt-t=14 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32z --sampling-kind=2 --input-format=nhwc --output-format=nhwc --weights-format=OIhw16i16o

echo "Compute conv: 3x3 s1"
# resnet50_res2a_branch2b, conv2d_3, conv2d_6, conv2d_9
NSOCKETS=1 ./scripts/run.sh -c -i64 -h56 -o64 -H56 -n1 -adirect --execution-mode=0xa160 --blk-i=4 --blk-o=1 --flt-o=2 --flt-t=14 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2
# resnet50_res3a_branch2b, conv2d_16, conv2d_19, conv2d_22
NSOCKETS=1 ./scripts/run.sh -c -i128 -h28 -o128 -H28 -n1 -adirect --execution-mode=0xa160 --blk-i=8 --blk-o=2 --flt-o=2 --flt-t=14 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2
# resnet50_res4a_branch2b, conv2d_29, conv2d_32, conv2d_35, conv2d_38, conv2d_41
NSOCKETS=1 ./scripts/run.sh -c -i256 -h14 -o256 -H14 -n1 -adirect --execution-mode=0xa160 --blk-i=16 --blk-o=1 --flt-o=2 --flt-t=14 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2
# resnet50_res5a_branch2b, conv2d_48, conv2d_51
NSOCKETS=1 ./scripts/run.sh -c -i512 -h7 -o512 -H7 -n1 -adirect --execution-mode=0xa160 --blk-i=32 --blk-o=2 --flt-o=2 --flt-t=7 --pat-i=1 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2

echo "Compute conv: 3x3 s2"
# conv2d_13
NSOCKETS=1 ./scripts/run.sh -c -i128 -h58 -o128 -H28 -k3 -K3 -s2 -S2 -p0 -P0 -n1 -adirect --execution-mode=0xa160 --blk-i=8 --blk-o=2 --flt-o=2 --flt-t=14 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2
# conv2d_26
NSOCKETS=1 ./scripts/run.sh -c -i256 -h30 -o256 -H14 -k3 -K3 -s2 -S2 -p0 -P0 -n1 -adirect --execution-mode=0xa160 --blk-i=16 --blk-o=2 --flt-o=2 --flt-t=14 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2
# conv2d_45
NSOCKETS=1 ./scripts/run.sh -c -i512 -h16 -o512 -H7 -k3 -K3 -s2 -S2 -p0 -P0 -n1 -adirect --execution-mode=0xa160 --blk-i=32 --blk-o=2 --flt-o=2 --flt-t=7 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2

echo "Compute conv: 1x1 s2"
# conv2d_11
NSOCKETS=1 ./scripts/run.sh -c -i256 -h56 -o512 -H28 -k1 -K1 -s2 -S2 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xb161 --blk-i=16 --blk-o=8 --flt-o=2 --flt-t=7 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2
# conv2d_24
NSOCKETS=1 ./scripts/run.sh -c -i512 -h28 -o1024 -H14 -k1 -K1 -s2 -S2 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xb161 --blk-i=32 --blk-o=8 --flt-o=2 --flt-t=7 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2
# conv2d_43
NSOCKETS=1 ./scripts/run.sh -c -i1024 -h14 -o2048 -H7 -k1 -K1 -s2 -S2 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xb161 --blk-i=64 --blk-o=8 --flt-o=2 --flt-t=7 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2

echo "Compute conv: 1x1 s1 h56"
# conv2d_2
NSOCKETS=1 ./scripts/run.sh -c -i64 -h56 -o64 -H56 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=4 --blk-o=2 --flt-o=2 --flt-t=12 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2
# conv2d_1, conv2d_4, conv2d_7, conv2d_10
NSOCKETS=1 ./scripts/run.sh -c -i64 -h56 -o256 -H56 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=4 --blk-o=4 --flt-o=2 --flt-t=12 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2
# conv2d_5, conv2d_8
NSOCKETS=1 ./scripts/run.sh -c -i256 -h56 -o64 -H56 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=16 --blk-o=2 --flt-o=2 --flt-t=12 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2
# conv2d_12
NSOCKETS=1 ./scripts/run.sh -c -i256 -h56 -o128 -H56 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=16 --blk-o=2 --flt-o=2 --flt-t=12 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2

echo "Compute conv: 1x1 s1 h28"
# conv2d_14, conv2d_17, conv2d_20, conv2d_23
NSOCKETS=1 ./scripts/run.sh -c -i128 -h28 -o512 -H28 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=8 --blk-o=4 --flt-o=2 --flt-t=12 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2
# conv2d_15, conv2d_18, conv2d_21
NSOCKETS=1 ./scripts/run.sh -c -i512 -h28 -o128 -H28 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=32 --blk-o=4 --flt-o=2 --flt-t=12 --pat-i=1 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2 --output-format=nhwc
# conv2d_25
NSOCKETS=1 ./scripts/run.sh -c -i512 -h28 -o256 -H28 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=32 --blk-o=4 --flt-o=4 --flt-t=6 --pat-i=1 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2

echo "Compute conv: 1x1 s1 h14"
# conv2d_27, conv2d_30, conv2d_33, conv2d_36, conv2d_39, conv2d_42
NSOCKETS=1 ./scripts/run.sh -c -i256 -h14 -o1024 -H14 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=16 --blk-o=4 --flt-o=4 --flt-t=6 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2 --input-format=nChw16c --output-format=nhwc
# conv2d_28, conv2d_31, conv2d_34, conv2d_37, conv2d_40
NSOCKETS=1 ./scripts/run.sh -c -i1024 -h14 -o256 -H14 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=64 --blk-o=2 --flt-o=4 --flt-t=6 --pat-i=1 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2 --input-format=nhwc --output-format=nhwc
# conv2d_44
NSOCKETS=1 ./scripts/run.sh -c -i1024 -h14 -o512 -H14 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=64 --blk-o=2 --flt-o=4 --flt-t=6 --pat-i=1 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2 --input-format=nhwc --output-format=nhwc

echo "Compute conv: 1x1 s1 h7"
# conv2d_46, conv2d_49, conv2d_52
NSOCKETS=1 ./scripts/run.sh -c -i512 -h7 -o2048 -H7 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=32 --blk-o=4 --flt-o=4 --flt-t=6 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2 --input-format=nChw16c --output-format=nhwc
# conv2d_47, conv2d_50
NSOCKETS=1 ./scripts/run.sh -c -i2048 -h7 -o512 -H7 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect_1x1 --execution-mode=0xc160 --blk-i=128 --blk-o=2 --flt-o=4 --flt-t=6 --pat-i=1 --pat-o=1 $COMMON --data-type-cfg=U8F32S8F32 --sampling-kind=2 --input-format=nhwc --output-format=nhwc

echo "Compute gemm: 1x2048, 2048x1001"
# FC
NSOCKETS=1 ./scripts/run.sh -c -i2048 -h1 -o1008 -H1 -k1 -K1 -s1 -S1 -p0 -P0 -n1 -adirect --execution-mode=0xd160 --blk-i=128 --blk-o=1 --flt-o=3 --flt-t=1 --pat-i=1 --pat-o=1 $COMMON --data-type-cfg=U8F32F32F32 --sampling-kind=2 --input-format=nhwc --output-format=nhwc
