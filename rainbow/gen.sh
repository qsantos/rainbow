#!/bin/bash
cd $(dirname $0)
if [ $# -lt 3 ]
then
	printf "Usage: %s l_string l_chains n_chains n_parts\n" $0
	exit 1
fi
l_string=$1
l_chains=$2
n_chains=$3

shift 3

for i in $(seq $@)
do
	bin/rtgen $l_string $i $l_chains $n_chains alnum_${l_strings}_${l_chains}_${n_chains}_$i.rt
	echo $i
done
