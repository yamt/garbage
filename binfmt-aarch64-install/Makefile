# XXX this name is historical
NAME=yamt/binfmt-install-qemu-aarch64

# XXX my local environment
# see README about how to prepare the binary.
QEMU_DIR=~/git/qemu/build-static/

.PHONY: build
build:
	cp ${QEMU_DIR}/qemu-aarch64 dist/
	cp ${QEMU_DIR}/qemu-arm dist/
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
		-v $(QEMU_DIR):/yamt \
		--privileged \
		${NAME}

.PHONY: run-with-default-qemu
run-with-default-qemu:
	docker run \
		--rm \
		--privileged \
		${NAME}

.PHONY: test
test:
	./test.sh
	./test-dind.sh

.PHONY: clean
clean:
	rm -f dist/*
