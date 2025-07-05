FROM ubuntu:noble

RUN \
apt-get update && \
apt-get install -y ca-certificates

COPY --chmod=755 build/awscurl /usr/bin

USER ubuntu:ubuntu

ENTRYPOINT ["/usr/bin/awscurl"]
