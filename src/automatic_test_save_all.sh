#!/bin/bash

SNAPTESTDIR=extras/snap_tests/

if [ ! -d $SNAPTESTDIR ]; then
	echo "$SNAPTESTDIR does not exist"
	exit 1
fi



for i in $SNAPTESTDIR/*.zx $SNAPTESTDIR/*.z80 $SNAPTESTDIR/*.sna $SNAPTESTDIR/*.o $SNAPTESTDIR/*.p $SNAPTESTDIR/*.sp $SNAPTESTDIR/*.zsf; do
	echo $i
	NOMBRE=`basename $i`
	./zesarux --noconfigfile --enable-remoteprotocol --verbose 0 $i &
	PIDNUM=$!

	( sleep 2 ; echo "snapshot-save /tmp/$NOMBRE" ; sleep 1 ; echo "exit-emulator" ) | telnet localhost 10000
	sleep 1
	kill $PIDNUM

done
