NAME=wamrc

.PHONY: load load-amd64
load: load-amd64

load-amd64:
	docker buildx build \
		--platform linux/amd64 \
		-t yamt/$(NAME)-amd64:latest \
		--load \
		.
	docker image tag \
		yamt/$(NAME)-amd64:latest \
		yamt/$(NAME):latest

.PHONY: push
push:
	docker buildx build \
		--platform linux/amd64 \
		-t yamt/$(NAME):latest \
		--push \
		.
