#!/usr/bin/python3

###############################################################################
#  Copyright 2016-2017 Cisco Systems, Inc.                                    #
#                                                                             #
#  Licensed under the Apache License, Version 2.0 (the "License");            #
#  you may not use this file except in compliance with the License.           #
#                                                                             #
#  You may obtain a copy of the License at                                    #
#                                                                             #
#      http://www.apache.org/licenses/LICENSE-2.0                             #
#                                                                             #
#  Unless required by applicable law or agreed to in writing, software        #
#  distributed under the License is distributed on an "AS IS" BASIS,          #
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
#  See the License for the specific language governing permissions and        #
#  limitations under the License.                                             #
###############################################################################

import os
import sys
import re
import json

SEP = '\t'

def process_row(row, width):
        if row is not None:
            vals = [str(v) for v in row]
            assert len(vals) == width
            line = SEP.join(vals)
        else:
            line = (SEP + 'NaN') * width
        return line

def process_controller_log(line, test_logs):
    'place-holder, parsing debug stats'
    match = re.search(r'controller_log: DEBUG:', line)
    if match:
        #Controller's debug log, ignore
        return

    'parsing nada-specific stats'
    # ts: 158114 loglen: 60 qdel: 286 rtt: 386 ploss: 0 plr: 0.00 xcurr: 4.72 rrate: 863655.56 srate: 916165.81 avgint: 437.10 curint: 997 delta: 100
    match = re.search(r'algo:nada (\S+) ts: (\d+) loglen: (\d+)', line)
    match_d = re.search(r'qdel: (\d+(?:\.\d*)?|\.\d+) rtt: (\d+(?:\.\d*)?|\.\d+)', line)
    match_p = re.search(r'ploss: (\d+) plr: (\d+(?:\.\d*)?|\.\d+)', line)
    match_x = re.search(r'xcurr: (\d+(?:\.\d*)?|\.\d+)', line)
    match_r = re.search(r'rrate: (\d+(?:\.\d*)?|\.\d+) srate: (\d+(?:\.\d*)?|\.\d+)', line)
    match_l = re.search(r'avgint: (\d+(?:\.\d*)?|\.\d+) curint: (\d+(?:\.\d*)?|\.\d+)', line)
    match_D = re.search(r'delta: (\d+(?:\.\d*)?|\.\d+)', line)

    if match:
        assert (match_d and match_p and match_x and match_r and match_l)
        obj = match.group(1);
        ts = int(match.group(2)) / 1000. # to seconds
        loglen = int(match.group(3));

        qdel = float(match_d.group(1))
        rtt = float(match_d.group(2))
        ploss = int(match_p.group(1))
        plr = float(match_p.group(2))
        x_curr = float(match_x.group(1))
        rrate = float(match_r.group(1))
        srate = float(match_r.group(2))
        avgint = float(match_l.group(1))
        curint = int(match_l.group(2))
        delta = float(match_D.group(1))

        if obj not in test_logs['nada']:
            test_logs['nada'][obj] = []
        test_logs['nada'][obj].append([ts, qdel, rtt, ploss, plr, x_curr,
                                       rrate, srate, loglen, avgint, curint, delta])
        return


def process_tcp_log(line, test_logs):
    #tcp_0 ts: 165000 recv: 16143000 rrate: 1160000.0000
    match = re.search(r'(\S+) ts: (\d+) recv: (\d+) rrate: (\d+(?:\.\d*)?|\.\d+)', line)
    if match:
        obj = match.group(1)
        ts = int(match.group(2)) / 1000. # to seconds
        bytes_recvd = int(match.group(3))
        rrate = float(match.group(4))
        if obj not in test_logs['tcp']:
            test_logs['tcp'][obj] = []
        test_logs['tcp'][obj].append([ts, bytes_recvd, rrate])
        return
    assert False, "Error: Unrecognized tcp log line: <{}>".format(line)

def process_log(dirname, filename, all_logs):
    abs_fn = os.path.join(dirname, filename)
    if not os.path.isfile(abs_fn):
        print("Skipping file {} (not a regular file)".format(filename))
        return
    match = re.match(r'([a-zA-Z0-9_\.-]+).log', filename)
    if match is None:
        print("Skipping file {} (not a log file)".format(filename))
        return

    print("Processing file {}...".format(filename))
    test_name = match.group(1).replace(".", "_").replace("-", "_")

    test_logs = {'nada': {}, 'tcp': {} }
    all_logs[test_name] = test_logs

    with open(abs_fn) as f_log:
        for line in f_log:
            match = re.search(r'controller_log:', line)
            if match:
                process_controller_log(line, test_logs)
                continue
            match = re.search(r'tcp_log:', line)
            if match:
                process_tcp_log(line, test_logs)
                continue
            #Unrecognized ns3 log line , ignore

    saveto_matfile(dirname, filename, test_logs)

def saveto_matfile(dirname, test_name, test_logs):
    'save to *.mat file'
    f_out_name = os.path.join(dirname, '{}.mat'.format(test_name))
    print('Creating matlab file: {}'.format(f_out_name))
    f_out = open(f_out_name, 'w')
    f_out.write('%  id | ts | qdel | rtt | ploss | plr | xcurr ')
    f_out.write('| rrate | srate | loglen | avgint | curint\n')
    for (i, obj) in enumerate(test_logs['nada'].keys()):
        nrec = len(test_logs['nada'][obj])
        print('parsing flow ', obj, ' number of records: ', nrec)
        for j in range(nrec):
            row = process_row(test_logs['nada'][obj][j], width=12)
            f_out.write(SEP.join([str(i), row]))
            f_out.write('\n')

# -------------------- #
if len(sys.argv) != 2:
    sys.stderr.write('Usage: python {} <log_directory>\n'.format(sys.argv[0]))
    sys.exit(1)

dirname = sys.argv[1]
assert os.path.isdir(dirname)
all_logs = {}
for filename in os.listdir(dirname):
    process_log(dirname, filename, all_logs)

print("Creating json file with all data: all_tests.json")
f_json_name = os.path.join(dirname, 'all_tests.json')
f_json = open(f_json_name, 'w')
json.dump(all_logs, f_json, indent=4, sort_keys=True)
