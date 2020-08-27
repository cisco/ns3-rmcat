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
import json
import matplotlib.pyplot as plt

colorlist = ['blue',
             'orange',
             'green',
             'pink',
             'red',
             'purple',
             'gray',
             'black',
             'brown',
             'aqua',
             'navy',
             'teal',
             'olive',
             'coral',
             'lime',
             'royalblue',
             'maroon',
             'yellowgreen',
             'tan',
             'khaki',
             'darkslategrey',
             'darkgreen',
             'sienna',
             'peachpuff',
             'sandybrown',
             'steelblue']

tableau20 = [(31, 119, 180), (174, 199, 232), (255, 127, 14), (255, 187, 120),
             (44, 160, 44), (152, 223, 138), (214, 39, 40), (255, 152, 150),
             (148, 103, 189), (197, 176, 213), (140, 86, 75), (196, 156, 148),
             (227, 119, 194), (247, 182, 210), (127, 127, 127), (199, 199, 199),
             (188, 189, 34), (219, 219, 141), (23, 190, 207), (158, 218, 229)]

# Scale the RGB values to the [0, 1] range, which is the format matplotlib accepts.
for i in range(len(tableau20)):
    r, g, b = tableau20[i]
    colorlist.append((r / 255., g / 255., b / 255.))


def adjust_tmax(tmax, new_max_ts):
    tmax_tmp = int(new_max_ts + 2.5) / 5 * 5
    return tmax_tmp if tmax_tmp > tmax else tmax

def plot_test_case(tc_name, contents, dirname):
    rmcat_log = contents['nada']
    tcp_log = contents['tcp']
    rmcat_keys = sorted(rmcat_log.keys())
    tcp_keys = sorted(tcp_log.keys())

    print('plotting data for tc {}...'.format(tc_name))
    nflow = len(rmcat_keys) + len(tcp_keys)

    l = len(colorlist)
    pngfile = '{}.png'.format(tc_name);
    tmax = 100
    fig = plt.figure()
    plt.subplot(311)
    for (i, obj) in enumerate(rmcat_keys):
        rcolor = colorlist[i % l]
        rcolor2 = colorlist[i+1 % l]
        ts = [x[0] for x in rmcat_log[obj]]
        tmax = adjust_tmax(tmax, max(ts))
        rrate = [x[6]/1.e+6 for x in rmcat_log[obj]]
        srate = [x[7]/1.e+6 for x in rmcat_log[obj]]
        if nflow == 1:
            plt.plot(ts, rrate, 'd', linewidth=1.0, color=rcolor2, mfc=rcolor2, mec='none',
                                     ms=2, label=obj+'(recv)')
            plt.plot(ts, srate, 'o', linewidth=1.0, color=rcolor, mfc=rcolor, mec='none',
                                 ms=2, label=obj+'(sent)')
        else:
            plt.plot(ts, srate, 'o', linewidth=1.0, color=rcolor, mfc=rcolor, mec='none',
                                 ms=2, label=obj)

    for (i, obj) in enumerate(tcp_keys):
        rcolor = colorlist[(i + 5) % l]
        ts = [x[0] for x in tcp_log[obj]]
        tmax = adjust_tmax(tmax, max(ts))
        rrate = [x[2]/1.e+6 for x in tcp_log[obj]]
        plt.plot(ts, rrate, '-o', linewidth=.7, color=rcolor, mfc=rcolor, mec='none',
                                  ms=2, label=obj)

    plt.xlim(0, tmax)
    plt.ylim(0, 2.5)
    plt.ylabel('Rate (Mbps)')
    # plt.legend(loc='upper left', prop={'size':6}, bbox_to_anchor=(1,1), ncol=1)
    all_curves = len(rmcat_keys) + len(tcp_keys)
    if all_curves < 12:
        plt.legend(ncol = (all_curves // 4) + 1, loc='upper right', fontsize = 'small')

    plt.subplot(312)
    for (i, obj) in enumerate(rmcat_keys):
        rcolor = colorlist[i % l]
        ts = [x[0] for x in rmcat_log[obj]]
        qdelay = [x[1] for x in rmcat_log[obj]]
        rtt = [x[2] for x in rmcat_log[obj]]
        xcurr = [x[5] for x in rmcat_log[obj]]
        delta = [x[11] for x in rmcat_log[obj]]
        plt.plot(ts, qdelay, 'o', color=rcolor, mfc=rcolor, mec='none', ms=2, label=obj)
        # plt.plot(ts, xcurr, 'o', color='purple', mfc='purple', mec='none', ms=2, label=obj)
        # plt.plot(ts, delta, 'o', color=rcolor, mfc=rcolor, mec='none', ms=1, label=obj)
    plt.xlim(0, tmax)
    plt.ylim(0, 400)
    # plt.ylim(0, plt.gca().get_ylim()[1] * 1.5) #Margin for legend
    plt.ylabel('QDelay (ms)')
    # plt.legend(loc='upper left', prop={'size':6}, bbox_to_anchor=(1,1), ncol=1)
    all_curves = len(rmcat_keys)
    # plt.legend(ncol = (all_curves / 4) + 1, loc='upper right', fontsize = 'small')

    plt.subplot(313)
    for (i, obj) in enumerate(rmcat_keys):
        rcolor = colorlist[i % l]
        ts = [x[0] for x in rmcat_log[obj]]
        ploss = [x[3] for x in rmcat_log[obj]]
        plr = [x[4]*100. for x in rmcat_log[obj]]  # ratio => %
        loglen = [x[8] for x in rmcat_log[obj]]
        plt.plot(ts, plr, 'o', color=rcolor, mfc=rcolor, mec='none', ms=2, label=obj)
        # plt.plot(ts, ploss, 'd',  color=rcolor, mfc=rcolor, mec='none', ms=4, label=obj)

    plt.xlim(0, tmax)
    # plt.ylim(0, 5)
    plt.ylabel('PLR (%)')
    plt.xlabel('Time (s)')
    # plt.legend(loc='upper left', prop={'size':6}, bbox_to_anchor=(1,1), ncol=1)
    # plt.legend(ncol = (all_curves / 4) + 1, loc='upper right', fontsize = 'small')
    fig.savefig(os.path.join(dirname, pngfile))
    plt.close(fig)


# ---------  #
if len(sys.argv) != 2:
    sys.stderr.write('Usage: python {} <log_directory>\n'.format(sys.argv[0]))
    sys.exit(1)

dirname = sys.argv[1]
assert os.path.isdir(dirname)

f_json_name = os.path.join(dirname, 'all_tests.json')
assert os.path.isfile(f_json_name)
f_json = open(f_json_name, 'r')
all_logs = json.load(f_json)
for test_case in all_logs.keys():
    plot_test_case(test_case, all_logs[test_case], dirname)
