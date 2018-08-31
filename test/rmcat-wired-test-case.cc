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

NS_LOG_COMPONENT_DEFINE ("RmcatSimTestWired");

/*
 * Implementation of the RmcatWiredTestCase
 * class. It defines the template for individual
 * rmcat-wired test cases as specified in
 * the following IETF draft (rmcat-eval-test):
 *
 * Test Cases for Evaluating RMCAT Proposals
 * https://tools.ietf.org/html/draft-ietf-rmcat-eval-test-05
 *
 * The RmcatTestSuite class simply instantiates
 * the collection of specific test cases as
 * defined in the rmcat-eval-test draft.
 */

// TODO (deferred):  Jitter model
// TODO (deferred):  Audio + Video combined
// TODO (deferred):  align topology implementation with wifi case

static void SenderPauseResume (Ptr<RmcatSender> send, bool pause)
{
    send->PauseResume (pause);
}

/* Constructor */
RmcatWiredTestCase::RmcatWiredTestCase (uint64_t capacity, // bottleneck capacity (in bps)
                                        uint32_t delay,    // one-way propagation delay (in ms)
                                        uint32_t qdelay,   // bottleneck queue depth (in ms)
                                        std::string desc)  // test case name/description
: RmcatTestCase{capacity, delay, qdelay, desc},
  m_numFlowsFw{1},  // default: a single forward RMCAT flow
  m_numFlowsBw{0},
  m_numTcpFlows{0},
  m_numShortTcpFlows{0},
  m_numInitOnFlows{0},
  m_simTime{RMCAT_TC_SIMTIME},
  m_pauseFid{0},
  m_codecType{SYNCODEC_TYPE_FIXFPS}
{}


/*
 * Configure time-varying available bandwidth
 */
void RmcatWiredTestCase::SetBW (const std::vector<uint32_t>& times,
                                const std::vector<uint64_t>& capacities,
                                bool fwd)
{
    if (capacities.size () > 0) {
        NS_ASSERT (capacities.size () == times.size ());
        NS_ASSERT (m_capacity >= *std::max_element (capacities.begin (), capacities.end ()));
        NS_ASSERT (times[0] == 0);

        if (fwd) {
            m_timesFw = times;
            m_capacitiesFw = capacities;
        } else {
            m_timesBw = times;
            m_capacitiesBw = capacities;
        }
    }
}

/*
 * Configure media pause/resume times for RMCAT flows
 *
 *  Only the forward direction is supported for now.
 */
void RmcatWiredTestCase::SetPauseResumeTimes (size_t fid,
                                              const std::vector<uint32_t> & ptimes,
                                              const std::vector<uint32_t> & rtimes,
                                              bool fwd)
{
    NS_ASSERT (fwd); // Only fwd direction supported for now
    NS_ASSERT (fid < m_numFlowsFw);
    NS_ASSERT (ptimes.size () == rtimes.size ());

    m_pauseFid = fid;
    m_pauseTimes = ptimes;
    m_resumeTimes = rtimes;
}

/*
 *  Configure RMCAT flows in terms of:
 *  -- direction (forward/backward)
 *  -- arrival/departure times
*/
void RmcatWiredTestCase::SetRMCATFlows (size_t numFlows,
                                        const std::vector<uint32_t>& startTimes,
                                        const std::vector<uint32_t>& endTimes,
                                        bool fwd)
{
    if (fwd) {
        // configure forward-direction flows
        m_numFlowsFw   = numFlows;
        m_startTimesFw = startTimes;
        m_endTimesFw = endTimes;
    } else {
        // configure backward-direction flows
        m_numFlowsBw = numFlows;
        m_startTimesBw = startTimes;
        m_endTimesBw = endTimes;
    }
}


/*
 *  Configure long lived TCP background flows
 *  in terms of arrival/departure times.
 *
 *  @note Only the forward direction is supported for now.
 */
void RmcatWiredTestCase::SetTCPLongFlows (size_t numFlows,
                                          const std::vector<uint32_t>& startTimes,
                                          const std::vector<uint32_t>& endTimes,
                                          bool fwd)
{
    NS_ASSERT (fwd);
    m_numTcpFlows   = numFlows;
    m_startTimesTcp = startTimes;
    m_endTimesTcp   = endTimes;
}

/**
 *  Configure a collection of short-lived TCP background flows.
 *
 *  Arrival/departure patterns of the short-lived flows are specified in
 *  in function #SetUpTCPShort as random processes
 *
 *  @note Only the forward direction is supported for now.
 */
void RmcatWiredTestCase::SetTCPShortFlows (size_t numFlows,
                                           size_t numInitOnFlows,
                                           bool fwd)
{
    NS_ASSERT (fwd);
    m_numShortTcpFlows = numFlows;
    m_numInitOnFlows = numInitOnFlows;

}

/*
 * Inherited DoSetup function:
 * -- Build network topology
 * -- Enable additional logging
 */
void RmcatWiredTestCase::DoSetup ()
{
    RmcatTestCase::DoSetup ();
    m_topo.Build (m_capacity, m_delay, m_qdelay);
    ns3::LogComponentEnable ("RmcatSimTestWired", LOG_LEVEL_INFO);
}

/*
 * Inherited DoRun () function:
 * -- Instantiate RMCAT and TCP background flows
 * -- Populate routing table
 * -- Kickoff simulation
 */
void RmcatWiredTestCase::DoRun ()
{

    NS_ASSERT (m_startTimesFw.size () == m_endTimesFw.size ());
    NS_ASSERT (m_startTimesFw.size () == 0 || m_startTimesFw.size () == m_numFlowsFw);
    NS_ASSERT (m_pDelays.size () == 0 || m_pDelays.size () == m_numFlowsFw);

    NS_ASSERT (m_pauseTimes.size () == m_resumeTimes.size ()); // always pause-resume pairs
    NS_ASSERT (m_startTimesTcp.size () == m_endTimesTcp.size ());
    NS_ASSERT (m_startTimesTcp.size () == 0 || m_startTimesTcp.size () == m_numTcpFlows);

    /*
     * Configure forward direction path and traffic
    */
    std::vector<Ptr<RmcatSender> > sendFw (m_numFlowsFw);
    std::vector<std::shared_ptr<Timer> > ptimersFw;
    std::vector<std::shared_ptr<Timer> > rtimersFw;
    std::vector<Ptr<BulkSendApplication> > tcpLongSend (m_numTcpFlows);
    std::vector<Ptr<BulkSendApplication> > tcpShortSend;

    SetUpPath (m_timesFw, m_capacitiesFw, true);     // time-varying available BW
    SetUpRMCAT (sendFw, ptimersFw, rtimersFw, true); // instantiate forward RMCAT flows
    SetUpTCPLong (m_numTcpFlows, tcpLongSend);       // instantiate background long lived TCP flows
    SetUpTCPShort (m_numShortTcpFlows,
                   m_numInitOnFlows,
                   tcpShortSend);  // instantiate background short TCP flows

    /*
     * Configure backward direction path and traffic
     */
    std::vector<Ptr<RmcatSender> > sendBw (m_numFlowsBw);
    std::vector<std::shared_ptr<Timer> > ptimersBw;
    std::vector<std::shared_ptr<Timer> > rtimersBw;

    SetUpPath (m_timesBw, m_capacitiesBw, false);
    SetUpRMCAT (sendBw, ptimersBw, rtimersBw, false);

    /* Populate routing table */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    /* Kick off simulation */
    NS_LOG_INFO ("Run Simulation.");
    Simulator::Stop (Seconds (m_simTime));
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
}


/*
 * Realize time-varying available bandwidth by
 * introducing background time-varying UDP background
 * traffic, as specified in Section 5.1 of the
 * rmcat-eval-test draft:
 *
 *     When using background non-adaptive UDP traffic to induce
 *     time-varying bottleneck , the physical path capacity
 *     remains at 4Mbps and the UDP traffic source rate changes
 *     over time as (4-x)Mbps, where x is the bottleneck capacity
 *     specified in Table 1.
 */
void RmcatWiredTestCase::SetUpPath (const std::vector<uint32_t>& times,
                                    const std::vector<uint64_t>& capacities,
                                    bool fwd)
{
    const uint16_t basePort = RMCAT_TC_CBR_UDP_PORT + (fwd ? 0:1000);

    if (capacities.size () > 0) {
        NS_ASSERT (capacities.size () == times.size ());
        NS_ASSERT (m_capacity >= *std::max_element (capacities.begin (), capacities.end ()));
        NS_ASSERT (times[0] == 0);

        uint32_t pktsize = RMCAT_TC_UDP_PKTSIZE;
        for (size_t i = 0; i < times.size (); ++i) {
            const uint32_t current_rate = m_capacity - capacities[i];

            if (current_rate > 0) {
                const uint32_t startTime = times[i];
                const uint32_t endTime = (i < times.size () - 1) ? times[i + 1] : m_simTime;

                // install a new CBR-over-UDP flow at current_rate
                ApplicationContainer cbrApps = m_topo.InstallCBR (basePort + i, // port number
                                                                  current_rate, // rate
                                                                  pktsize,      // packet size
                                                                  fwd);         // direction indicator
                cbrApps.Get (0)->SetStartTime (Seconds (startTime));
                cbrApps.Get (0)->SetStopTime (Seconds (endTime));
            }
        }
    }
}

/*
 *  Instantiate RMCAT flows
 */
void RmcatWiredTestCase::SetUpRMCAT (std::vector<Ptr<RmcatSender> >& send,
                                     std::vector<std::shared_ptr<Timer> >& ptimers,
                                     std::vector<std::shared_ptr<Timer> >& rtimers,
                                     bool fwd)
{
    const uint32_t basePort = RMCAT_TC_RMCAT_PORT + (fwd ? 0: 1000);
    size_t numFlows = fwd ? m_numFlowsFw : m_numFlowsBw;
    uint32_t pDelayMs = 0;

    // configure flowID as string
    std::stringstream ss0;
    ss0 << "rmcat_";
    switch (m_codecType) {
        case SYNCODEC_TYPE_PERFECT:
            ss0 << "cbr_";
            break;
        case SYNCODEC_TYPE_FIXFPS:
            ss0 << "fixfps_";
            break;
        case SYNCODEC_TYPE_STATS:
            ss0 << "stats_";
            break;
        case SYNCODEC_TYPE_TRACE:
            ss0 << "tr_";
            break;
        case SYNCODEC_TYPE_SHARING:
            ss0 << "cs_";
            break;
        case SYNCODEC_TYPE_HYBRID:
            ss0 << "hybrid_";
            break;
        default:
            ss0 << "other_";
    }

    if (fwd) {
        ss0 << "fwd_";
    } else {
        ss0 << "bwd_";
    }

    for (size_t i = 0; i < numFlows; ++i) {
        // configure per-flow RTT
        if (fwd && m_pDelays.size () > 0) {
            pDelayMs = m_pDelays[i];
        }

        std::stringstream ss;
        ss << ss0.str () << i;

        ApplicationContainer rmcatApps = m_topo.InstallRMCAT (ss.str (),          // Flow ID
                                                              basePort + (i * 2), // port number
                                                              pDelayMs,           // path RTT
                                                              fwd);               // direction indicator

        send[i] = DynamicCast<RmcatSender> (rmcatApps.Get (0));
        send[i]->SetCodecType (m_codecType);
        send[i]->SetRinit (RMCAT_TC_RINIT);
        send[i]->SetRmin (RMCAT_TC_RMIN);
        send[i]->SetRmax (RMCAT_TC_RMAX);
        send[i]->SetStartTime (Seconds (0));
        send[i]->SetStopTime (Seconds (m_simTime-1));
    }

    /* configure start/end times for forward flows */
    if (fwd && m_startTimesFw.size () > 0) {
        NS_ASSERT (m_startTimesFw.size () == numFlows);
        NS_ASSERT (m_startTimesFw.size () == numFlows);
        for (uint32_t i = 0; i < numFlows; i++) {
            send[i]->SetStartTime (Seconds (m_startTimesFw[i]));
            send[i]->SetStopTime (Seconds (m_endTimesFw[i]));
        }
    }

    /* configure start/end times for backward flows */
    if (!fwd && m_startTimesBw.size () > 0) {
        NS_ASSERT (m_startTimesBw.size () == numFlows);
        NS_ASSERT (m_startTimesBw.size () == numFlows);
        for (uint32_t i = 0; i < numFlows; i++) {
            send[i]->SetStartTime (Seconds (m_startTimesBw[i]));
            send[i]->SetStopTime (Seconds (m_endTimesBw[i]));
        }
    }

    /* configure media pause/resume times for given flow */
    if (fwd && m_pauseTimes.size () > 0
            && m_resumeTimes.size () > 0) {

        size_t fid = m_pauseFid;
        NS_ASSERT (fid < m_numFlowsFw);
        size_t ntimes = m_pauseTimes.size ();
        ptimers.resize (ntimes);
        rtimers.resize (ntimes);
        // configure pause times
        for (size_t i = 0; i < ntimes; ++i) {
            Timer *ptimer = new Timer{Timer::CANCEL_ON_DESTROY};
            Timer *rtimer = new Timer{Timer::CANCEL_ON_DESTROY};

            ptimer->SetFunction (&SenderPauseResume);
            rtimer->SetFunction (&SenderPauseResume);

            ptimer->SetArguments (send[fid], true);  // pause
            rtimer->SetArguments (send[fid], false);  // resume

            ptimer->SetDelay (Seconds (m_pauseTimes[i]));
            rtimer->SetDelay (Seconds (m_resumeTimes[i]));

            ptimer->Schedule ();
            rtimer->Schedule ();

            ptimers[i] = std::shared_ptr<Timer>{ptimer};
            rtimers[i] = std::shared_ptr<Timer>{rtimer};
        }
    }
}

/*
 * Instantiate long lived background TCP flows
 * (only forward direction is supported for now)
 */
void RmcatWiredTestCase::SetUpTCPLong (size_t numFlows,
                                       std::vector<Ptr<BulkSendApplication> >& tcpSend)
{
    const uint16_t basePort = RMCAT_TC_LONG_TCP_PORT;
    for (size_t i = 0; i < numFlows; ++i) {
        std::stringstream ss;
        ss << "tcp_" << i;

        ApplicationContainer tcpApps = m_topo.InstallTCP (ss.str (),          // flow description
                                                          basePort + (i * 2), // port number
                                                          true);              // create a new node for this flow

        tcpSend[i] = DynamicCast<BulkSendApplication> (tcpApps.Get (0));
        tcpSend[i]->SetStartTime (Seconds (RMCAT_TC_BG_TSTART));
        tcpSend[i]->SetStopTime (Seconds (RMCAT_TC_BG_TFINIS));
    }

    // configure start/end time
    if (m_startTimesTcp.size () > 0) {
        for (size_t i = 0; i < numFlows; ++i) {
            tcpSend[i]->SetStartTime (Seconds (m_startTimesTcp[i]));
        }
    }

    if (m_endTimesTcp.size () > 0) {
        for (size_t i = 0; i < numFlows; ++i) {
            tcpSend[i]->SetStopTime (Seconds (m_endTimesTcp[i]));
        }
    }
}

/*
 * Instantiate short-lived background TCP flows
 *
 * The expected behavior of short TCP flows are specified
 * in Section 6.1 of the following IETF draft (rmcat-eval-criteria):
 *
 * Evaluating Congestion Control for Interactive Real-time Media
 * https://tools.ietf.org/html/draft-ietf-rmcat-eval-criteria-06
 *
 *     Each short TCP flow is modeled as a sequence of file downloads
 *     interleaved with idle periods.  Not all short TCPs start at the
 *     same time, i.e., some start in the ON state while others start
 *     in the OFF state.
 *
 *     The short TCP flows can be modeled as follows: 30 connections
 *     start simultaneously fetching small (30-50 KB) amounts of data.
 *     This covers the case where the short TCP flows are not fetching
 *     a video file.
 *
 *     The idle period between bursts of starting a group of TCP flows
 *     is typically derived from an exponential distribution with the
 *     mean value of 10 seconds.
 *
 *     [These values were picked based on the data available at
 *      http://httparchive.org/interesting.php as of October 2015].
 *
 * The timeline of the short TCP flows are specified in
 * Section 5.6 of the rmcat-eval-test draft
 *
 *     +  Traffic timeline: each short TCP flow is modeled as a
 *        sequence of file downloads interleaved with idle periods.
 *        Not all short TCP flows start at the same time, 2 of them
 *        start in the ON state while rest of the 8 flows start in an
 *        OFF stats.
 *
 * Currently only forward direction is supported
 *
 * @param [in] numFlows:  number of short TCP background flows
 * @param [in] numInitOnFlows: number of short TCP flows that are initially in ON
 *                             state at start time
 * @param [in, out] tcpSend:   pointers to the instantiated TCP senders
*/
void RmcatWiredTestCase::SetUpTCPShort (size_t numFlows,
                                        size_t numInitOnFlows,
                                        std::vector<Ptr<BulkSendApplication> >& tcpSend)
{
    uint16_t port = RMCAT_TC_SHORT_TCP_PORT;
    int startTime = RMCAT_TC_SHORT_TCP_TGAP;
    int endTime = m_simTime - RMCAT_TC_SHORT_TCP_TGAP;

    /*
     * Default values of the following parameters follow
     *  Section 6.1 of the rmcat-eval-criteria draft
     */
    uint32_t meanOffDuration = RMCAT_TC_SHORT_TCP_MEAN_OFF_TIME;
    uint32_t minFileSize = RMCAT_TC_SHORT_TCP_MIN_FILE_SIZE;
    uint32_t maxFileSize = RMCAT_TC_SHORT_TCP_MAX_FILE_SIZE;

    // Draw random values of ON/OFF duration
    Ptr<ExponentialRandomVariable> offDurationRnd = CreateObject<ExponentialRandomVariable> ();
    offDurationRnd->SetAttribute ("Mean", DoubleValue (meanOffDuration));

    // Draw random values of transfer file sizes
    // uniformly distributed between minFileSize and maxFileSize
    Ptr<UniformRandomVariable> transferSizeRnd = CreateObject<UniformRandomVariable> ();
    transferSizeRnd->SetAttribute ("Min", DoubleValue (minFileSize));
    transferSizeRnd->SetAttribute ("Max", DoubleValue (maxFileSize));

    for (size_t i = 0; i < numFlows; ++i) {
        bool first = true;   // create a new node for the first file transfer session

        double initTime = ((numInitOnFlows < i) ? startTime : startTime + offDurationRnd->GetValue ());
        for (double lastTime = initTime;
                    lastTime < endTime;
                    lastTime += offDurationRnd->GetValue ()) {

            std::stringstream ss;
            ss << "tcp_" << i;
            ApplicationContainer tcpApps = m_topo.InstallTCP (ss.str (), // flow description
                                                              port++,    // port number
                                                              first);    // whether to create a new node

            Ptr<BulkSendApplication> app = DynamicCast<BulkSendApplication> (tcpApps.Get (0));
            app->SetStartTime (Seconds (lastTime));
            app->SetStopTime  (Seconds (m_simTime - 1));
            app->SetAttribute ("MaxBytes", UintegerValue (transferSizeRnd->GetInteger ()));
            app->SetAttribute ("SendSize", UintegerValue (RMCAT_TC_TCP_PKTSIZE));

            tcpSend.push_back (app);
            first = false;
        }
    }
}

