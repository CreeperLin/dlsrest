FROM ubuntu
RUN apt-get update && apt-get install -yqq libcpprest
COPY build/src/server /workspace/
WORKDIR /workspace
