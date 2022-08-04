#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import json

ver_file = open('version.mk', 'r')
ver_dict = dict()

# parse version.mk, put result to ver_dict
for line in ver_file:
    # remove last \n char, then split by '='
    pair = line[0:-1].split("=")
    ver_dict[pair[0]]=pair[1]

# sync version.mk to rfid_engine.json
with open('rfid_engine.json') as f:
    cfg = json.load(f)

cfg['version_major'] = int(ver_dict['MAJOR'])
cfg['version_minor'] = int(ver_dict['MINOR'])
cfg['version_subminor'] = int(ver_dict['SUB_MINOR'])

print(cfg)

with open('rfid_engine.json', 'w') as outfile:
    json.dump(cfg, outfile, indent = 4)
