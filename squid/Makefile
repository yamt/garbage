NAME = yamt/squid

build:
	docker build -t $(NAME) .

run:
	docker container run -i -t -p 3128:3128 $(NAME)

test:
	mkdir -p out
	curl -o out/https.txt \
	--proxy http://localhost:3128 \
	--proxy-basic \
	--proxy-user aaa:bbb \
	https://raw.githubusercontent.com/yamt/garbage/master/squid/testfile.txt
	diff -u testfile.txt out/https.txt

	# Note: github redirects http to https
	curl -L -o out/http.txt \
	--proxy http://localhost:3128 \
	--proxy-basic \
	--proxy-user aaa:bbb \
	http://raw.githubusercontent.com/yamt/garbage/master/squid/testfile.txt
	diff -u testfile.txt out/http.txt

	curl -L -o out/http-tunnel.txt \
	--proxy http://localhost:3128 \
	--proxy-basic \
	--proxy-user aaa:bbb \
	--proxytunnel \
	http://raw.githubusercontent.com/yamt/garbage/master/squid/testfile.txt
	diff -u testfile.txt out/http-tunnel.txt

clean:
	rm -rf out
