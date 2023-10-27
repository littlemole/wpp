# This is a comment
ARG BASE_IMAGE
FROM ${BASE_IMAGE}
LABEL me <little.mole@oha7.org>

#RUN DEBIAN_FRONTEND=noninteractive apt-get install -y mysql-client libmysqlclient-dev

ARG CXX=g++
ENV CXX=${CXX}

ARG EVENTLIB=
ENV BACKEND=${EVENTLIB}

ARG BUILDCHAIN=make
ENV BUILDCHAIN=${BUILDCHAIN}

ARG TS=
ENV TS=${TS}

#ENV REDIS_HOST=redis

#RUN  /usr/local/bin/install.sh  repro-mysql 

RUN mkdir -p /usr/local/src/reproweb-example
ADD . /usr/local/src/reproweb-example

#RUN mkdir /usr/local/share/ca-certificates/reprocpp && \
#    cp /usr/local/src/reproweb-example/pem/ca.crt /usr/local/share/ca-certificates/reprocpp/root.cert.crt && \
 #   update-ca-certificates

RUN /usr/local/bin/install_cert

#RUN cd /usr/local/src/reproweb-example && (if [ "$BUILDCHAIN" != "cmake" ] ; then make -e; else (mkdir -p build && cd build && cmake .. && make) fi)

RUN bash -c 'cd /usr/local/src/reproweb-example && WITH_TEST=true VERSION="${CXX}_${BACKEND}_${BUILDCHAIN}" /usr/local/bin/compile.sh reproweb-example'

CMD bash -c 'cd /usr/local/src/reproweb-example && Release/hello_soa_userservice.bin'




