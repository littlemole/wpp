# This is a comment
ARG BASE_IMAGE
FROM ${BASE_IMAGE}
LABEL me <little.mole@oha7.org>

ARG CXX=g++
ENV CXX=${CXX}

ARG BACKEND=
ENV BACKEND=${BACKEND}

ARG BUILDCHAIN=cmake
ENV BUILDCHAIN=${BUILDCHAIN}

ARG TS=
ENV TS=${TS}

RUN mkdir -p /usr/local/src/reproweb-example
ADD . /usr/local/src/reproweb-example

ENV USER_SERVICE_HOST=user
ENV SESSION_SERVICE_HOST=session

RUN mkdir /usr/local/share/ca-certificates/reprocpp && \
    cp /usr/local/src/reproweb-example/pem/ca.crt /usr/local/share/ca-certificates/reprocpp/root.cert.crt && \
    update-ca-certificates


#RUN cd /usr/local/src/reproweb-example && (if [ "$BUILDCHAIN" != "cmake" ] ; then make -e; else (mkdir -p build && cd build && cmake .. && make) fi)

RUN bash -c 'cd /usr/local/src/reproweb-example && SKIPTESTS=true VERSION="${CXX}_${BACKEND}_${BUILDCHAIN}" /usr/local/bin/compile.sh reproweb-example'

CMD bash -c 'cd /usr/local/src/reproweb-example && (if [ "$BUILDCHAIN" != "cmake" ] ; then build/hello_world.bin; else Release/hello_world.bin; fi )'


