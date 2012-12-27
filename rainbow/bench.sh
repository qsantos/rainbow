#!/bin/bash
cd $(dirname $0)
if [ $# -lt 3 ]
then
	printf "Usage: %s l_string n_tests src1 [src2 [...]]\n" $0
	exit 1
fi
l_string=$1
n_tests=$2

shift 2

echo $(
(
for i in $(seq $n_tests)
do
	string=$(dd if=/dev/urandom 2> /dev/null | tr -cd "[:digit:][:lower:]" | head -c $l_string)
	echo -n $string | md5sum | cut -d' ' -f1
done
) | bin/rtcrack -f - $@ | grep -Ec "^[a-z0-9]{32} "
) / $n_tests
