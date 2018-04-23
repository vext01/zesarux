#!/bin/bash

mkdir /tmp/pruebas
cp *.c /tmp/pruebas
cp *.rom /tmp/pruebas


for i in /tmp/pruebas/*; do
	echo "Running compress/uncompress test for $i"
	./zesarux --codetests $i
	if [ $? != 0 ]; then
		echo "ERROR"
		exit 1
	fi
done
