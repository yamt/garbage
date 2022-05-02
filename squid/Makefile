NAME = yamt/squid

build:
	docker build -t $(NAME) .

run:
	docker container run -i -t -p 3128:3128 $(NAME)

test:
	curl --proxy http://localhost:3128 http://github.com/yamt/garbage
	curl --proxy http://localhost:3128 https://github.com/yamt/garbage
