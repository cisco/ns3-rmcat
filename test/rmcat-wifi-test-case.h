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
 * Template for rmcat wifi test cases.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

/*
 * This file contains definition of the rmcatWifiTestCase
 * class. It defines the template for individual
 * rmcat-wifi test cases as specified in
 * in Section 4 of the following IETF draft:
 *
 * Evaluation Test Cases for Interactive Real-Time
 * Media over Wireless Networks
 * https://tools.ietf.org/html/draft-ietf-rmcat-wireless-tests-04
 *
 */


#ifndef RMCAT_WIFI_TEST_CASE_H
#define RMCAT_WIFI_TEST_CASE_H

#include "ns3/wifi-topo.h"
#include "ns3/rmcat-sender.h"
#include "ns3/rmcat-receiver.h"
#include "ns3/rmcat-constants.h"
#include "rmcat-common-test.h"

using namespace ns3;

/*
 * Defines common configuration parameters of a
 * RMCAT wifi test case.
 *
 * Defines common procedures in setting up a
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

class RmcatWifiTestCase : public RmcatTestCase
{
public:
    /* Constructor */
    RmcatWifiTestCase (uint64_t capacity,  // wired link capacity (in bps)
                       uint32_t pdelay,    // propagation delay (in ms)
                       uint32_t qdelay,    // wired bottleneck queue depth (in ms)
                       std::string desc);  // test case description/name

    virtual void DoSetup ();
    virtual void DoRun ();

    /*
     * Configure various parameters of
     * the test case by passing along function
     * input values to member variables
     */
    void SetCapacity (uint64_t capacity) {m_capacity = capacity; };
    void SetSimTime (uint32_t simTime)  {m_simTime = simTime; };
    void SetCodec (SyncodecType codecType) {m_codecType = codecType; };
    void SetPHYMode (ns3::WifiMode phyMode) {m_phyMode = phyMode; };
    void SetCBRRate (uint64_t rCBR) { m_rCBR = rCBR; };

    /* configure RMCAT flows and
     * their arrival/departure patterns
     */
    void SetRMCATFlows (size_t numFlows,
                        const std::vector<uint32_t>& startTimes,
                        const std::vector<uint32_t>& endTimes,
                        bool fwd);

    /* configure long lived TCP background flows and
     * their arrival/departure patterns
     */
    void SetTCPFlows (size_t numFlows,
                      const std::vector<uint32_t>& startTimes,
                      const std::vector<uint32_t>& endTimes,
                      bool fwd);

    /* configure CBR over UDP background flows
     * and their arrival/departure patterns
     */
    void SetUDPFlows (size_t numFlows,
                      const std::vector<uint32_t>& startTimes,
                      const std::vector<uint32_t>& endTimes,
                      bool fwd);

    /* Instantiate flows in DoRun () */
    void SetUpRMCAT (std::vector< Ptr<RmcatSender> >& send,
                     bool fwd);

    void SetUpTCP (std::vector<Ptr<BulkSendApplication> >& tcpSend,
                   bool fwd);

    void SetUpCBR (std::vector<Ptr<Application> >& cbrSend,
                   bool fwd);

protected:
    /* network toplogy configuration */
    WifiTopo m_topo;

private:
    /* Member variables specifying test case configuration */
    uint32_t m_nWifi;    // # of wireless nodes in test topology
    uint32_t m_nDnRMCAT; // # of downlink RMCAT flows
    uint32_t m_nUpRMCAT; // # of uplink RMCAT flows
    uint32_t m_nDnTCP;   // # of downlink long lived TCP background flows
    uint32_t m_nUpTCP;   // # of uplink long lived TCP background flows
    uint32_t m_nDnCBR;   // # of downlink CBR-over-UDP background flows
    uint32_t m_nUpCBR;   // # of uplink CBR-over-UDP background flows
    uint64_t m_rCBR;     // rate of each CBR background flow (in bps)

    uint32_t m_simTime;  // simulation duration (in seconds)

    // start/end times for each RMCAT flow
    std::vector<uint32_t> m_startTDnRMCAT;
    std::vector<uint32_t> m_finisTDnRMCAT;
    std::vector<uint32_t> m_startTUpRMCAT;
    std::vector<uint32_t> m_finisTUpRMCAT;

    // start/end times for each long lived TCP flow
    std::vector<uint32_t> m_startTDnTCP;
    std::vector<uint32_t> m_finisTDnTCP;
    std::vector<uint32_t> m_startTUpTCP;
    std::vector<uint32_t> m_finisTUpTCP;

    // start/end times for each CBR over UDP flow
    std::vector<uint32_t> m_startTDnCBR;
    std::vector<uint32_t> m_finisTDnCBR;
    std::vector<uint32_t> m_startTUpCBR;
    std::vector<uint32_t> m_finisTUpCBR;

    SyncodecType m_codecType; // traffic source type
    ns3::WifiMode m_phyMode;  // PHY mode for wireless connections
};

#endif /* RMCAT_WIFI_TEST_CASE_H */
