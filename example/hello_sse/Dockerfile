# This is a comment
ARG BASE_IMAGE
FROM ${BASE_IMAGE}
LABEL me <little.mole@oha7.org>

#RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libsqlite3-dev

ARG CXX=g++
ENV CXX=${CXX}

ARG BACKEND=
ENV BACKEND=${BACKEND}

ARG BUILDCHAIN=make
ENV BUILDCHAIN=${BUILDCHAIN}

#RUN /usr/local/bin/install.sh  repro-sqlite

RUN mkdir -p /usr/local/src/reproweb-example
ADD . /usr/local/src/reproweb-example

ENV REDIS_HOST=redis

#RUN cd /usr/local/src/reproweb-example && (if [ "$BUILDCHAIN" != "cmake" ] ; then make -e; else (mkdir -p build && cd build && cmake .. && make) fi)

RUN bash -c 'cd /usr/local/src/reproweb-example && SKIPTESTS=true VERSION="${CXX}_${BACKEND}_${BUILDCHAIN}" /usr/local/bin/compile.sh reproweb-example'

CMD bash -c 'cd /usr/local/src/reproweb-example && (if [ "$BUILDCHAIN" != "cmake" ] ; then build/hello_world.bin; else Release/hello_world.bin; fi )'


