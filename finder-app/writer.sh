#!/bin/bash

writefile="$1"
writestr="$2"


if [ -z "${writefile}" ] || [ -z "${writestr}" ]; then
	echo "ERROR: Two arguements [writefile] [writestr] must be provided."
	exit 1
fi


echo "Try to make the path if it doesn't exist"
mkdir -p "$(dirname "$writefile")"

if ls "${writefile}" > /dev/null 2>&1; then
	echo "File exists."
else
	echo "File doesn't exist. Create now."
	if ! touch "${writefile}"; then
		echo "ERROR: faild to create ${writefile}."
		exit 1
	fi
fi


echo "Writing string ${writestr} to file ${writefile}"

if ! echo "${writestr}" > ${writefile}; then
	echo "ERROR: write failure."
	exit 1
else
	echo "Done!"
	exit 0
fi



