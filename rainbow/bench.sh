#!/bin/bash
if [ $# -lt 3 ]
then
	printf "Usage: %s l_string n_tests src1 [src2 [...]]\n" $0
	exit 1
fi
l_string=$1
n_tests=$2

shift 2

C=0
T=0
for i in $(seq $n_tests)
do
	string=$(dd if=/dev/urandom 2> /dev/null | tr -cd "[:digit:][:lower:]" | head -c $l_string)
	hash=$(echo -n $string | md5sum | cut -d' ' -f1)
	if bin/rtcrack $hash $@ 2>&1 > /dev/null
	then
		C=$(($C+1))
	fi
	T=$(($T+1))
	echo $C / $T
done
