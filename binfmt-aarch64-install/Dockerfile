FROM alpine:3.12.0
ADD add.sh /add.sh
ADD add0.sh /add0.sh
ADD dist/qemu-aarch64 /yamt/qemu-aarch64
ADD dist/qemu-arm /yamt/qemu-arm
ENTRYPOINT ["/add0.sh"]
