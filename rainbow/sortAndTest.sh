#!/bin/bash
if [ $# -lt 4 ]
then
	printf "Usage: %s slen l_chains filename currentSize\n" $0
	exit 1
fi
slen=$1
l_chains=$2
filename=$3
currentSize=$4

echo "\$ time ./rainbow $slen $l_chains rsize $currentSize $filename ${filename}_$currentSize" &&
time ./rainbow $slen $l_chains rsize $currentSize $filename ${filename}_$currentSize &&
echo "\$ time ./rainbow $slen $l_chains rtres ${filename}_$currentSize" &&
time ./rainbow $slen $l_chains rtres ${filename}_$currentSize &&
echo "\$ time ./rainbow $slen $l_chains tests 10000 ${filename}_$currentSize" &&
time ./rainbow $slen $l_chains tests 10000 ${filename}_$currentSize
