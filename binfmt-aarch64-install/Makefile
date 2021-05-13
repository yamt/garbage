NAME=yamt/binfmt-install-qemu-aarch64

# XXX my local environment
# see README about how to prepare the binary.
QEMU=~/git/qemu/build-static/qemu-aarch64

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
		-v $(QEMU):/yamt/qemu-aarch64 \
		--privileged \
		${NAME}

.PHONY: run-with-default-qemu
run-with-default-qemu:
	docker run \
		--privileged \
		${NAME}

.PHONY: clean
clean:
	rm -f dist/*
