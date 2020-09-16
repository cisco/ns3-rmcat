#!/bin/csh

###############################################################################
#  Copyright 2016-2020 Cisco Systems, Inc.                                    #
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

# run from ns3 root directory: ns-3.xx/
#
# Example:
# ./src/ns3-rmcat/tools/test_v2.csh wired 2020-08-25-rmcat-wired
# ./src/ns3-rmcat/tools/test_v2.csh vparam 2020-08-25-rmcat-wired-vparam
# ./src/ns3-rmcat/tools/test_v2.csh wifi 2020-08-25-rmcat-wifi
#
# The second parameter, output directory, is optional. If not specified,
# the script will use a folder with a name based on current GMT time

set scen = $1
set odir = $2
if ( "$odir" == "" ) then
    set odir = `date -u +"%Y-%m-%d-%H-%M-%S-CUT"`
    set odir = "testpy-output/$odir"
endif

set patched_script = test_patched.py

# uncomment the following for building from scratch
#
# clean up and rebuild
# echo "cleaning up and rebuiling ..."
# ./waf clean
# ./waf

# patch test.py
if ( ! -f  "$patched_script") then
    rm -f "$patched_script"
endif
patch -p1 -i src/ns3-rmcat/tools/test.py.diff -o "$patched_script" test.py

# run tests
echo "running tests ..."
python2 "$patched_script" -o $odir -s rmcat-$scen -w rmcat-$scen.html -r

# process and plot
echo "processing and plotting ..."
python3 src/ns3-rmcat/tools/process_test_logs.py  $odir
python3 src/ns3-rmcat/tools/plot_tests.py $odir

# for reviewing results
#echo "reviewing results ..."
#open $odir/*.png
