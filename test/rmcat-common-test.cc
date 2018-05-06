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
 * Base class implementation for testing rmcat ns3 module.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#include "rmcat-common-test.h"
#include "ns3/log.h"

using namespace ns3;

/* Base class of RMCAT test cases: constructor */
RmcatTestCase::RmcatTestCase (uint64_t capacity,
                              uint32_t delay,
                              uint32_t qdelay,
                              std::string desc)
: TestCase{desc}
, m_debug{false}
, m_sb{NULL}
, m_capacity{capacity}   // bottleneck capacity
, m_delay{delay}         // one-way propagation delay
, m_qdelay{qdelay}       // bottleneck queue depth
{

    // name of log file same as test case descriptions
    std::stringstream ss;
    ss << desc << ".log";
    m_logfile = ss.str ();
}

void RmcatTestCase::DoSetup ()
{
    // configure logging level
    LogLevel l = (LogLevel)(LOG_LEVEL_INFO |
                            LOG_PREFIX_TIME |
                            LOG_PREFIX_NODE);
    LogComponentEnable ("Topo", l);

    if (m_debug) {
        LogComponentEnable ("OnOffApplication", l);
        LogComponentEnable ("UdpClient", l);
        LogComponentEnable ("BulkSendApplication", l);
        LogComponentEnable ("V4Ping", l);
        LogComponentEnable ("RmcatSender", l);
        LogComponentEnable ("RmcatReceiver", l);
    }

    // open output file stream and corresponding streaming buffer
    // for logging
    m_ofs.open (m_logfile.c_str (), std::ios_base::out);
    m_sb = std::clog.rdbuf (m_ofs.rdbuf ());
}

void RmcatTestCase::DoTeardown ()
{
    // close up output file stream
    std::clog.rdbuf (m_sb);
    m_ofs.close ();
}
