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
 * Test suite for rmcat wired test cases, obtained by varying various parameters.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#include "rmcat-wired-test-case.h"

/*
 * Implementation of the RmcatVaryParamTestSuite class,
 * which instantiates the collection of test
 * cases as specified in the following IETF draft
 * (rmcat-eval-test):
 *
 * Test Cases for Evaluating RMCAT Proposals
 * https://tools.ietf.org/html/draft-ietf-rmcat-eval-test-05
 */

/*
 * Defines collection of test cases with varying
 * parameters (e.g., bottleneck BW, propagation
 * delay) for specific test cases in the rmcat-eval-test
 * draft
 */
class RmcatVaryParamTestSuite : public TestSuite
{
public:
  RmcatVaryParamTestSuite ();
};

RmcatVaryParamTestSuite::RmcatVaryParamTestSuite ()
  : TestSuite{"rmcat-vparam", UNIT}
{
    // ----------------
    // Default test case parameters
    // -----------------
    uint32_t qdel = 300;            // bottleneck queuing delay:    300ms
    uint32_t simT = 300;            // default simulation duration: 120s

    // TODO (deferred): decide where to specify default TCP behavior (currently duplicated
    // in rmcat-wired and rmcat-wifi test suites)
    //
    // Default TCP configuration
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (RMCAT_TC_TCP_PKTSIZE));
    Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));
    Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (RMCAT_TC_TCP_RECVBUF_SIZE));
    Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (RMCAT_TC_TCP_RECVBUF_SIZE));

    // -----------------------
    // Test Case 5.6: Media Flow Competing with a Long TCP Flow
    // -----------------------
    // configure TCP flow start/end time
    std::vector<uint32_t> tstartTC56; // Seconds
    std::vector<uint32_t> tstopTC56; // Seconds
    tstartTC56.push_back (60);
    tstopTC56.push_back (240);

    /*
     * Duplicating Julius's Tests
     */
    std::vector<uint32_t> pdellist;
    std::vector<uint64_t> bwlist;
    pdellist.push_back (20);
    pdellist.push_back (40);
    pdellist.push_back (60);
    pdellist.push_back (80);
    pdellist.push_back (100);

    bwlist.push_back (400 * (1u << 10));  // 400Kbps
    bwlist.push_back (600 * (1u << 10));  // 600Kbps
    bwlist.push_back (800 * (1u << 10));  // 800Kbps
    bwlist.push_back (1000 * (1u << 10));    // 1 Mbps
    bwlist.push_back (1200 * (1u << 10));    // 1.2 Mbps
    bwlist.push_back (1600 * (1u << 10));    // 1.6 Mbps
    bwlist.push_back (2000 * (1u << 10));    // 2 Mbps
    bwlist.push_back (4000 * (1u << 10));    // 4 Mbps
    bwlist.push_back (6000 * (1u << 10));    // 6 Mbps
    bwlist.push_back (10000 * (1u << 10));    // 10 Mbps

    for (size_t i = 0; i < bwlist.size (); ++i) {
        uint64_t bw_i = bwlist[i];
        for (size_t j = 0; j < pdellist.size (); ++j) {
            uint32_t pdel_j = pdellist[j];

            std::stringstream ss;
            ss << "rmcat-test-case-5.6-C" << bw_i / (1u << 10) << "-pdel" << pdel_j;
            RmcatWiredTestCase * tc56tmp = new RmcatWiredTestCase{bw_i, pdel_j, qdel, ss.str ()};
            tc56tmp->SetSimTime (simT);                // Simulation time: 300s
            tc56tmp->SetTCPLongFlows (1, tstartTC56, tstopTC56, true);    // Forward path

            AddTestCase (tc56tmp, TestCase::QUICK);
        }
    }
}

static RmcatVaryParamTestSuite rmcatVparamTestSuite;
