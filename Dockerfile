FROM ubuntu:latest
RUN apt update && apt install -y build-essential
WORKDIR /
COPY webhello.c /
RUN gcc webhello.c -o /webhello
CMD /webhello
