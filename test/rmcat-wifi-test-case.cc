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
 * Common template for rmcat wifi test cases.
 *
 * This file contains implementation of the
 * RmcatWifiTestCase class. It defines the
 * common template for individual rmcat-wifi
 * test cases as specified in Section 4 of the
 * following IETF draft:
 *
 * Evaluation Test Cases for Interactive Real-Time
 * Media over Wireless Networks
 * https://tools.ietf.org/html/draft-ietf-rmcat-wireless-tests-04
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#include "rmcat-wifi-test-case.h"

NS_LOG_COMPONENT_DEFINE ("RmcatSimTestWifi");

/*
 * Implement common procedures in setting up a
 * rmcat-wifi test case:
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

/* Constructor */
RmcatWifiTestCase::RmcatWifiTestCase (uint64_t capacity, // wired link capacity (in bps)
                                      uint32_t pdelay,   // wired link forward propagation delay (in ms)
                                      uint32_t qdelay,   // wired link queue depth (in ms)
                                      std::string desc)  // test case name/description
: RmcatTestCase{capacity, pdelay, qdelay, desc}
, m_nWifi{2}  // default: a pair of bi-directional RMCAT flows
, m_nDnRMCAT{1}
, m_nUpRMCAT{1}
, m_nDnTCP{0}
, m_nUpTCP{0}
, m_nDnCBR{0}
, m_nUpCBR{0}
, m_rCBR{0}
, m_simTime{RMCAT_TC_SIMTIME}
, m_codecType{SYNCODEC_TYPE_FIXFPS}
, m_phyMode{WifiMode ("HtMcs11")}
{}


/**
 * Configure RMCAT flows in terms of:
 * -- direction (downlink/uplink)
 * -- arrival/departure times
 */
void RmcatWifiTestCase::SetRMCATFlows (size_t numFlows,
                                       const std::vector<uint32_t>& startTimes,
                                       const std::vector<uint32_t>& endTimes,
                                       bool fwd)
{
    if (fwd) {
        // configure downlink flows
        m_nDnRMCAT   = numFlows;
        m_startTDnRMCAT = startTimes;
        m_finisTDnRMCAT = endTimes;
    } else {
        // configure uplink flows
        m_nUpRMCAT = numFlows;
        m_startTUpRMCAT  = startTimes;
        m_finisTUpRMCAT = endTimes;
    }
}

/**
 * Configure long lived TCP background flows
 * in terms of:
 * -- direction (downlink/uplink)
 * -- arrival/departure times
 */
void RmcatWifiTestCase::SetTCPFlows (size_t numFlows,
                                     const std::vector<uint32_t>& startTimes,
                                     const std::vector<uint32_t>& endTimes,
                                     bool fwd)
{
    if (fwd) {
        // configure downlink flows
        m_nDnTCP   = numFlows;
        m_startTDnTCP = startTimes;
        m_finisTDnTCP = endTimes;
    } else {
        // configure uplink flows
        m_nUpTCP = numFlows;
        m_startTUpTCP = startTimes;
        m_finisTUpTCP = endTimes;
    }
}

/*
 * Configure CBR-over-UDP background flows
 * in terms of:
 * -- direction (downlink/uplink)
 * -- arrival/departure times
 */
void RmcatWifiTestCase::SetUDPFlows (size_t numFlows,
                                     const std::vector<uint32_t>& startTimes,
                                     const std::vector<uint32_t>& endTimes,
                                     bool fwd)
{
    if (fwd) {
        // configure downlink flows
        m_nDnCBR   = numFlows;
        m_startTDnCBR = startTimes;
        m_finisTDnCBR = endTimes;
    } else {
        // configure uplink flows
        m_nUpCBR = numFlows;
        m_startTUpCBR = startTimes;
        m_finisTUpCBR = endTimes;
    }
}


/**
 * Instantiate RMCAT flows
 */
void RmcatWifiTestCase::SetUpRMCAT (std::vector<Ptr<RmcatSender> >& send,
                                    bool fwd)
{
    size_t numFlows = fwd ? m_nDnRMCAT : m_nUpRMCAT;
    const uint32_t basePort = RMCAT_TC_RMCAT_PORT + (fwd ? 0: 1000);
    const auto nRmcatBase = (fwd ? 0 : m_nDnRMCAT);
    std::stringstream ss0;
    if (fwd) {
        ss0 << "rmcatDn_";
    } else {
        ss0 << "rmcatUp_";
    }

    for (uint32_t i = 0; i < numFlows; i++) {
        std::stringstream ss;
        ss << ss0.str () << i;

        ApplicationContainer rmcatApps = m_topo.InstallRMCAT (ss.str (),        // flowID
                                                              nRmcatBase + i,   // nodeID
                                                              basePort + i * 2, // port # ,
                                                              fwd);             // direction

        send[i] = DynamicCast<RmcatSender> (rmcatApps.Get (0));
        send[i]->SetCodecType (m_codecType);
        send[i]->SetRinit (RMCAT_TC_RINIT);
        send[i]->SetRmin (RMCAT_TC_RMIN);
        send[i]->SetRmax (RMCAT_TC_RMAX);
        send[i]->SetStartTime (Seconds (0));
        send[i]->SetStopTime (Seconds (m_simTime-1));
    }

    // configure start/end times for downlink flows
    if (fwd && m_startTDnRMCAT.size () > 0) {
        NS_ASSERT (m_startTDnRMCAT.size () == numFlows);
        NS_ASSERT (m_finisTDnRMCAT.size () == numFlows);
        for (uint32_t i = 0; i < numFlows; i++) {
            send[i]->SetStartTime (Seconds (m_startTDnRMCAT[i]));
            send[i]->SetStopTime (Seconds (m_finisTDnRMCAT[i]));
        }
    }

    // configure start/end times for uplink flows
    if (!fwd && m_startTUpRMCAT.size () > 0) {
        NS_ASSERT (m_startTUpRMCAT.size () == numFlows);
        NS_ASSERT (m_finisTUpRMCAT.size () == numFlows);
        for (uint32_t i = 0; i < numFlows; i++) {
            send[i]->SetStartTime (Seconds (m_startTUpRMCAT[i]));
            send[i]->SetStopTime (Seconds (m_finisTUpRMCAT[i]));
        }
    }
}

/** Instantiate long lived TCP background flows */
void RmcatWifiTestCase::SetUpTCP (std::vector<Ptr<BulkSendApplication> >& tcpSend,
                                  bool fwd)
{
    size_t numFlows = fwd ? m_nDnTCP : m_nUpTCP;
    const uint32_t basePort = RMCAT_TC_LONG_TCP_PORT + (fwd ? 0: 1000);
    const auto nTcpBase = m_nDnRMCAT+m_nUpRMCAT+ (fwd? 0:m_nDnTCP);

    std::stringstream ss0;
    if (fwd) {
        ss0 << "tcpDn_";
    } else {
        ss0 << "tcpUp_";
    }

    std::vector<Ptr<Application> > tcpRecv (numFlows);

    for (uint32_t i = 0; i < numFlows; i++) {
        std::stringstream ss;
        ss << ss0.str () << i;

        auto tcpApps = m_topo.InstallTCP (ss.str (),        // flow description
                                          nTcpBase+i,       // node ID for server/client
                                          basePort + 2 * i, // server port
                                          fwd);

        tcpSend[i] = DynamicCast<BulkSendApplication> (tcpApps.Get (0));
        tcpSend[i]->SetStartTime (Seconds (RMCAT_TC_BG_TSTART));
        tcpSend[i]->SetStopTime (Seconds (RMCAT_TC_BG_TFINIS));
        tcpRecv[i] = DynamicCast<Application> (tcpApps.Get (1));
        tcpRecv[i]->SetStartTime (Seconds (RMCAT_TC_BG_TSTART));
        tcpRecv[i]->SetStopTime (Seconds (RMCAT_TC_BG_TFINIS));
    }

    // configure start/end times for downlink flows
    if (fwd && m_startTDnTCP.size () > 0) {
        NS_ASSERT (m_startTDnTCP.size () == numFlows);
        NS_ASSERT (m_finisTDnTCP.size () == numFlows);
        for (uint32_t i = 0; i < numFlows; i++) {
            tcpSend[i]->SetStartTime (Seconds (m_startTDnTCP[i]));
            tcpSend[i]->SetStopTime (Seconds (m_finisTDnTCP[i]));
            tcpRecv[i]->SetStartTime (Seconds (m_startTDnTCP[i]));
            tcpRecv[i]->SetStopTime (Seconds (m_finisTDnTCP[i]));
        }
    }

    // configure start/end times for uplink flows
    if (!fwd && m_startTUpTCP.size () > 0) {
        NS_ASSERT (m_startTUpTCP.size () == numFlows);
        NS_ASSERT (m_finisTUpTCP.size () == numFlows);
        for (uint32_t i = 0; i < numFlows; i++) {
            tcpSend[i]->SetStartTime (Seconds (m_startTUpTCP[i]));
            tcpSend[i]->SetStopTime (Seconds (m_finisTUpTCP[i]));
            tcpRecv[i]->SetStartTime (Seconds (m_startTUpTCP[i]));
            tcpRecv[i]->SetStopTime (Seconds (m_finisTUpTCP[i]));
        }
    }
}

/** Instantiate CBR-over-UDP background flows */
void RmcatWifiTestCase::SetUpCBR (std::vector<Ptr<Application> >& cbrSend,
                                  bool fwd)
{

    size_t numFlows = fwd ? m_nDnCBR : m_nUpCBR;
    const uint32_t basePort = RMCAT_TC_CBR_UDP_PORT + (fwd ? 0: 1000);
    const uint32_t nCbrBase = m_nDnRMCAT+m_nUpRMCAT+m_nDnTCP+m_nUpTCP + (fwd? 0:m_nDnCBR);
    for (uint32_t i = 0; i < numFlows; i++) {
        ApplicationContainer cbrApps = m_topo.InstallCBR (nCbrBase+i,   // node ID
                                                          basePort+i,   // port #,
                                                          m_rCBR,       // rate of CBR
                                                          RMCAT_TC_UDP_PKTSIZE,
                                                          //   RMCAT_TC_BG_TSTART,
                                                          //   RMCAT_TC_BG_TFINIS,
                                                          fwd);

        cbrSend[i] = cbrApps.Get (0);
        cbrSend[i]->SetStartTime (Seconds (RMCAT_TC_BG_TSTART));
        cbrSend[i]->SetStopTime (Seconds (RMCAT_TC_BG_TFINIS));
    }

    // configure start/end times for downlink flows
    if (fwd && m_startTDnCBR.size () > 0) {
        NS_ASSERT (m_startTDnCBR.size () == numFlows);
        NS_ASSERT (m_finisTDnCBR.size () == numFlows);
        for (uint32_t i = 0; i < numFlows; i++) {
            cbrSend[i]->SetStartTime (Seconds (m_startTDnCBR[i]));
            cbrSend[i]->SetStopTime (Seconds (m_finisTDnCBR[i]));
        }
    }

    // configure start/end times for uplink flows
    if (!fwd && m_startTUpCBR.size () > 0) {
        NS_ASSERT (m_startTUpCBR.size () == numFlows);
        NS_ASSERT (m_finisTUpCBR.size () == numFlows);
        for (uint32_t i = 0; i < numFlows; i++) {
            cbrSend[i]->SetStartTime (Seconds (m_startTUpCBR[i]));
            cbrSend[i]->SetStopTime (Seconds (m_finisTUpCBR[i]));
        }
    }
}

/**
 * Inherited DoSetup function:
 * -- Build network topology
 * -- Enable additional logging
 */
void RmcatWifiTestCase::DoSetup ()
{
    RmcatTestCase::DoSetup ();
    m_nWifi  = m_nDnRMCAT + m_nUpRMCAT;
    m_nWifi += m_nDnTCP + m_nUpTCP;
    m_nWifi += m_nDnCBR + m_nUpCBR;

    m_topo.Build (m_capacity,
                  m_delay,
                  m_qdelay,
                  m_nWifi,
                  WIFI_PHY_STANDARD_80211n_5GHZ,
                  m_phyMode);

    ns3::LogComponentEnable ("RmcatSimTestWifi", LOG_LEVEL_INFO);
}

/**
 * Inherited DoRun () function:
 * -- Instantiate RMCAT/TCP/UDP flows
 * -- Populate routing table
 * -- Kickoff simulation
 */
void RmcatWifiTestCase::DoRun ()
{
    /* Configure downlink/uplink flows */
    std::vector< Ptr<RmcatSender> > sendDnRMCAT (m_nDnRMCAT);
    std::vector< Ptr<RmcatSender> > sendUpRMCAT (m_nUpRMCAT);
    std::vector< Ptr<BulkSendApplication> > sendDnTCP (m_nDnTCP);
    std::vector< Ptr<BulkSendApplication> > sendUpTCP (m_nUpTCP);
    std::vector< Ptr<Application> > sendDnCBR (m_nDnCBR);
    std::vector< Ptr<Application> > sendUpCBR (m_nUpCBR);

    SetUpRMCAT (sendDnRMCAT, true);
    SetUpRMCAT (sendUpRMCAT, false);

    SetUpTCP (sendDnTCP, true);
    SetUpTCP (sendUpTCP, false);

    SetUpCBR (sendDnCBR, true);
    SetUpCBR (sendUpCBR, false);

    /* Kick off simulation */
    NS_LOG_INFO ("Run Simulation.");
    Simulator::Stop (Seconds (m_simTime));
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
}
