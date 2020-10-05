FROM alpine:3.8

WORKDIR "/compile"

COPY . /compile

RUN apk update && apk add build-base unzip wget cmake libstdc++ boost-dev git && \
	cmake . && make && make install && \
	apk --purge del build-base unzip wget cmake git boost-dev && \
	rm -rf /cache/apk/* && cd / && rm -rf /compile

WORKDIR "/"
