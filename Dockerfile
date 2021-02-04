FROM ubuntu:18.04
RUN apt update && apt install -y build-essential
RUN apt install -y curl
WORKDIR /
COPY webhello.c /
RUN gcc webhello.c -o /webhello
CMD /webhello
