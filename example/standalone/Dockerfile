# This is a comment
FROM ubuntu:24.04 as build
LABEL author <little.mole@oha7.org>

# std debian dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get upgrade -y
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  git sudo joe wget netcat-traditional psmisc net-tools \
  build-essential g++ pkg-config valgrind \
  libgtest-dev  openssl libssl-dev libevent-dev uuid-dev \
  nghttp2 libnghttp2-dev libcurl4-openssl-dev \
  clang libc++-dev libc++abi-dev redis-server \
  libboost-dev libboost-system-dev zlib1g-dev \
  libmysqlclient-dev sqlite3 libsqlite3-dev \
  libexpat-dev cmake nlohmann-json3-dev libkainjow-mustache-dev


ARG CXX=g++
ENV CXX=${CXX}

ARG EVENTLIB=
ENV BACKEND=${EVENTLIB}

# add little moles build & install helper scripts
ADD ./docker/build.sh /usr/local/bin/build.sh
ADD ./docker/install.sh /usr/local/bin/install.sh
ADD ./docker/compile.sh /usr/local/bin/compile.sh
ADD ./docker/mustache.sh /usr/local/bin/mustache.sh
ADD ./docker/utest.sh /usr/local/bin/utest.sh

RUN /usr/local/bin/utest.sh

# install little moles basic dev packages, for given compiler
RUN /usr/local/bin/install.sh cryptoneat 
RUN /usr/local/bin/install.sh diy 
RUN /usr/local/bin/install.sh patex
RUN /usr/local/bin/install.sh metacpp
 
RUN /usr/local/bin/install.sh repro

RUN /usr/local/bin/install.sh wpp

RUN mkdir -p /usr/local/src/reproweb-example
ADD . /usr/local/src/reproweb-example

# build the webapp
RUN bash -c 'cd /usr/local/src/reproweb-example && WITH_TEST=Off VERSION="${CXX}_${BACKEND}" /usr/local/bin/compile.sh reproweb-example'



FROM ubuntu:22.04 

# std debian dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get upgrade -y
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  openssl libevent-2.1-7 uuid \
  nghttp2 libcurl4 \
  zlib1g \
  sqlite3 libevent-pthreads-2.1-7  \
  libexpat1 

#  joe wget netcat psmisc net-tools libboost-system1.74.0  \


COPY --from=build usr/local/src/reproweb-example /usr/local/src/reproweb-example
# run the webapp
CMD bash -c 'cd /usr/local/src/reproweb-example && Release/hello_standalone.bin'


