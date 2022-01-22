#!/usr/bin/sh


build_dir=build/bin
${build_dir}/microbench -test 0 > task0.log
${build_dir}/microbench -test 1 > task1.log
${build_dir}/microbench -test 2 > task2.log
${build_dir}/microbench -test 3 > task3.log
${build_dir}/microbench -test 4 > task4.log
${build_dir}/microbench -test 5 > task5.log
${build_dir}/microbench -test 6 > task6.log