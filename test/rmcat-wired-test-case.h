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
 * Class declaration for rmcat wired test cases.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#ifndef RMCAT_WIRED_TEST_CASE_H
#define RMCAT_WIRED_TEST_CASE_H


#include "ns3/wired-topo.h"
#include "ns3/rmcat-sender.h"
#include "ns3/rmcat-receiver.h"
#include "ns3/rmcat-constants.h"
#include "ns3/bulk-send-application.h"
#include "ns3/application-container.h"
#include "ns3/log.h"
#include "ns3/timer.h"
#include "rmcat-common-test.h"
#include <fstream>

using namespace ns3;

/**
 * Defines common configuration parameters of a RMCAT
 * wired test case;
 *
 * Implements common procedures such as configurations
 * of network topology, traffic flow types (RMCAT/TCP),
 * and arrival/departure patterns.
 */
class RmcatWiredTestCase : public RmcatTestCase
{

public:
    /* Constructor */
    RmcatWiredTestCase (uint64_t capacity,  // bottleneck capacity (in bps)
                        uint32_t delay,     // one-way propagation delay (in ms)
                        uint32_t qdelay,    // bottleneck queue depth (in ms)
                        std::string desc);  // test case description/name

    virtual void DoSetup ();
    virtual void DoRun ();

    /*
     * Configure various parameters of the
     * test case by passing along function
     * input values to member variables
     */
    void SetCapacity (uint64_t capacity) {m_capacity = capacity; };
    void SetSimTime (uint32_t simTime) {m_simTime = simTime; };
    void SetCodec (SyncodecType codecType) { m_codecType = codecType; };
    void SetPropDelays (const std::vector<uint32_t>& pDelays) { m_pDelays = pDelays; } ;

    /* configure time-varying BW */
    void SetBW (const std::vector<uint32_t>& times,
                const std::vector<uint64_t>& capacities,
                bool fwd);

    /* configure pause time of a given flow */
    void SetPauseResumeTimes (size_t fid,
                              const std::vector<uint32_t> & ptimes,
                              const std::vector<uint32_t> & rtimes,
                              bool fwd);

    /* configure RMCAT flows and their arrival/departure pattern */
    void SetRMCATFlows (size_t numFlows,
                        const std::vector<uint32_t>& startTimes,
                        const std::vector<uint32_t>& endTimes,
                        bool fwd);

    /* configure long lived TCP background flows
     * and their arrival/departure patterns */
    void SetTCPLongFlows (size_t numFlows,
                          const std::vector<uint32_t>& startTimes,
                          const std::vector<uint32_t>& endTimes,
                          bool fwd);

    /* configure short-lived TCP background flows */
    void SetTCPShortFlows (size_t numFlows,
                           size_t numInitOnFlows,
                           bool fwd);

protected:
    /* Instantiate flows in DoRun () */
    void SetUpPath (const std::vector<uint32_t>& timesFw,
                    const std::vector<uint64_t>& capacities,
                    bool  fwd);

    void SetUpRMCAT (std::vector< Ptr<RmcatSender> >& send,
                     std::vector<std::shared_ptr<Timer> >& ptimers,
                     std::vector<std::shared_ptr<Timer> >& rtimers,
                     bool fwd);

    void SetUpTCPLong (size_t numFlows,
                       std::vector<Ptr<BulkSendApplication> >& tcpSend);

    void SetUpTCPShort (size_t numFlows,
                        size_t numInitOnFlows,
                        std::vector<Ptr<BulkSendApplication> >& tcpSend);

    /* network toplogy configuration */
    WiredTopo m_topo;

private:
    /* Member variables specifying test case configuration */
    size_t m_numFlowsFw;        // # of RMCAT flows on forward path
    size_t m_numFlowsBw;        // # of RMCAT flows on backward path
    size_t m_numTcpFlows;       // # of long lived TCP flows, only on forward path
    size_t m_numShortTcpFlows;  // # of short lived TCP flows, only on forward path
    size_t m_numInitOnFlows;    // # of short lived TCP flows initially in ON state
    uint32_t m_simTime;         // simulation duration (in seconds)

    /* time-varying capacities */
    std::vector<uint64_t> m_capacitiesFw;
    std::vector<uint64_t> m_capacitiesBw;
    std::vector<uint32_t> m_timesFw;
    std::vector<uint32_t> m_timesBw;

    /* per-flow one-way propagation delay (in ms) */
    std::vector<uint32_t> m_pDelays;

    /* start/end times for each RMCAT flow */
    std::vector<uint32_t> m_startTimesFw;
    std::vector<uint32_t> m_endTimesFw;
    std::vector<uint32_t> m_startTimesBw;
    std::vector<uint32_t> m_endTimesBw;

    /* start/end times for long lived TCP flows
     * (currently only supporting forward direction)
     */
    std::vector<uint32_t> m_startTimesTcp;
    std::vector<uint32_t> m_endTimesTcp;

    /* pause/resume times of a given RMCAT flow
     * (currently only supporting forward direction)
     */
    size_t m_pauseFid;
    std::vector<uint32_t> m_pauseTimes;
    std::vector<uint32_t> m_resumeTimes;

    SyncodecType m_codecType;
};

#endif /* RMCAT_WIRED_TEST_CASE_H */
