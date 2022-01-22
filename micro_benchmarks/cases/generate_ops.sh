#!/usr/bin/bash 
# for i in $(seq 2 6)  
# do
# max_num=$[2**$(($i))]
# echo "" > op_$[2**$i]_flush.h
# for((j=0;j<$(($max_num));j++));  
# do

# echo "DOIT("  $((16*$j))  ") " >> op_$[2**$i]_flush.h

# done
# # echo "p[" $(($max_num-1)) "];" >> op_$[2**$i].h
# done 


# op_flush
# for i in $(seq 6 6)  
# do
# max_num=$[2**$(($i))]
# echo "" > op_flush.h
# for((j=0;j<$(($max_num));j++));  
# do

# echo "DOIT("  $((16*$j))  ") " >> op_flush.h

# if [ $((($j + 1) % 4)) == 0 ];then
# echo "COMP_GRANU($((($j + 1)/4 )))" >> op_flush.h
# fi

# done
# # echo "p[" $(($max_num-1)) "];" >> op_$[2**$i].h
# done 

# op 
for i in $(seq 6 6)  
do
max_num=$[2**$(($i))]
echo "" > ops_1_64.h
for((j=0;j<$(($max_num));j++));  
do

echo "DOIT("  $(($j))  ") " >> ops_1_64.h

# if [ $((($j + 1) % 1)) == 0 ];then
# # echo "COMP_GRANU($((($j + 1)/64 )))" >> ops_1_64.h
# fi

done
# echo "p[" $(($max_num-1)) "];" >> op_$[2**$i].h
done 