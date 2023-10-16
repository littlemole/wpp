# This is a comment
FROM littlemole/reproweb_gpp_libevent_make
MAINTAINER me <little.mole@oha7.org>

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libhiredis-dev redis-server libsqlite3-dev

ARG CXX=g++
ENV CXX=${CXX}

ARG BACKEND=
ENV BACKEND=${BACKEND}

ARG BUILDCHAIN=make
ENV BUILDCHAIN=${BUILDCHAIN}

RUN /usr/local/bin/install.sh  repro-redis
RUN /usr/local/bin/install.sh  repro-sqlite

RUN mkdir -p /usr/local/src/reproweb-example
ADD . /usr/local/src/reproweb-example

RUN cd /usr/local/src/reproweb-example && (if [ "$BUILDCHAIN" != "cmake" ] ; then make -e; else (mkdir -p build && cd build && cmake .. && make) fi)

CMD bash -c "/etc/init.d/redis-server start && cd /usr/local/src/reproweb-example && build/test.bin"


