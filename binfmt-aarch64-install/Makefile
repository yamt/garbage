NAME=binfmt-install-qemu-aarch64

# XXX my local environment
QEMU=~/git/qemu/build-static/qemu-aarch64

.PHONY: build install

build:
	docker build \
		-t ${NAME} \
		.

run:
	docker run \
		-v $(QEMU):/usr/bin/qemu-aarch64 \
		--privileged \
		${NAME}
