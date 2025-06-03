#!/bin/bash


docker run -it --rm \
  -v /home/nick/Projects/homelab:/root/projects \
  imagefordrivers:v0.0.1 bash