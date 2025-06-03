# Use official Ubuntu base image
FROM ubuntu:22.04

# Set home and update PATH so Rust tools are accessible
ENV HOME=/root
ENV PATH=$HOME/.cargo/bin:$PATH

# Prevent apt from prompting for input (useful in Docker builds)
ENV DEBIAN_FRONTEND=noninteractive

# Install development tools for C and Rust
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    make \
    curl \
    gdb \
    git \
    pkg-config \
    libssl-dev \
    ca-certificates \
    python3 \
    python3-pip \
    && pip3 install lizard \
    && rm -rf /var/lib/apt/lists/*    

# Install Rust via rustup (installs to /root/.cargo by default)
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y

# ------------------------------------------------------------------------------
# Create a project directory inside the container image
# This is useful if:
# - You want the container to work even WITHOUT a mounted volume
# - You want to COPY code into the image during build
# - You want a predictable place to land inside the container
# ------------------------------------------------------------------------------
RUN mkdir -p /root/projects

# Set working directory for all following RUN/COPY commands and shell sessions
# This makes sure you land in /root/projects when you run the container interactively
WORKDIR /root/projects
