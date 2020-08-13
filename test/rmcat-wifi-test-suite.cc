/******************************************************************************
 * Copyright 2016-2017 Cisco Systems, Inc.                                    *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 *                                                                            *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 ******************************************************************************/

/**
 * @file
 * Test suite for rmcat wifi test cases.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

/*
 * This file contains implementation of the
 * RmcatWifiTestSuite class. It instantiates
 * the collection of wifi test cases as described
 * in Section 4 of the following IETF draft:
 *
 * Evaluation Test Cases for Interactive Real-Time
 * Media over Wireless Networks
 * https://tools.ietf.org/html/draft-ietf-rmcat-wireless-tests-04
 *
 */

#include "rmcat-wifi-test-case.h"

/*
 * Defines common configuration parameters of a
 * RMCAT wifi test case.
 *
 * Implement common procedures in setting up a
 * test case:
 * -- configuration of network topology, including
 *    number of wired and wireless nodes;
 * -- configuration of capacity and propagation
 *    delay of the wired connection
 * -- configuration of PHY- and MAC-layer parameters
 *    for the wireless connections
 * -- configuration of traversing traffic flows
 *    in terms of traffic type (i.e., RMCAT/TCP/UDP),
 *    arrival and departure patterns
 */

/*
 * Defines collection of test cases as specified in
 * Section 4 of the rmcat-wireless-tests draft
 */
class RmcatWifiTestSuite : public TestSuite
{
public:
    RmcatWifiTestSuite ();
};

RmcatWifiTestSuite :: RmcatWifiTestSuite ()
    : TestSuite{"rmcat-wifi", UNIT}
{
    // ----------------
    // Default test case parameters
    // -----------------
    uint64_t bw  = 1 * (1u << 20);   // wired bottleneck capacity: 1Mbps
    uint32_t pdel = 50;              // forward propagation delay: 50ms
    uint32_t qdel = 300;             // wired bottleneck queue depth: 300ms
    uint64_t rCBR = 300* (1u << 10); // rate of each CBR background flow: 300Kbps
    uint32_t simT = 120;             // default simulation duration: 120s
    uint32_t nRMCAT = 1;             // place-holder param for # of RMCAT flows

    ns3::WifiMode phyMode = WifiMode ("HtMcs11"); // default PHY mode
    const std::vector<uint32_t> t0s; // null time vector as filler

    // Default TCP configuration
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (RMCAT_TC_TCP_PKTSIZE));
    Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));
    // Uncomment these lines if you wish to modify TCP's send or receive buffer sizes
    // Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (RMCAT_TC_TCP_RECVBUF_SIZE));
    // Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (RMCAT_TC_TCP_RECVBUF_SIZE));

    /* start/stop times of background CBR traffic, single flow */
    uint32_t nCBR = 1;
    std::vector<uint32_t> tstartCBRTC41; // Seconds
    std::vector<uint32_t> tstopCBRTC41;  // Seconds
    tstartCBRTC41.push_back (RMCAT_TC_BG_TSTART);  // t_start = 40s
    tstopCBRTC41.push_back (RMCAT_TC_BG_TFINIS);   // t_stop = 80s

    /* start/stop times of background TCP traffic; single flow */
    uint32_t nTCP = 1;
    std::vector<uint32_t> tstartTCPTC41; // Seconds
    std::vector<uint32_t> tstopTCPTC41;  // Seconds
    tstartTCPTC41.push_back (RMCAT_TC_BG_TSTART);  // t_start = 40s
    tstopTCPTC41.push_back (RMCAT_TC_BG_TFINIS);   // t_stop = 80s

    // -----------------------
    // Test Case 4.1.a: Wired Bottleneck; Single downlink RMCAT flow [10|00|00]
    // -----------------------
    RmcatWifiTestCase * tc41a = new RmcatWifiTestCase{bw, pdel, qdel, "rmcat-wifi-test-case-4.1.a"};
    tc41a->SetSimTime (simT);
    tc41a->SetPHYMode (phyMode);
    tc41a->SetRMCATFlows (1, t0s, t0s, true);
    tc41a->SetRMCATFlows (0, t0s, t0s, false);

    // -----------------------
    // Test Case 4.1.b: Wired Bottleneck; Single uplink RMCAT flow [01|00|00]
    // -----------------------
    RmcatWifiTestCase * tc41b = new RmcatWifiTestCase{bw, pdel, qdel, "rmcat-wifi-test-case-4.1.b"};
    tc41b->SetSimTime (simT);
    tc41b->SetPHYMode (phyMode);
    tc41b->SetRMCATFlows (0, t0s, t0s, true);
    tc41b->SetRMCATFlows (1, t0s, t0s, false);

    // -----------------------
    // Test Case 4.1.c: Wired Bottleneck; Bi-directional RMCAT flows [11|00|00]
    // -----------------------
    RmcatWifiTestCase * tc41c = new RmcatWifiTestCase{bw, pdel, qdel, "rmcat-wifi-test-case-4.1.c"};
    tc41c->SetSimTime (simT);
    tc41c->SetPHYMode (phyMode);
    tc41c->SetRMCATFlows (1, t0s, t0s, true);
    tc41c->SetRMCATFlows (1, t0s, t0s, false);

    // -----------------------
    // Test Case 4.1.d: Wired Bottleneck;
    //      Bi-directional RMCAT flow + one downlink UDP background flow
    //      [11|10|00]
    // -----------------------
    RmcatWifiTestCase * tc41d = new RmcatWifiTestCase{bw, pdel, qdel, "rmcat-wifi-test-case-4.1.d"};
    tc41d->SetSimTime (simT);
    tc41d->SetPHYMode (phyMode);
    tc41d->SetCBRRate (rCBR);
    tc41d->SetRMCATFlows (1, t0s, t0s, true);
    tc41d->SetRMCATFlows (1, t0s, t0s, false);
    tc41d->SetUDPFlows (nCBR, tstartCBRTC41, tstopCBRTC41, true);
    tc41d->SetUDPFlows (0, t0s, t0s, false);

    // -----------------------
    // Test Case 4.1.e: Wired Bottleneck;
    //      Bi-directional RMCAT flow + one uplink UDP background flow
    //      [11|01|00]
    // -----------------------
    RmcatWifiTestCase * tc41e = new RmcatWifiTestCase{bw, pdel, qdel, "rmcat-wifi-test-case-4.1.e"};
    tc41e->SetSimTime (simT);
    tc41e->SetPHYMode (phyMode);
    tc41e->SetCBRRate (rCBR);
    tc41e->SetRMCATFlows (1, t0s, t0s, true);
    tc41e->SetRMCATFlows (1, t0s, t0s, false);
    tc41e->SetUDPFlows (0, t0s, t0s, true);
    tc41e->SetUDPFlows (nCBR, tstartCBRTC41, tstopCBRTC41, false);

    // -----------------------
    // Test Case 4.1.f: Wired Bottleneck;
    //      Bi-directional RMCAT flow + one downlink TCP background flow
    //      [11|00|10]
    // -----------------------
    RmcatWifiTestCase * tc41f = new RmcatWifiTestCase{bw, pdel, qdel, "rmcat-wifi-test-case-4.1.f"};
    tc41f->SetSimTime (simT);
    tc41f->SetPHYMode (phyMode);
    tc41f->SetRMCATFlows (1, t0s, t0s, true);
    tc41f->SetRMCATFlows (1, t0s, t0s, false);

    tc41f->SetTCPFlows (nTCP, tstartTCPTC41, tstopTCPTC41, true);
    tc41f->SetTCPFlows (0, t0s, t0s, false);

    // -----------------------
    // Test Case 4.1.g: Wired Bottleneck;
    //      Bi-directional RMCAT flow + one uplink TCP background flow
    //      [11|00|01]
    // -----------------------
    RmcatWifiTestCase * tc41g = new RmcatWifiTestCase{bw, pdel, qdel, "rmcat-wifi-test-case-4.1.g"};
    tc41g->SetSimTime (simT);
    tc41g->SetPHYMode (phyMode);
    tc41g->SetRMCATFlows (1, t0s, t0s, true);
    tc41g->SetRMCATFlows (1, t0s, t0s, false);

    tc41g->SetTCPFlows (0, t0s, t0s, true);
    tc41g->SetTCPFlows (nTCP, tstartTCPTC41, tstopTCPTC41, false);

    /*
     * Add collection of wired bottleneck test cases
     * (Section 4.1. in rmcat-wireless-tests draft)
     * to test suite
     */
    AddTestCase (tc41a, TestCase::QUICK);
    AddTestCase (tc41b, TestCase::QUICK);
    AddTestCase (tc41c, TestCase::QUICK);
    AddTestCase (tc41d, TestCase::QUICK);
    AddTestCase (tc41e, TestCase::QUICK);
    AddTestCase (tc41f, TestCase::QUICK);
    AddTestCase (tc41g, TestCase::QUICK);

    // -----------------------
    // Test Case 4.2.x: Wireless Bottleneck
    // -----------------------
    bw = 100 * (1u << 20);   // default wired capacity: 100Mbps
    rCBR = 600 * (1u << 10); // rate of each CBR-over-UDP flow: 600Kbps

    std::vector<uint32_t> nFlows;
    nFlows.push_back (8);
    nFlows.push_back (12);
    nFlows.push_back (16);
    for (size_t i = 0; i<nFlows.size (); ++i) {
        uint32_t n = nFlows[i];
        std::stringstream ass;
        std::stringstream bss;
        std::stringstream css;

        // -----------------------
        // Test Case 4.2.a: Wireless Bottleneck;
        //     Multiple downlink RMCAT flows
        // -----------------------
        ass << "rmcat-wifi-test-case-4.2.a-n" << n * 2;
        RmcatWifiTestCase * tc42a = new RmcatWifiTestCase{bw, pdel, qdel, ass.str ()};
        tc42a->SetSimTime (simT);
        tc42a->SetPHYMode (phyMode);
        tc42a->SetRMCATFlows (n * 2, t0s, t0s, true);
        tc42a->SetRMCATFlows (0, t0s, t0s, false);

        // -----------------------
        // Test Case 4.2.b: Wireless Bottleneck;
        //     Multiple uplink RMCAT flows
        // -----------------------
        bss << "rmcat-wifi-test-case-4.2.b-n" << n * 2;
        RmcatWifiTestCase * tc42b = new RmcatWifiTestCase{bw, pdel, qdel, bss.str ()};
        tc42b->SetSimTime (simT);
        tc42b->SetPHYMode (phyMode);
        tc42b->SetRMCATFlows (0, t0s, t0s, true);
        tc42b->SetRMCATFlows (n * 2, t0s, t0s, false);

        // -----------------------
        // Test Case 4.2.c: Wireless Bottleneck;
        //     Multiple bi-directional RMCAT flows
        // -----------------------
        css << "rmcat-wifi-test-case-4.2.c-n" << n * 2;
        RmcatWifiTestCase * tc42c = new RmcatWifiTestCase{bw, pdel, qdel, css.str ()};
        tc42c->SetSimTime (simT);
        tc42c->SetPHYMode (phyMode);
        tc42c->SetRMCATFlows (n, t0s, t0s, true);
        tc42c->SetRMCATFlows (n, t0s, t0s, false);

        /* Add test cases to test suite */
        // You can comment out these lines if you wish to reduce the time
        //    it takes to run the suite, as these test cases take a while
        AddTestCase (tc42a,TestCase::QUICK);
        AddTestCase (tc42b,TestCase::QUICK);
        AddTestCase (tc42c,TestCase::QUICK);
    }

    // -----------------------
    // Test Case 4.2.d: Wireless Bottleneck;
    //     Multiple bi-directional RMCAT flows
    //     + multiple uplink CBR-over-UDP flows
    // -----------------------
    nRMCAT = 12;
    nCBR   = 4;
    /* start/stop times of background CBR traffic, multiple flows */
    std::vector<uint32_t> tstartCBRTC42; // Seconds
    std::vector<uint32_t> tstopCBRTC42;  // Seconds
    for (uint32_t i = 0; i < nCBR; ++i) {
        tstartCBRTC42.push_back (RMCAT_TC_BG_TSTART);  // t_start = 40s
        tstopCBRTC42.push_back (RMCAT_TC_BG_TFINIS);   // t_stop = 80s
    }

    std::stringstream dss;
    dss << "rmcat-wifi-test-case-4.2.d-n" << nRMCAT*2;
    RmcatWifiTestCase * tc42d = new RmcatWifiTestCase{bw, pdel, qdel, dss.str ()};
    tc42d->SetSimTime (simT);
    tc42d->SetPHYMode (phyMode);
    tc42d->SetCBRRate (rCBR);
    tc42d->SetRMCATFlows (nRMCAT, t0s, t0s, true);
    tc42d->SetRMCATFlows (nRMCAT, t0s, t0s, false);
    tc42d->SetUDPFlows (0, t0s, t0s, true);
    tc42d->SetUDPFlows (nCBR, tstartCBRTC42, tstopCBRTC42, false);

    // -----------------------
    // Test Case 4.2.f: Wireless Bottleneck;
    //     Multiple bi-directional RMCAT flows
    //     + multiple uplink TCP flows
    // -----------------------
    nRMCAT = 12;
    nTCP = 4;

    /* start/stop times of background TCP traffic; single flow */
    std::vector<uint32_t> tstartTCPTC42; // Seconds
    std::vector<uint32_t> tstopTCPTC42;  // Seconds
    for (uint32_t i = 0; i < nTCP; ++i) {
        tstartTCPTC42.push_back (RMCAT_TC_BG_TSTART);  // t_start = 40s
        tstopTCPTC42.push_back (RMCAT_TC_BG_TFINIS);   // t_stop = 80s
    }
    std::stringstream ess;
    ess << "rmcat-wifi-test-case-4.2.e-n" << nRMCAT * 2;
    RmcatWifiTestCase * tc42e = new RmcatWifiTestCase{bw, pdel, qdel, ess.str ()};
    tc42e->SetSimTime (simT);
    tc42e->SetPHYMode (phyMode);
    tc42e->SetRMCATFlows (nRMCAT, t0s, t0s, true);
    tc42e->SetRMCATFlows (nRMCAT, t0s, t0s, false);
    tc42e->SetTCPFlows (0, t0s, t0s, true);
    tc42e->SetTCPFlows (nTCP, tstartTCPTC42, tstopTCPTC42, false);

    /*
     * Add collection of wi-fi bottleneck test cases
     * (Section 4.2. in rmcat-wireless-tests draft)
     * to test suite
     */
    AddTestCase (tc42d,TestCase::QUICK);
    AddTestCase (tc42e,TestCase::QUICK);
}

static RmcatWifiTestSuite rmcatWifiTestSuite;
