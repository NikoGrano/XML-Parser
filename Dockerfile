FROM alpine:latest

WORKDIR "/compile"

COPY . /compile

RUN apk update && apk add build-base unzip wget cmake libstdc++ boost-dev git && \
	cmake . && make -j $(nproc --all) && make install && \
        mv bin/server /bin/server && \
	apk --purge del build-base unzip wget cmake git boost-dev && \
	rm -rf /cache/apk/* && cd / && rm -rf /compile

WORKDIR "/"
