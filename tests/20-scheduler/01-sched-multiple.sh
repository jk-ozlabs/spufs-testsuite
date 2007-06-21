#!/bin/bash

set -e

n_spes=$(ls -1 /sys/devices/system/spu/ | wc -l)

tests="1 2 $(($n_spes - 1)) $n_spes $(($n_spes + 1)) $((n_spes * 2))"


for i in $tests
do
	gen_datafile.py run.data $i 1024
	sched_multiple run.data
done
