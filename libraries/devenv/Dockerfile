# This is a comment
FROM ubuntu:22.04
LABEL author <little.mole@oha7.org>

# std debian dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get upgrade -y
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  git sudo joe wget netcat psmisc net-tools \
  build-essential g++ pkg-config valgrind \
  libgtest-dev  openssl libssl-dev libevent-dev uuid-dev \
  nghttp2 libnghttp2-dev libcurl4-openssl-dev \
  clang libc++-dev libc++abi-dev redis-server \
  libboost-dev libboost-system-dev zlib1g-dev \
  libmysqlclient-dev sqlite3 libsqlite3-dev \
  libexpat-dev cmake nlohmann-json3-dev libkainjow-mustache-dev


# make specific cmake version
# ADD ./docker/cmake.sh /usr/local/bin/cmake.sh
# RUN /usr/local/bin/cmake.sh

# depend on compiler
ARG CXX=g++
ENV CXX=${CXX}

# depend on buildchain (optional as it should yield same results)
ARG BUILDCHAIN=cmake
ENV BUILDCHAIN=${BUILDCHAIN}

ARG SKIPTESTS=false
ENV SKIPTESTS=${SKIPTESTS}

ARG WITH_TEST=On
ENV WITH_TEST=${WITH_TEST}

# compile gtest with given compiler
#ADD ./docker/gtest.sh /usr/local/bin/gtest.sh
#RUN /usr/local/bin/gtest.sh

# compile jsoncpp with given compiler
#ADD ./docker/jsoncpp.sh /usr/local/bin/jsoncpp.sh
#RUN /usr/local/bin/jsoncpp.sh


# add little moles build & install helper scripts
ADD ./docker/build.sh /usr/local/bin/build.sh
ADD ./docker/install.sh /usr/local/bin/install.sh
ADD ./docker/compile.sh /usr/local/bin/compile.sh
ADD ./docker/mustache.sh /usr/local/bin/mustache.sh
ADD ./docker/utest.sh /usr/local/bin/utest.sh
ADD ./docker/install_cert /usr/local/bin/install_cert

RUN /usr/local/bin/utest.sh

# install little moles basic dev packages, for given compiler & buildchain
RUN /usr/local/bin/install.sh cryptoneat 
RUN /usr/local/bin/install.sh diy 
RUN /usr/local/bin/install.sh patex
RUN /usr/local/bin/install.sh metacpp
 
RUN /usr/local/bin/install.sh repro


