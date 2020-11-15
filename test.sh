#!/bin/bash  
  
for((i=1;i<=$1;i++));  
do
    ./broccoli-client --address 127.0.0.1:5050 --key MDYwEAYHKoZIzj0CAQYFK4EEABwDIgAEeDMpwTfhO5QrOxbrLYHo1CZOZjSnkUEwtJCaBE5zAce= --id $i &
    sleep 0.01
done 

