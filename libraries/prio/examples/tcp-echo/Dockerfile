FROM littlemole/priocpp_clangpp_boost_asio_make
MAINTAINER me <little.mole@oha7.org>

ARG CXX=g++
ARG BACKEND=libevent

ENV CXX=${CXX}
ENV BACKEND=${BACKEND}

RUN cd /usr/local/src && \
  git clone https://github.com/littlemole/prio.git && \
  cd prio && \
  make clean && \
  make CXX=${CXX} && \
  make CXX=${CXX} test && \
  make CXX=${CXX} install 

RUN mkdir -p /opt/workspace/tcp-echo

ADD test.sh /usr/local/bin/test.sh
CMD ["/bin/bash", "/usr/local/bin/test.sh"]
