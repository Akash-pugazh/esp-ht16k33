#!/bin/sh
# Host test for the HT16K33 frame and 7-segment logic (no board). Run from anywhere:
#   sh components/ht16k33/test/host/run.sh
set -e
cd "$(dirname "$0")"
mkdir -p build
${CC:-cc} -std=c11 -Wall -Wextra -Werror -Istubs -I../../include -I../../src \
    ../../src/ht16k33_frame.c ../../src/ht16k33_seg.c test_ht16k33.c -o build/test_ht16k33
./build/test_ht16k33
