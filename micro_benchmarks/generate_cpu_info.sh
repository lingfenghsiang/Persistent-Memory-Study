#!/usr/bin/sh
FILE=cpu_info.h
if [ -f "$FILE" ]; then
    echo "$FILE exists."
else 
    (lscpu --all --extended | awk 'NR>1 {print $1, $3, $4}') > tmp0.txt
    echo "#pragma once" > cpu_info.h
    echo "#include <iostream>" >> cpu_info.h
    echo -n "#define SOCKET_NUM " >> cpu_info.h
    (awk '{k=0;if($2>=k)k=$2}END{print k+1}' tmp0.txt) >> cpu_info.h
    echo -n "#define PHY_CORE_NUM " >> cpu_info.h
    (awk '{k=0;if($3>=k)k=$3}END{print k+1}' tmp0.txt) >> cpu_info.h
    echo -n "#define HYPER_THERAD_CORE_NUM " >> cpu_info.h

    (wc -l < tmp0.txt) >> cpu_info.h
    echo  "" >> cpu_info.h
    cat cpu_data_structure.txt >> cpu_info.h
    echo  "" >> cpu_info.h
    echo  "" >> cpu_info.h
    echo static const uint16_t cpus_on_board[] = >> cpu_info.h
    echo "{">> cpu_info.h
    (sort -k2n -k3n tmp0.txt | awk '{print $1}') | awk '{printf "%s%s",sep,$1; sep=","} END{print ""}'  >> cpu_info.h
    echo "};">> cpu_info.h

    rm tmp*
fi


