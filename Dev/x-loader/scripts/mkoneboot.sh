#!/bin/sh

IMAGE1=x-load.bin.ift
IMAGE2=x-load-signed.ift
TMP=oneloader
PAGESIZE=2048

size=`ls -la $IMAGE1 | awk -F' ' '{ printf $5}'`
let remain=$PAGESIZE-$size
dd if=/dev/zero of=$IMAGE2 bs=1 count=512
#dd if=/dev/zero of=zerofile bs=$remain count=1 > /dev/null 2> /dev/null
cat $IMAGE1>>$IMAGE2

rm -f $IMAGE1

echo "Create $IMAGE2 completed..."
