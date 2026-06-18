# attempt to install pg with asan support but not working
FROM debian:bookworm

COPY c_extension/i4r_audb_extension/badproxy_DONTDELETE /etc/apt/apt.conf.d/99fixbadproxy

RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    llvm \
    git \
    libc6-dev \
    libreadline-dev \
    zlib1g-dev \
    flex \
    bison

# Build Postgres with ASan
ENV CC=clang
ENV CFLAGS="-fsanitize=address -fno-omit-frame-pointer -g"
ENV LDFLAGS="-fsanitize=address"

RUN git clone https://github.com/postgres/postgres.git
WORKDIR /postgres
RUN git checkout REL_16_STABLE

RUN ./configure --enable-debug --enable-cassert
RUN make -j
RUN make install