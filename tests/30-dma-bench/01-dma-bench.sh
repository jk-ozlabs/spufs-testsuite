#!/bin/bash

VERSION=$(uname -r)

function run_test
{
	typeset -i i=0

	while [[ $i -lt 10 ]];
	do
		taskset -c 0 $2
		i=$i+1
	done
}
set -e
run_test "put" "test_dma_put"
run_test "get" "test_dma_get"
