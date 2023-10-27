ARG BASE_IMAGE
FROM ${BASE_IMAGE}
LABEL me <little.mole@oha7.org>

ARG CXX=g++
ENV CXX=${CXX}

ARG EVENTLIB=libevent
ENV BACKEND=${EVENTLIB}

ARG MODE=Release
ENV MODE=${MODE}
 
ARG WITH_TEST=On
ENV WITH_TEST=${WITH_TEST}

ARG WITH_DEBUG=Off
ENV WITH_DEBUG=${WITH_DEBUG}

ADD . /usr/local/src/wpp

RUN cd /usr/local/src && /usr/local/bin/build.sh wpp
