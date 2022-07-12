#! /bin/sh

curl -L -o testfiles/model.xml https://github.com/intel/openvino-rs/raw/main/crates/openvino/tests/fixtures/mobilenet/mobilenet.xml
curl -L -o testfiles/model.bin https://github.com/intel/openvino-rs/raw/main/crates/openvino/tests/fixtures/mobilenet/mobilenet.bin
curl -L -o testfiles/tensor.bgr https://github.com/intel/openvino-rs/raw/main/crates/openvino/tests/fixtures/mobilenet/tensor-1x224x224x3-f32.bgr
