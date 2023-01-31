#!/bin/bash

echo "start to run small...";
echo "small: " >> res_log;
./small_range_rand_allocs >> res_log;
echo "small finished"

echo "start to run large...";
echo "large: " >> res_log;
./large_range_rand_allocs >> res_log;
echo "large finished"

echo "start to run equal...";
echo "equal: " >> res_log;
./equal_size_allocs >> res_log;
echo "equal finished"

echo >> res_log;
