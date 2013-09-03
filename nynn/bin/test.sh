#!/bin/bash
for i in {8..22};do
	time cat $1 |./nynn_cli_write -s $((2**$i)) -n 1 192.168.1.4
	if [ $i -lt 10 ];then
		echo $((2**$i))B:
	elif [ $i -lt 20 ];then
		echo $((2**($i-10)))KB:
	else
		echo $((2**($i-20)))MB:
	fi
	sleep 10s
done
	
