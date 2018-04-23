#!/bin/bash

./zesarux --codetests

mkdir /tmp/pruebas
cp *.c /tmp/pruebas


for i in /tmp/pruebas/*.c; do
	echo "Running compress/uncompress test for $i"
	./zesarux --codetests $i
done
