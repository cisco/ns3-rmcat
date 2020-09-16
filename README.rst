.. contents::

ns3-rmcat Documentation
----------------------------

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

ns3-rmcat is an `ns3 <https://www.nsnam.org/release/ns-allinone-3.26.tar.bz2>`_ module (Currently ns-3.26 is supported) used for `IETF RMCAT <https://datatracker.ietf.org/wg/rmcat/charter/>`_ candidate algorithm testing.

It can be used in one of these ways:

1. As complementary tests for the unit test and integration test in emulator environment.

2. Algorithm tuning. It's easier to support different topologies using ns3, as ns3 provides sophisticated LTE and WiFi models, which make it easier to peek the internals than a real world system and emulator testbed.

3. Algorithm comparison. Simulation testbed is more reproducible than real world test and emulator testbed. The different candidate algorithms can be easily switched, thus making side-by-side comparison of different algorithms an easy and fair task.


Model
*****************

The model for real-time media congestion is documented at `"Framework for Real-time Media Congestion Avoidance Techniques" <https://tools.ietf.org/html/draft-zhu-rmcat-framework-00>`_, and ns3-rmcat is a simplified implementation focussing on evaluation of real-time media congestion control algorithms in a simulation environment.

ns3-rmcat defines ns3 applications (see `model/apps <model/apps>`_) running on different network topologies. These ns3 applications send fake video codec data (`model/syncodecs <model/syncodecs>`_, more on `syncodecs <https://github.com/cisco/syncodecs>`_), according to the congestion control algorithm under test.

The sender application, ``RmcatSender``, sends fake video codec data in media packets to the receiver application, ``RmcatReceiver``. ``RmcatReceiver`` gets the sequence of packets and takes reception timestamp information, and sends it back to ``RmcatSender`` in feedback packets. The (sender-based) congestion control algorithm running on ``RmcatSender`` processes the feedback information (see `model/congestion-control <model/congestion-control>`_), to get bandwidth estimation. The sender application then uses this bandwidth estimation to control the fake video encoder by adjusting its target video bitrate.

Different topologies (see `model/topo <model/topo>`_) are currently supported, currently only point-to-point wired topology and WIFI topologies are used. We will add LTE support later.

Testcases
*****************

The test cases are in `test/rmcat-wired-test-suite <test/rmcat-wired-test-suite.cc>`_ and `test/rmcat-wifi-test-suite <test/rmcat-wifi-test-suite.cc>`_; and currently organized in three test suites:

  - `rmcat-wifi <https://datatracker.ietf.org/doc/draft-ietf-rmcat-eval-test/?include_text=1>`_

  - `rmcat-wired <https://datatracker.ietf.org/doc/draft-fu-rmcat-wifi-test-case/?include_text=1>`_

  - rmcat-wired-vparam, which is based on some of the wired test cases, but varying other parameters such as bottleneck bandwidth, propagation delay, etc.

`LTE <https://datatracker.ietf.org/doc/draft-ietf-rmcat-wireless-tests/?include_text=1>`_ test case are not implemented yet.

Examples
*****************

`examples <examples>`_ is provided as an application template for experimenting new test cases and algorithm changes.

Write your own congestion control algorithm
***************************************************

You can create your own congestion control algorithm by inheriting from  `SenderBasedController <model/congestion-control/sender-based-controller.h#L85>`_, `DummyController <model/congestion-control/dummy-controller.h#L39>`_ is an example which just prints the packet loss, queuing delay and receive rate without doing any congestion control: the bandwidth estimation is hard-coded.

To reuse the plotting tool, the following logs are expected to be written (see `NadaController <model/congestion-control/nada-controller.cc>`_, `process_test_logs.py <tools/process_test_logs.py>`_):

::

    # rmcat flow 0, this is the flow id, SenderBasedController::m_id
    # ts, current timestamp when receving the rmcat feedback in millionseconds
    # loglen, packet history size, SenderBasedController::m_packetHistory.size()
    # qdel, queuing delay, SenderBasedController::getCurrentQdelay()
    # rtt, round trip time, SenderBasedController::getCurrentRTT()
    # ploss, packet loss count in last 500 ms, SenderBasedController::getPktLossInfo()
    # plr, packet loss ratio, SenderBasedController::getPktLossInfo()
    # xcurr, aggregated congestion signal that accounts for queuing delay, ECN
    # rrate, current receive rate in bps, SenderBasedController::getCurrentRecvRate()
    # srate, current estimated available bandwidth in bps
    # avgint, average inter-loss interval in packets, SenderBasedController::getLossIntervalInfo()
    # curint, most recent (currently growing) inter-loss interval in packets, SenderBasedController::getLossIntervalInfo()

    rmcat_0 ts: 158114 loglen: 60 qdel: 286 rtt: 386 ploss: 0 plr: 0.00 xcurr: 4.72 rrate: 863655.56 srate: 916165.81 avgint: 437.10 curint: 997


Usage
*****************

1. Download ns3 (ns-3.26 is currently supported, other version may also work but are untested).

2. Git clone ns3-rmcat into ``ns-3.xx/src``. Initialize syncodecs submodule (``git submodule update --init --recursive``)

3. configure the workspace, ``CXXFLAGS="-std=c++11 -Wall -Werror -Wno-potentially-evaluated-expression -Wno-unused-local-typedefs" ./waf configure --enable-examples --enable-tests``.

4. build, ``./waf build``

5. run tests, ``./test.py -s rmcat-wired -w rmcat.html -r``, where ``rmcat.html`` is the test report.

7. [optional] run examples, ``./waf --run "rmcat-example --log"``, ``--log`` will turn on RmcatSender/RmcatReceiver logs for debugging.

8. draw the plots (need to install the python module `matplotlib <https://matplotlib.org/>`_), ``python src/ns3-rmcat/tools/process_test_logs.py testpy-output/2017-08-11-18-52-15-CUT; python src/ns3-rmcat/tools/plot_tests.py testpy-output/2017-08-11-18-52-15-CUT``

You can also use `test.csh <tools/test.csh>`_ to run the testcases and the plot scripts in one shot. If you do so, logs with testcase names will be located in the "testpy-output/[CURRENT UTC TIME]" directory, if none specified.

::

    # run from ns3 root directory: ns-3.xx/
    #
    # Example:
    # ./src/ns3-rmcat/tools/test.csh wired 2017-07-21-rmcat-wired
    # ./src/ns3-rmcat/tools/test.csh vparam 2017-07-21-rmcat-wired-vparam
    # ./src/ns3-rmcat/tools/test.csh wifi 2017-07-21-rmcat-wifi
    #
    # The second parameter, output directory, is optional. If not specified,
    # the script will use a folder with a name based on current GMT time


Note that in ns-3.26, the testing script (test.py) only works with python2. So one may want to point the python alias to python to ensure that the `test.csh` script runs out of box:

::
    alias python=python2.7.3
::

Alternatively, you can use `test_v2.csh <tools/test_v2.csh>`_ to explicitly invoke python2 for running the testing script and python3 for running the processing and plotting scripts.  The latter works with both python2 and python3.


Troubleshooting
*****************

To build ns-3.26 on newer compilers: see tips `here <https://www.nsnam.org/wiki/HOWTO_build_old_versions_of_ns-3_on_newer_compilers>`_. To disable warnings from breaking your build, do the following:
::

      CXXFLAGS="-Wall" ./waf configure
      ./waf -vv
::



To debug "rmcat-wired" test suite:

::

    ./waf --command-template="gdb %s" --run "test-runner"
    r --assert-on-failure --suite=rmcat-wired

To debug rmcat example, enter ns3 source directory:

::

    ./waf --command-template="gdb %s" --run src/ns3-rmcat/examples/rmcat-example

Future work
**********************************

Adding LTE topology and test cases

Add support for ECN marking

Encapsulate Sender's rate shaping buffer in a C++ interface or class

Wired test cases: implement time-varying bottleneck capacity by changing the physical link properties
