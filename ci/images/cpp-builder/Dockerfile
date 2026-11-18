# Canonical reusable image: ci-foundry/cpp-qt6-ubuntu2404:qt6-ubuntu24.04-v1
# Recipe only: image must be built, scanned, and digest-pinned before use.
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates cmake g++ pkg-config libglib2.0-dev qt6-base-dev \
 && rm -rf /var/lib/apt/lists/*
RUN cmake --version && g++ --version && pkg-config --modversion Qt6Core
