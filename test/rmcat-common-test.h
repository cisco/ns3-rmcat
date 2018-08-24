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
 * Base classes declaration for testing rmcat ns3 module.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#ifndef RMCAT_COMMON_TEST_H
#define RMCAT_COMMON_TEST_H

#include "ns3/test.h"
#include <fstream>

/* default simulation parameters */
const uint32_t RMCAT_TC_BG_TSTART = 40;
const uint32_t RMCAT_TC_BG_TFINIS = 80;
const uint32_t RMCAT_TC_SIMTIME = 120;  // default simulation duration

#define RMCAT_TC_TCP_RECVBUF_SIZE 524288 * 2

const uint32_t RMCAT_TC_UDP_PKTSIZE = 1000;
const uint32_t RMCAT_TC_TCP_PKTSIZE = 1000;
const uint32_t RMCAT_TC_RINIT = 150 * (1u << 10);  // R_init: 150 Kbps
const uint32_t RMCAT_TC_RMIN = 150 * (1u << 10);   // R_min:  150 Kbps
const uint32_t RMCAT_TC_RMAX = 1500 * (1u << 10);  // R_max: 1500 Kbps

// default port assignment: base numbers
const uint32_t RMCAT_TC_CBR_UDP_PORT   = 4000;
const uint32_t RMCAT_TC_LONG_TCP_PORT  = 6000;
const uint32_t RMCAT_TC_SHORT_TCP_PORT = 7000;
const uint32_t RMCAT_TC_RMCAT_PORT     = 8000;

// TODO (deferred): These two values should be set to 0 to match eval-test-06 draft
const uint32_t RMCAT_TC_SHORT_TCP_TGAP = 10;
const uint32_t RMCAT_TC_SHORT_TCP_MEAN_OFF_TIME = 10;  // mean off duration of each short TCP flow: 10 seconds
const uint32_t RMCAT_TC_SHORT_TCP_MIN_FILE_SIZE = 30 * (1u << 10); // minimum file size: 30KB
const uint32_t RMCAT_TC_SHORT_TCP_MAX_FILE_SIZE = 50 * (1u << 10); // minimum file size: 30KB

/** Base class of RMCAT test cases */
class RmcatTestCase : public ns3::TestCase
{
public:
    /* Constructor */
    RmcatTestCase (uint64_t capacity,   // bottleneck capacity  (in bps)
                   uint32_t delay,      // one-way propagation delay (in ms)
                   uint32_t qdelay,     // bottleneck queue depth (in ms)
                   std::string desc);   // test case name/description

    virtual void DoSetup ();
    virtual void DoTeardown ();

protected:

    bool m_debug;           // debugging mode

    /* Log file of current test case */
    std::string m_logfile;  // name of log file
    std::ofstream m_ofs;    // output file stream
    std::streambuf* m_sb;   // output stream buffer

    uint64_t m_capacity;   // bottleneck capacity (in bps)
    uint32_t m_delay;      // one-way propagation delay (in ms)
    uint32_t m_qdelay;     // bottleneck queue depth (in ms)
};

#endif /* RMCAT_COMMON_TEST_H */
