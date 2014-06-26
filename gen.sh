#!/bin/bash
# avoid looping over an user ^C
trap "exit" INT

# handle arguments
if [ $# -lt 6 ]
then
	printf "Usage: %s l_string l_chains n_chains min_red max_red n_parts [min_part max_part]\n" $0
	printf "\n"
	printf "Parameters:\n"
	printf " l_string  string length\n"
	printf " l_chains  chain length\n"
	printf " n_chains  number of chains\n"
	printf " min_red   initial reduce seed (i.e. table index)\n"
	printf " max_red   final reduce seed\n"
	printf " min_part  initial part (i.e. table part)\n"
	printf " max_part  final part\n"
	exit 1
fi
l_string=$1
l_chains=$2
n_chains=$3
min_red=$4
max_red=$5
n_parts=$6
min_part=$7
max_part=$8
if [ $# -lt 8 ]
then
	min_part=0
	max_part=$(($n_parts-1))
fi

# base name of the files to be generated
name=alnum_${l_string}_${l_chains}_${n_chains}_${n_parts}

# create the files
mkdir -p rt/$name
for red in $(seq $min_red $max_red)
do
	for part in $(seq $min_part $max_part)
	do
		echo "Seed $red, part $part"
		nice ./rtgen $l_string $red $l_chains $n_chains $n_parts $part rt/$name/${name}_${red}_$part.rt
	done
done
