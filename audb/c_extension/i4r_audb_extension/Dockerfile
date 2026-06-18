FROM postgres:16-bookworm

# error on Mac no matter the attempted changes. This 'magically' fixes it
# https://forums.docker.com/t/hash-sum-mismatch-writing-more-data-as-expected/45940/3
COPY ./badproxy_DONTDELETE /etc/apt/apt.conf.d/99fixbadproxy

RUN apt-get update && apt-get install -y \
    build-essential \
    postgresql-server-dev-16 \
    git \
    make \
    gcc \
    libc6-dev 

WORKDIR /audbExtension
