# This is a comment
ARG BASE_IMAGE=${BASE_IMAGE}
FROM ${BASE_IMAGE}
LABEL me <little.mole@oha7.org>

#RUN DEBIAN_FRONTEND=noninteractive apt-get install -y redis-server libsqlite3-dev libnss3-tools

ARG CXX=g++
ENV CXX=${CXX}

ARG EVENTLIB=
ENV BACKEND=${EVENTLIB}

RUN mkdir -p /usr/local/src/reproweb-example
ADD . /usr/local/src/reproweb-example

RUN bash -c 'find /usr/local/src/reproweb-example/ca/ca -name "*.key" -exec rm {} \;'

ENV REDIS_HOST=redis

RUN bash -c 'cd /usr/local/src/reproweb-example && WITH_TEST=Off VERSION="${CXX}_${BACKEND}" /usr/local/bin/compile.sh reproweb-example'
CMD bash -c 'cd /usr/local/src/reproweb-example && Release/hello_world.bin'


