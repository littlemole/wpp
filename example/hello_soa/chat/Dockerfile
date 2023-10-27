ARG BASE_IMAGE
FROM ${BASE_IMAGE}
LABEL me <little.mole@oha7.org>

ARG CXX=g++
ENV CXX=${CXX}

ARG EVENTLIB=
ENV BACKEND=${EVENTLIB}

RUN mkdir -p /usr/local/src/reproweb-example
ADD . /usr/local/src/reproweb-example

ENV USER_SERVICE_HOST=user
ENV SESSION_SERVICE_HOST=session
ENV REDIS_HOST=redis

#RUN mkdir /usr/local/share/ca-certificates/reprocpp && \
#    cp /usr/local/src/reproweb-example/pem/ca.crt /usr/local/share/ca-certificates/reprocpp/root.cert.crt && \
#    update-ca-certificates

RUN /usr/local/bin/install_cert

RUN bash -c 'cd /usr/local/src/reproweb-example && WITH_TEST=Off VERSION="${CXX}_${BACKEND}" /usr/local/bin/compile.sh reproweb-example'

CMD bash -c 'cd /usr/local/src/reproweb-example && Release/hello_world.bin'


