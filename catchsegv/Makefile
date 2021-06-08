native:
	docker build -t catchsegv-example .
	docker run catchsegv-example

arm64:
	docker build \
		--platform linux/arm64 \
		-t catchsegv-example .
	docker run \
		--platform linux/arm64 \
		catchsegv-example

arm:
	docker build \
		--platform linux/arm/v7 \
		-t catchsegv-example .
	docker run \
		--platform linux/arm/v7 \
		catchsegv-example
