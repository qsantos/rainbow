#!/bin/bash
if [ $# -lt 4 ]
then
	printf "Usage: %s n_tests filename\n" $0
	exit 1
fi
n_tests=$1
filename=$2

C=0
T=0
for i in $(seq $n_tests)
do
	string=$(dd if=/dev/urandom 2> /dev/null | tr -cd "[:digit:][:lower:]" | head -c $slen)
	hash=$(echo -n $string | md5sum | cut -d' ' -f1)
	if bin/rtcrack $hash $filename 2>&1 > /dev/null
	then
		C=$(($C+1))
	fi
	T=$(($T+1))
	echo $C / $T
done
