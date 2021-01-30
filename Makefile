all: run

build: webhello.c
	docker build -t ibmosquito/webhello_amd64:1.0.0 .

run: build
	-docker rm -f webhello 2>/dev/null
	docker run -d -p 80:80 -e MY_VAR=test --name webhello ibmosquito/webhello_amd64:1.0.0

push:
	docker push ibmosquito/webhello_amd64:1.0.0

check:
	curl -sS http://localhost:80
