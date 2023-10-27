ARG BASE_IMAGE
FROM ${BASE_IMAGE}
LABEL me <little.mole@oha7.org>

ARG CXX=g++
ENV CXX=${CXX}

ARG EVENTLIB=
ENV BACKEND=${EVENTLIB}

RUN mkdir -p /usr/local/src/reproweb-example
ADD ./CMakeLists.txt /usr/local/src/reproweb-example/

ADD ./config.json /usr/local/src/reproweb-example/
ADD ./*.h /usr/local/src/reproweb-example/
ADD ./*.cpp /usr/local/src/reproweb-example/
ADD ./user.db /usr/local/src/reproweb-example/
ADD ./mime.types /usr/local/src/reproweb-example/
ADD ./Makefile /usr/local/src/reproweb-example/

RUN mkdir -p /usr/local/src/reproweb-example/htdocs
ADD ./htdocs /usr/local/src/reproweb-example/htdocs

RUN mkdir -p /usr/local/src/reproweb-example/locale
ADD ./locale /usr/local/src/reproweb-example/locale

RUN mkdir -p /usr/local/src/reproweb-example/view
ADD ./view /usr/local/src/reproweb-example/view

RUN mkdir -p /usr/local/src/reproweb-example/pem
ADD ./pem /usr/local/src/reproweb-example/pem

ENV REDIS_HOST=redis

RUN bash -c 'cd /usr/local/src/reproweb-example && WITH_TEST=Off VERSION="${CXX}_${BACKEND}" /usr/local/bin/compile.sh reproweb-example'

CMD bash -c 'cd /usr/local/src/reproweb-example && Release/hello_world.bin'


