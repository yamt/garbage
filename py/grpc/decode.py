# decode grpc length prefixed messages
#
# pkgin in protobuf

import subprocess
import struct
import sys

b = sys.stdin.buffer
i = 0
while True:
    hdr = b.read(5)
    if len(hdr) < 5:
        break
    (compressed, length) = struct.unpack(">bI", hdr)
    print(f"=== message {i} compressed {compressed} len {length}")
    payload = b.read(length)

    p = subprocess.run(["protoc", "--decode_raw"], input=payload, capture_output=True)
    print(p.stdout.decode(errors="replace") or p.stderr.decode(errors="replace"))

    i += 1
