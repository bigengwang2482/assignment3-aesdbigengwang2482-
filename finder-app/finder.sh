#!/bin/bash

filesdir="$1"
searchstr="$2"


if [ -z "${filesdir}" ] || [ -z "${searchstr}" ]; then
	echo "ERROR: Two arguements [filesdir] [searchstr] must be provided."
	exit 1
fi

if [ ! -d "$filesdir" ] || [ ! -e "$filesdir" ]; then
	echo "ERROR: The path $filesdir is not valid."
	exit 1
fi

num_files=$(grep -r -l "$searchstr" $filesdir | wc -l)
num_lines=$(grep -r -o "$searchstr" $filesdir | wc -l)
print_msg="The number of files are ${num_files} and the number of matching lines are ${num_lines}"
echo "${print_msg}"
