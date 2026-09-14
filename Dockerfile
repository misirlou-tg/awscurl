FROM ubuntu:noble

# Prevent interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Install ca-certificates and clean up apt lists to keep the image small
RUN apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates && \
    rm -rf /var/lib/apt/lists/*

COPY --chmod=755 build/awscurl /usr/bin

USER ubuntu:ubuntu

ENTRYPOINT ["/usr/bin/awscurl"]
