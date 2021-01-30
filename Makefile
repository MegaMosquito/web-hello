all: run

build: webhello.c
	docker build -t ibmosquito/web-hello_amd64:1.0.0 .

run: build
	-docker rm -f web-hello 2>/dev/null
	docker run -d -p 80:80 -e MY_VAR=test --name web-hello ibmosquito/web-hello_amd64:1.0.0

push:
	docker push ibmosquito/web-hello_amd64:1.0.0

check:
	curl -sS http://localhost:80
