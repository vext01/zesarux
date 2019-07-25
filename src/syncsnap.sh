#!/bin/bash
#simple test to show snapshot copy from one instance of ZEsarUX to another
#the second instance has ZRCP port 10010

TEMPFILE=`mktemp`
( sleep 1 ; echo "get-snapshot " ; sleep 1 ) | telnet localhost 10000 > $TEMPFILE

PARSEDFILE=`mktemp`

cat $TEMPFILE|tail -2|head -1|sed 's/command> //' > $PARSEDFILE

( sleep 1 ; echo -n "put-snapshot " ; cat $PARSEDFILE ; echo ; sleep 1 ) | telnet localhost 10010

rm -f $TEMPFILE
rm -f $PARSEDFILE
