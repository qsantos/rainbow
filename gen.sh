#!/bin/bash
trap "exit" INT
if [ $# -lt 4 ]
then
	printf "Usage: %s l_string l_chains n_chains PARTS\n" $0
	printf "\n"
	printf "PARTS  are arguments given to seq (e.g. '10', '0 9')\n"
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
	rtgen $l_string $i $l_chains $n_chains rt/alnum_${l_string}_${l_chains}_${n_chains}_$i.rt
	echo $i
done
