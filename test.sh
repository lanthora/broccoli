#!/bin/bash  
  
for((i=1;i<=$1;i++));  
do
    ./broccoli-client --address 10.20.30.1:5050 --key MDYwEAYHKoZIzj0CAQYFK4EEABwDIgAEeDMpwTfhO5QrOxbrLYHo1CZOZjSnkUEwtJCaBE5zAce= --id $i &
    sleep 0.01
done 

