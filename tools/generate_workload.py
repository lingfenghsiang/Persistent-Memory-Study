import re
import os
import argparse

this_file_dir = os.path.abspath(os.path.dirname(__file__))
tmp_dir = os.path.join(this_file_dir, "../tmp")

def floattostr(x,xsnum):
    if x < 1000:
        ret = x
        ret = str(round(ret*1.0,xsnum))
    elif x < 1000000:
        ret = x/1000
        ret = str(round(ret*1.0,xsnum)) + "K"
    elif x < 1000000000:
        ret = x/1000000
        ret = str(round(ret*1.0,xsnum)) + "M"
    return ret

def generateRawLog(ycsb_bin_path, config_path, op_num):
    
    # os.system(load_command)
    out_config = open("ycsb_config", 'w')
    with open(config_path, 'r') as f:
        while 1:
            line = f.readline()
            if not line:
                break
            op = str(op_num)
            if re.search(r'recordcount=[0-9]*', line):
                replaced_line = re.sub(r'recordcount=[0-9]*', 'recordcount=' + op, line)
            elif re.search(r'operationcount=[0-9]*', line):
                replaced_line = re.sub(r'operationcount=[0-9]*', 'operationcount=' + op, line)
            else:
                replaced_line =  line
            out_config.write(replaced_line)
    out_config.close()

    load_command = ycsb_bin_path + "/ycsb.sh load basic -P " + "ycsb_config > " + os.path.join(tmp_dir , "load_" + floattostr(op_num, 1) + ".log") 
    run_command = ycsb_bin_path + "/ycsb.sh run basic -P " + "ycsb_config > " + os.path.join(tmp_dir, "run_" + floattostr(op_num, 1) + ".log") 
    os.system(load_command)
    os.system(run_command)

def prepare(raw_file, new_file_name):
    raw_log = open(raw_file, 'r')
    out_file = open(new_file_name, 'w')
    while True:
        line = raw_log.readline()
        if not line:
            break
        search_result = re.search(r'([A-Z]*) usertable user([0-9]*) ', line)
        if search_result:
            out_file.write(search_result.groups(1)[0] + " " + search_result.groups(1)[1] + "\n")
    raw_log.close()
    out_file.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Prepare ycsb running data.')
    parser.add_argument('-op_num', type=int, 
                        help='how many operations to do in the workload')
    args = parser.parse_args()
    print("this workload has ", (args.op_num), " operations")
    
    generateRawLog(os.path.join(this_file_dir, "../tmp/ycsb-0.17.0/bin"), os.path.join(this_file_dir, "ycsb_workload"), args.op_num)

    prepare(os.path.join(tmp_dir,"run_"+floattostr(args.op_num, 1)+".log"), os.path.join(tmp_dir,"run.log" ))
    prepare(os.path.join(tmp_dir,"load_"+floattostr(args.op_num, 1)+".log"), os.path.join(tmp_dir,"load.log" ))