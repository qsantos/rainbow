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

name=alnum_${l_string}_${l_chains}_${n_chains}

shift 3

if [ ! -e rt ]
then
	mkdir rt
fi
if [ ! -e rt/$name ]
then
	mkdir rt/$name
fi

for i in $(seq $@)
do
	bin/rtgen $l_string $i $l_chains $n_chains rt/${name}/${name}_$i.rt
	echo $i
done
