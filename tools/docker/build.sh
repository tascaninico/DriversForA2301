#!/bin/bash -x
#
# Script to build a Docker image which is then used as a container to build Pasolink.
#
# Syntax:
#     build.sh [IMAGE_NAME:(IMAGE_TAG)] (PUSH)
#
# Specify IMAGE_NAME as the name of the image to be built.
# Specify :IMAGE_TAG as the value to use when tagging the image (optional).
# Specify "PUSH" as the second command line parameter to push result to Aviat Registry (admin function).
#
# Set DOCKER_FILE to specify an alternative Dockerfile.
# Set DOCKER_PATH to specify an alternative context directory to upload to the Daemon.
#

set -o errexit
set -o nounset

#ROOT=$(readlink -f ./"$(git rev-parse --show-cdup)")

ROOT="/home/nick/Projects/homelab"
DOCKER_IMAGE_NAME=${DOCKER_IMAGE_NAME:-imagefordrivers}
DOCKER_IMAGE_TAG=${DOCKER_IMAGE_TAG:-v0.0.1}
DOCKER_IMAGE=${DOCKER_IMAGE:-$DOCKER_IMAGE_NAME:$DOCKER_IMAGE_TAG}
DOCKER_PATH=${DOCKER_PATH:-$ROOT/tools/docker}
DOCKER_FILE=${DOCKER_FILE:-${DOCKER_PATH}/${DOCKER_IMAGE_NAME}.Dockerfile}

# build image:
echo "Building $DOCKER_IMAGE"
docker build \
    --tag "$DOCKER_IMAGE" \
    --file "$DOCKER_FILE" \
    "$DOCKER_PATH"
