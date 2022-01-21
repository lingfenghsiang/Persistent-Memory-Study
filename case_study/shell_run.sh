#!/usr/bin/sh
root_dir=/home/xlf/Documents/kv_test

# rm /mnt/pmem/*.pm

# numactl -N 0 $root_dir/build/kv_test -thread=20 -preread -prereadnum=80

# numactl -N 0 $root_dir/build/kv_test -thread=20 -nopreread
echo > volatile_sfence_result.log
for i in $(seq 1 10)
do
# echo ------------start test------------ >>result.log
# echo command: >>result.log
echo numactl -N 0 $root_dir/build/kv_test -thread=$i -preread >>volatile_sfence_result.log
numactl -N 0 $root_dir/build/kv_test -thread=$i -preread -pool_dir=/mnt/volatile_pm/xlf 1>>volatile_sfence_result.log
rm /mnt/volatile_pm/xlf
# echo ------------end test------------ >>result.log
echo
# rm /mnt/pmem/*.pm
done


for i in $(seq 1 10)
do
# echo ------------start test------------ >>result.log
# echo command: >>result.log
echo numactl -N 0 $root_dir/build/kv_test -thread=$i -nopreread >>volatile_sfence_result.log
numactl -N 0 $root_dir/build/kv_test -thread=$i -nopreread -pool_dir=/mnt/volatile_pm/xlf 1>>volatile_sfence_result.log
rm /mnt/volatile_pm/xlf
# echo ------------end test------------ >>result.log
echo
# rm /mnt/pmem/*.pm
done




# for i in $(seq 1 10)
# do
# echo ------------start test------------
# echo command:
# echo numactl -N 0 $root_dir/build/kv_test -thread=$i -nopreread
# numactl -N 0 $root_dir/build/kv_test -thread=$i -nopreread
# echo ------------end test------------
# echo
# rm /mnt/pmem/*.pm
# done