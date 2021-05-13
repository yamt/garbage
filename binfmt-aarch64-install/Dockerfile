FROM alpine:3.12.0
ADD add.sh /add.sh
ADD dist/qemu-aarch64 /yamt/qemu-aarch64
ENTRYPOINT ["/add.sh"]
