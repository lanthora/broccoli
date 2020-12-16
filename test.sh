#!/bin/bash  

if (($#<1));
then
    n=1
else
    n=$1
fi

echo "Close"
pkill -f broccoli-

echo "Start Server"
./broccoli-server &

echo "Start Client"

for((i=1;i<=$n;i++));
do
    ./broccoli-client &
    sleep 0.01
done 

