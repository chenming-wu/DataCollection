# -*- coding: utf-8 -*-
import glob
import random
import os.path

names = []

def generate_data(data_dir):
    params = {}
    for root, dirs, files in os.walk(data_dir):
        for file in files:
            if file.endswith('.txt'):
                name = file[:-4]
                names.append(name)

def save_pairwise(file_name):
    pairwise_list = []
    for i in range(0,len(names)):
        for j in range(i + 1, len(names)):
            pairwise_list.append(names[i] + ' ' + names[j])

    random.shuffle(pairwise_list)
    random.shuffle(pairwise_list)
    random.shuffle(pairwise_list)
    file = open(file_name,'w+')
    for i in pairwise_list:
        file.write("%s\n" % i)

generate_data('data')
save_pairwise('data.txt')
