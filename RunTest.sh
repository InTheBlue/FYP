#!/bin/bash
ver=`git log --format="%h" -n 1`
timestamp=`date +%Y%m%d-%H%M%S`
logfile="./Logs/Runlog-$ver-$timestamp.txt"
exec 6>&1
exec > $logfile
printf "Git Hash: %s\n" $ver
count=5
total=0
for ((i=1;i<=count;i++)); do
	output=$(./TestProgram.exe)
	printf "Run %d: %s\n" $i $output
	total=$(echo $total+$output | bc)
done
avg=$(echo "scale=3; $total / $count" | bc)
echo "Total: $total"
echo "Avg: $avg"
exec 1>&6 6>&-
cat $logfile
