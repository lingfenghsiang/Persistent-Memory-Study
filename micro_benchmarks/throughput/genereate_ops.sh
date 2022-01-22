#!/usr/bin/bash 
# echo $1
echo "" > op_$1_$2.h
for ((i=0;i < $2;i++));
do
echo "DOIT("  $(($i*$1))  ") " >> op_$1_$2.h

done

# for i in $(seq 6 12)  
# do
# # max_num=$[2**$(($i-6))]
# # echo "" > op_$[2**$i]B.h
# # for((j=0;j<$(($max_num));j++));  
# do   
# # echo "DOIT("  $(($j*16))  ") " >> op_$[2**$i]B.h

# done
# # echo "p[" $(($max_num-1)) "];" >> op_$[2**$i].h
# done 