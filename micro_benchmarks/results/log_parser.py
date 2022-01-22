import enum
import re
import copy
import csv
import os
import json
import pandas as pd

this_file_dir = os.path.abspath(os.path.dirname(__file__))

class LogParser:
    def __init__(self):
        self.data_list_ = []
        self.attributes_ = []
        # 0: not in the middle of a transaction, 1 processing a transaction
        self.stage_flag_ = 0
        self.seperator_start_ = re.compile(r"-------result--------")
        self.seperator_end_ = re.compile(r"---------------------")
        self.pair_match_expression_ = re.compile(r"\[(.*?)\]\ *:\ *\[(.*?)\]")


    def scanFile(self, file_name):
        f = open(file_name)
        new_item = 0
        kv_dict = {}
        while(1):                
            line = f.readline()
            if(not line):
                break
            seperate_start_status = re.search(self.seperator_start_, line)
            seperate_end_status = re.search(self.seperator_end_, line)
            # if we have detected a new item
            if(seperate_start_status):
                self.stage_flag_ = 1
            # if we come to the end of this item, dump this item
            if(seperate_end_status):
                self.stage_flag_ = 0
                if(kv_dict):
                    self.data_list_.append(copy.deepcopy(kv_dict))
                kv_dict.clear()
            if(self.stage_flag_ == 0):
                continue
            
            
            obj = re.findall(self.pair_match_expression_, line)
            if(obj):
                for kv_pair in obj:
                    kv_dict[kv_pair[0]] = kv_pair[1]
                
        f.close()
        with open(this_file_dir+'/dumped_data.csv', 'w') as f:
            headers = dict.keys(self.data_list_[0])
            f_csv = csv.DictWriter(f, headers)
            f_csv.writeheader()
            f_csv.writerows(self.data_list_)



def dropAttributeColumn(df,attributes):
    # the column is the attibute of the dataframe, it must be all the same in thw whole column
    # df.reset_index(drop=True)
    attribute_name = ''
    for i in attributes:
        attribute_name = attribute_name+" " +str(df[i][0])
        df.drop(columns= i, inplace=True)
    names = df.columns.tolist()
    for idx, item in enumerate(names):
        names[idx] = attribute_name + " " + item
    df.columns = names
    return df

class dataFrameMerger:
    def __init__(self) -> None:
        pass
    def mergeDf(self, df, attributes, independent_var, output_file_name):
        # attributes is a string list
        tmplist = []
        if attributes:
            grouped = df.groupby(attributes)
            for i in grouped:
                tmplist.append(i) 
            col_name=independent_var
            sub_df_len = len(tmplist)
            for i in range(sub_df_len):
                first_col = tmplist[i][1].pop(col_name)
                tmplist[i] = dropAttributeColumn(tmplist[i][1].reset_index(drop=True) , attributes)
        else:
            tmplist.append(df.reset_index(drop=True))
            col_name=independent_var
            first_col = tmplist[0].pop(col_name)

        c = pd.concat(tmplist, axis=1)
        c.insert(0, col_name, first_col.reset_index(drop=True))
        c.to_csv(output_file_name)

class tabGenerator:
    def __init__(self, config_file):
        with open(config_file,) as f:
            data = json.load(f)
            self.out_file_name_ = data[0]
            self.independent_var_ = data[1]
            self.attributes_ = data[2]
    def parser(self, df):
        tmp = dataFrameMerger()
        tmp.mergeDf(df, self.attributes_, self.independent_var_, self.out_file_name_)

# parse file and get all the data
def parse_log(log_path, config_path):
    a = LogParser()
    a.scanFile(log_path)
    df = pd.read_csv(this_file_dir+'/dumped_data.csv')
    worker = tabGenerator(config_path)
    worker.parser(df)

parse_log(os.path.join(this_file_dir, "../task0.log"),os.path.join(this_file_dir, "task0.json"))
parse_log(os.path.join(this_file_dir, "../task1.log"),os.path.join(this_file_dir, "task1.json"))



