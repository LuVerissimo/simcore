#!/bin/bash
docker run --rm -v $(pwd):/src -w /src nvidia/cuda:12.8.0-devel-ubuntu22.04 \
  bash -c "apt update -qq && apt install -y -qq python3-pip git > /dev/null 2>&1 && \
  pip3 install -q cmake && cmake -B build-cuda -DSIMCORE_CUDA=ON && cmake --build build-cuda"
