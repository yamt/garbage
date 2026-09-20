# decode grpc length prefixed messages
#
# pkgin in protobuf
#
# export envoy module (a set of .proto files):
#   pkgin in buf
#   buf export buf.build/envoyproxy/envoy --output envoy-protos
#   buf export buf.build/envoyproxy/envoy:v1.36.0 --output envoy-protos

import subprocess
import struct
import sys

try:
    message_type = sys.argv[1]
except IndexError:
    message_type = None

decoders = {
    "ProcessingRequest": [
        "protoc",
        "-I",
        "envoy-protos",
        "envoy-protos/envoy/service/ext_proc/v3/external_processor.proto",
        "--decode=envoy.service.ext_proc.v3.ProcessingRequest",
    ],
    "ProcessingResponse": [
        "protoc",
        "-I",
        "envoy-protos",
        "envoy-protos/envoy/service/ext_proc/v3/external_processor.proto",
        "--decode=envoy.service.ext_proc.v3.ProcessingResponse",
    ],
}
default_decoder = ["protoc", "--decode_raw"]

decoder = decoders.get(message_type, default_decoder)

b = sys.stdin.buffer
i = 0
while True:
    hdr = b.read(5)
    if len(hdr) < 5:
        break
    compressed, length = struct.unpack(">bI", hdr)
    print(f"=== message {i} compressed {compressed} len {length}")
    payload = b.read(length)

    p = subprocess.run(decoder, input=payload, capture_output=True)
    print(p.stdout.decode(errors="replace") or p.stderr.decode(errors="replace"))

    i += 1
