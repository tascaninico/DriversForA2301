docker run --rm -it \
  -v ~/Projects/homelab:/workdir \
  -w /workdir \
  -u 1000:1000 \
  ghcr.io/siemens/kas/kas:latest /bin/bash
