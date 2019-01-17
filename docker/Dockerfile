FROM alpine:3.8
LABEL maintainer="Konstantin Bläsi <kblaesi@gmail.com>"

ENV YASDI_VERSION 1.8.1
ENV LOGGER_VERSION 0.1.0

ENV YASDI_SRC /usr/src/yasdi
ENV LOGGER_SRC /usr/src/sma-yasdi-logger

# manually upgrade 3.8 to 3.9 until it is released to get newer g++
RUN sed -i -e 's/v3\.8/v3.9/g' /etc/apk/repositories && apk -U upgrade -a

# install build dependencies
RUN apk update && apk add --no-cache \
    binutils \
    cmake \
    ca-certificates \
    make \
    linux-headers \
    musl-dev \
    gcc \
    g++ \
    libgcc \
    libstdc++ \
    curl \
    mosquitto-libs \
    mosquitto-dev

RUN apk --no-cache add --repository http://dl-cdn.alpinelinux.org/alpine/edge/testing \
    flatbuffers \
    flatbuffers-dev

COPY . $LOGGER_SRC

# build and install yasdi
# debug build: cmake -D YASDI_DEBUG_OUTPUT=1 ..
RUN cd /usr/src \
    && curl -sSLo v$YASDI_VERSION.tar.gz https://github.com/konstantinblaesi/yasdi/archive/v$YASDI_VERSION.tar.gz \
    && mkdir $YASDI_SRC \
    && tar -xf v$YASDI_VERSION.tar.gz -C $YASDI_SRC --strip 1 \
    && rm -f v$YASDI_VERSION.tar.gz \
    && cd $YASDI_SRC \
    && patch -p1 < $LOGGER_SRC/docker/libc_musl.patch \
    && mkdir $YASDI_SRC/projects/generic-cmake/build \
    && cd $YASDI_SRC/projects/generic-cmake/build \
    && cmake -D CMAKE_INSTALL_PREFIX=/usr -D CMAKE_C_FLAGS="-DLIBC_MUSL" .. \
    && make -j$(getconf _NPROCESSORS_ONLN) \
    && make install

# build and install logger
#RUN curl -sSLo logger.tar.gz https://github.com/konstantinblaesi/sma-yasdi-logger/archive/master.tar.gz \
#    && mkdir $LOGGER_SRC \
#    && tar -xf logger.tar.gz -C $LOGGER_SRC --strip 1 && rm -f logger.tar.gz
WORKDIR $LOGGER_SRC
RUN ./build.sh
RUN mkdir $LOGGER_SRC/build
WORKDIR $LOGGER_SRC/build
RUN cmake -D CMAKE_INSTALL_PREFIX=/usr .. \
    && make -j$(getconf _NPROCESSORS_ONLN) \
    && make install \
    && rm -rf $YASDI_SRC \
    && rm -rf $LOGGER_SRC

# uninstall build dependencies
RUN apk del --purge \
    binutils \
    cmake \
    make \
    linux-headers \
    musl-dev \
    gcc \
    g++ \
    curl \
    mosquitto-dev \
    flatbuffers-dev

# RUN apk add

COPY ./docker/docker-entrypoint.sh /

STOPSIGNAL SIGTERM

ENTRYPOINT ["/docker-entrypoint.sh"]

CMD ["sma_logger"]
