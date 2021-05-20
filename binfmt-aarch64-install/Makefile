ARCH=aarch64
NAME=yamt/binfmt-install-qemu-${ARCH}

# XXX my local environment
# see README about how to prepare the binary.
QEMU=~/git/qemu/build-static/qemu-${ARCH}

.PHONY: build
build:
	cp ${QEMU} dist/
	docker build \
		-t ${NAME} \
		.

.PHONY: push
push:
	docker push \
		${NAME}

.PHONY: run
run:
	docker run \
		--rm \
		-v $(QEMU):/yamt/qemu-${ARCH} \
		--privileged \
		${NAME}

.PHONY: run-with-default-qemu
run-with-default-qemu:
	docker run \
		--rm \
		--privileged \
		${NAME}

.PHONY: clean
clean:
	rm -f dist/*
