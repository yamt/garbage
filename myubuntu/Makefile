.PHONY: load load-amd64 load-arm64 load-armv7
load: load-amd64 load-arm64 load-armv7

load-amd64:
	docker buildx build \
		--platform linux/amd64 \
		-t yamt/ubuntu-amd64:latest \
		--load \
		.
	docker image tag \
		yamt/ubuntu-amd64:latest \
		yamt/ubuntu:latest

load-arm64:
	docker buildx build \
		--platform linux/arm64 \
		-t yamt/ubuntu-arm64:latest \
		--load \
		.
	docker image tag \
		yamt/ubuntu-arm64:latest \
		yamt/ubuntu:latest

load-armv7:
	docker buildx build \
		--platform linux/arm/v7 \
		-t yamt/ubuntu-armv7:latest \
		--load \
		.
	docker image tag \
		yamt/ubuntu-armv7:latest \
		yamt/ubuntu:latest

.PHONY: push
push:
	docker buildx build \
		--platform linux/amd64,linux/arm64,linux/arm/v7 \
		-t yamt/ubuntu:latest \
		--push \
		.
