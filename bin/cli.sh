#!/bin/bash
cat $1 |while read line;do
	./cli $2 $3 <<DONE
	$line
DONE
done
