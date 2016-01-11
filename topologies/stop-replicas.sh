#!/bin/bash

while read line           
do           
  kill $line
done < .PIDS

