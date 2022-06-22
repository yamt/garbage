NAME = yamt/squid
TS = $(shell TZ=UTC date "+%Y-%m-%d-%H-%M-%S")

CONF = squid-allowall.conf squid-basicauth.conf

ifneq ($(PROXY_USER),)
PROXY_AUTH_OPTIONS = --proxy-basic --proxy-user $(PROXY_USER):$(PROXY_PASS)
endif

build: $(CONF)
	docker build -t $(NAME) .

%.conf: m4/%.conf m4/common.conf
	m4 -I m4 $< > $@.tmp
	mv $@.tmp $@

push:
	docker tag $(NAME) $(NAME):$(TS)
	docker push $(NAME):$(TS)
	docker push $(NAME)

run:
	docker container run -i -t -p 3128:3128 \
	-e PROXY_USER \
	-e PROXY_PASS \
	$(NAME)

test:
	mkdir -p out
	curl -o out/https.txt \
	--proxy http://localhost:3128 \
	$(PROXY_AUTH_OPTIONS) \
	https://raw.githubusercontent.com/yamt/garbage/master/squid/testfile.txt
	diff -u testfile.txt out/https.txt

	# Note: github redirects http to https
	curl -L -o out/http.txt \
	--proxy http://localhost:3128 \
	$(PROXY_AUTH_OPTIONS) \
	http://raw.githubusercontent.com/yamt/garbage/master/squid/testfile.txt
	diff -u testfile.txt out/http.txt

	curl -L -o out/http-tunnel.txt \
	--proxy http://localhost:3128 \
	$(PROXY_AUTH_OPTIONS) \
	--proxytunnel \
	http://raw.githubusercontent.com/yamt/garbage/master/squid/testfile.txt
	diff -u testfile.txt out/http-tunnel.txt

clean:
	rm -rf $(CONF)
	rm -rf out

