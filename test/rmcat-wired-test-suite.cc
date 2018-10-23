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
 * Test suite for rmcat wired test cases.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#include "rmcat-wired-test-case.h"

/*
 * Implementation of the RmcatTestSuite class,
 * which instantiates the collection of test
 * cases as specified in the following IETF draft
 * (rmcat-eval-test):
 *
 * Test Cases for Evaluating RMCAT Proposals
 * https://tools.ietf.org/html/draft-ietf-rmcat-eval-test-05
 */

/*
 * Defines collection of test cases as specified in
 * the rmcat-eval-test draft
 */
class RmcatTestSuite : public TestSuite
{
public:
  RmcatTestSuite ();
};

RmcatTestSuite::RmcatTestSuite ()
  : TestSuite{"rmcat-wired", UNIT}
{
    // ----------------
    // Default test case parameters
    // -----------------
    uint64_t bw =  4 * (1u << 20);  // capacity: 4Mbps
    uint32_t pdel = 50;             // one-way propagation delay:   50ms
    uint32_t qdel = 300;            // bottleneck queuing delay:    300ms
    uint32_t simT = 120;            // default simulation duration: 120s

    // null time vector as filler
    const std::vector<uint32_t> t0s;

    // TODO (deferred): decide where to specify default TCP behavior (currently duplicated
    // in rmcat-wired and rmcat-wifi test suites)
    //
    // Default TCP configuration
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (RMCAT_TC_TCP_PKTSIZE));
    Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));
    // Uncomment these lines if you wish to modify TCP's send or receive buffer sizes
    // Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (RMCAT_TC_TCP_RECVBUF_SIZE));
    // Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (RMCAT_TC_TCP_RECVBUF_SIZE));

    // TODO (deferred): Set up a mechanism (e.g., json-based) to load TC setup. Goal: improve readability

    // -----------------------
    // Test Case 5.1: Variable Available Capacity with a Single Flow
    // -----------------------
    std::vector<uint32_t> timeTC51; // in seconds
    std::vector<uint64_t> bwTC51;   // in bps
    timeTC51.push_back (0);   bwTC51.push_back        (1u << 20);  // 1 Mbps
    timeTC51.push_back (40);  bwTC51.push_back (2.5 * (1u << 20)); // 2.5 Mbps
    timeTC51.push_back (60);  bwTC51.push_back ( .6 * (1u << 20)); // 0.6 Mbps
    timeTC51.push_back (80);  bwTC51.push_back        (1u << 20);  // 1 Mbps

    RmcatWiredTestCase * tc51a = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.1-fixfps"};
    tc51a->SetSimTime (100); // simulation time: 100s
    tc51a->SetBW (timeTC51, bwTC51, true); // FWD path

    RmcatWiredTestCase * tc51b = new RmcatWiredTestCase{bw, 100, qdel, "rmcat-test-case-5.1-fixfps-pdel_100ms"};
    tc51b->SetSimTime (100); // simulation time: 100s
    tc51b->SetBW (timeTC51, bwTC51, true); // FWD path

    RmcatWiredTestCase * tc51c = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.1-cbrlike"};
    tc51c->SetSimTime (100); // simulation time: 100s
    tc51c->SetBW (timeTC51, bwTC51, true); // FWD path
    tc51c->SetCodec (SYNCODEC_TYPE_PERFECT); // CBR-like traffic source

    RmcatWiredTestCase * tc51d = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.1-stats"};
    tc51d->SetSimTime (100); // simulation time: 100s
    tc51d->SetBW (timeTC51, bwTC51, true); // FWD path
    tc51d->SetCodec (SYNCODEC_TYPE_STATS); // statistical video source

    RmcatWiredTestCase * tc51e = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.1-trace"};
    tc51e->SetSimTime (100); // simulation time: 100s
    tc51e->SetBW (timeTC51, bwTC51, true); // FWD path
    tc51e->SetCodec (SYNCODEC_TYPE_TRACE); // trace-based video source

    RmcatWiredTestCase * tc51f = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.1-sharing"};
    tc51f->SetSimTime (100); // simulation time: 100s
    tc51f->SetBW (timeTC51, bwTC51, true); // FWD path
    tc51f->SetCodec (SYNCODEC_TYPE_SHARING); // content-sharing video source

    RmcatWiredTestCase * tc51g = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.1-hybrid"};
    tc51g->SetSimTime (100); // simulation time: 100s
    tc51g->SetBW (timeTC51, bwTC51, true); // FWD path
    tc51g->SetCodec (SYNCODEC_TYPE_HYBRID); // hybrid (trace/statistics) video source

    // -----------------------
    // Test Case 5.2: Variable Available Capacity with Multiple Flows
    // -----------------------
    std::vector<uint32_t> timeTC52;
    std::vector<uint64_t> bwTC52;
    timeTC52.push_back (0);   bwTC52.push_back (2.   * (1u << 21)); // 2 * 2 Mbps
    timeTC52.push_back (25);  bwTC52.push_back         (1u << 21);  // 2 Mbps
    timeTC52.push_back (50);  bwTC52.push_back (1.75 * (1u << 21)); // 1.75 * 2 Mbps
    timeTC52.push_back (75);  bwTC52.push_back ( .5  * (1u << 21)); // 0.5 * 2 Mbps
    timeTC52.push_back (100); bwTC52.push_back         (1u << 21);  // 2 Mbps

    RmcatWiredTestCase * tc52 = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.2-fixfps"};
    tc52->SetSimTime (125); // simulation time: 125s
    tc52->SetBW (timeTC52, bwTC52, true);
    tc52->SetRMCATFlows (2, t0s, t0s, true);

    // -----------------------
    // Test Case 5.3: Congested Feedback Link with Bi-directional Media Flows
    // -----------------------
    std::vector<uint32_t> timeTC53fwd;
    std::vector<uint32_t> timeTC53bwd;
    std::vector<uint64_t> bwTC53fwd;
    std::vector<uint64_t> bwTC53bwd;
    timeTC53fwd.push_back (0);  bwTC53fwd.push_back (2.  * (1u << 20)); // 2 Mbps
    timeTC53fwd.push_back (20); bwTC53fwd.push_back (1.  * (1u << 20)); // 1 Mbps
    timeTC53fwd.push_back (40); bwTC53fwd.push_back ( .5 * (1u << 20)); // 0.5 Mbps
    timeTC53fwd.push_back (60); bwTC53fwd.push_back (2.  * (1u << 20)); // 2 Mbps

    timeTC53bwd.push_back (0);  bwTC53bwd.push_back (2.  * (1u << 20)); // 2 Mbps
    timeTC53bwd.push_back (35); bwTC53bwd.push_back ( .8 * (1u << 20)); // 0.8 Mbps
    timeTC53bwd.push_back (70); bwTC53bwd.push_back (2.  * (1u << 20)); // 2 Mbps

    RmcatWiredTestCase * tc53 = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.3-fixfps"};
    tc53->SetSimTime (100); // simulation time: 100s
    tc53->SetBW (timeTC53fwd, bwTC53fwd, true);  // Forward path
    tc53->SetBW (timeTC53bwd, bwTC53bwd, false); // Backward path
    tc53->SetRMCATFlows (1, t0s, t0s, true);     // Forward path
    tc53->SetRMCATFlows (1, t0s, t0s, false);    // Backward path

    // -----------------------
    // Test Case 5.4: Competing Media Flows with same Congestion Control Algorithm
    // -----------------------
    std::vector<uint32_t> tstartTC54; // Seconds
    std::vector<uint32_t> tstopTC54; // Seconds
    tstartTC54.push_back (0);  tstopTC54.push_back (119);
    tstartTC54.push_back (20);  tstopTC54.push_back (119);
    tstartTC54.push_back (40);  tstopTC54.push_back (119);
    RmcatWiredTestCase * tc54 = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.4-fixfps"};
    tc54->SetCapacity (3.5 * (1u << 20));  // bottleneck capacity: 3.5 Mbps
    tc54->SetSimTime (simT); // default simulation time: 120s
    tc54->SetRMCATFlows (3, tstartTC54, tstopTC54, true);    // Forward path

    // -----------------------
    // Test Case 5.5: Round Trip Time Fairness
    // -----------------------
    // configure per-flow one-way propagation delay
    std::vector<uint32_t> pDelaysTC55; // ms
    pDelaysTC55.push_back (10);   // 1st flow: 10ms
    pDelaysTC55.push_back (25);   // 2nd flow: 25ms
    pDelaysTC55.push_back (50);   // 3rd flow: 50ms
    pDelaysTC55.push_back (100);  // 4th flow: 100ms
    pDelaysTC55.push_back (150);  // 5th flow: 150ms

    // configure per-flow start/stop time
    std::vector<uint32_t> tstartTC55; // Seconds
    std::vector<uint32_t> tstopTC55; // Seconds
    tstartTC55.push_back (0);  tstopTC55.push_back (299);
    tstartTC55.push_back (10);  tstopTC55.push_back (299);
    tstartTC55.push_back (20);  tstopTC55.push_back (299);
    tstartTC55.push_back (30);  tstopTC55.push_back (299);
    tstartTC55.push_back (40);  tstopTC55.push_back (299);
    RmcatWiredTestCase * tc55 = new RmcatWiredTestCase{bw, 10, qdel, "rmcat-test-case-5.5-fixfps"};
    tc55->SetSimTime (300); // simulation time: 300s
    tc55->SetRMCATFlows (5, tstartTC55, tstopTC55, true);  // Forward path
    tc55->SetPropDelays (pDelaysTC55);

    // -----------------------
    // Test Case 5.6: Media Flow Competing with a Long TCP Flow
    // -----------------------
    // configure TCP flow start/end time
    std::vector<uint32_t> tstartTC56; // Seconds
    std::vector<uint32_t> tstopTC56; // Seconds
    tstartTC56.push_back (5); tstopTC56.push_back (119);

    RmcatWiredTestCase * tc56 = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.6-fixfps"};
    tc56->SetCapacity (2 * (1u << 20));     // Bottleneck capacity: 2Mbps
    tc56->SetSimTime (simT);                // Default simulation time: 120s
    tc56->SetRMCATFlows (1, tstartTC56, tstopTC56, true); // Forward path
    tc56->SetTCPLongFlows (1, t0s, t0s, true); // Forward path
    // TODO (deferred): Bottleneck queue sizes: [300ms, 1000ms]

    // -----------------------
    // Test Case 5.7: Media Flow Competing with Short TCP Flows
    // -----------------------
    std::vector<uint32_t> tstartTC57; // Seconds
    std::vector<uint32_t> tstopTC57; // Seconds
    tstartTC57.push_back (5); tstopTC57.push_back (299);
    tstartTC57.push_back (5); tstopTC57.push_back (299);

    RmcatWiredTestCase * tc57 = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.7-fixfps"};
    tc57->SetCapacity (2 * (1u << 20)); // Bottleneck capacity: 2Mbps
    tc57->SetSimTime (300);                  // Simulation time: 300s
    tc57->SetRMCATFlows (2, tstartTC57, tstopTC57, true); // Forward path
    tc57->SetTCPShortFlows (10, 2, true);    // Forward path

    // -----------------------
    // Test Case 5.8: Media Pause and Resume (Modified from TC5.4)
    // -----------------------

    // configure flow pause/resume timeline
    uint32_t fid8 = 1; // flowID to be paused & resumed
    std::vector<uint32_t> tpauseTC58; // Seconds
    std::vector<uint32_t> tresumeTC58; // Seconds
    tpauseTC58.push_back (40);
    tresumeTC58.push_back (60);

    RmcatWiredTestCase * tc58 = new RmcatWiredTestCase{bw, pdel, qdel, "rmcat-test-case-5.8-fixfps"};
    tc58->SetCapacity (3.5 * (1u << 20));  // bottleneck capacity: 3.5 Mbps (same as TC5.4)
    tc58->SetSimTime (simT); // default simulation time: 120s (same as TC5.4)
    tc58->SetRMCATFlows (3, t0s, t0s, true);  // Forward path
    tc58->SetPauseResumeTimes (fid8, tpauseTC58, tresumeTC58, true);

    // -------------------------------
    // Add test cases to test suite
    // -------------------------------

    AddTestCase (tc51a, TestCase::QUICK);
    AddTestCase (tc51b, TestCase::QUICK);
    AddTestCase (tc51c, TestCase::QUICK);
    AddTestCase (tc51d, TestCase::QUICK);
    AddTestCase (tc51e, TestCase::QUICK);
    AddTestCase (tc51f, TestCase::QUICK);
    AddTestCase (tc51g, TestCase::QUICK);

    AddTestCase (tc52, TestCase::QUICK);

    AddTestCase (tc53, TestCase::QUICK);
    AddTestCase (tc54, TestCase::QUICK);
    AddTestCase (tc55, TestCase::QUICK);
    AddTestCase (tc56, TestCase::QUICK);
    AddTestCase (tc57, TestCase::QUICK);
    AddTestCase (tc58, TestCase::QUICK);
}

static RmcatTestSuite rmcatTestSuite;
