NAME=ubuntu

.PHONY: load load-amd64 load-arm64 load-armv7
load: load-amd64 load-arm64 load-armv7

load-amd64:
	docker buildx build \
		--platform linux/amd64 \
		-t yamt/$(NAME)-amd64:latest \
		--load \
		.
	docker image tag \
		yamt/$(NAME)-amd64:latest \
		yamt/$(NAME):latest

load-arm64:
	docker buildx build \
		--platform linux/arm64 \
		-t yamt/$(NAME)-arm64:latest \
		--load \
		.
	docker image tag \
		yamt/$(NAME)-arm64:latest \
		yamt/$(NAME):latest

load-armv7:
	docker buildx build \
		--platform linux/arm/v7 \
		-t yamt/$(NAME)-armv7:latest \
		--load \
		.
	docker image tag \
		yamt/$(NAME)-armv7:latest \
		yamt/$(NAME):latest

.PHONY: push
push:
	docker buildx build \
		--platform linux/amd64,linux/arm64,linux/arm/v7 \
		-t yamt/$(NAME):latest \
		--push \
		.
