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
 * Receiver application interface for rmcat ns3 module.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#ifndef RMCAT_RECEIVER_H
#define RMCAT_RECEIVER_H

#include "rtp-header.h"
#include "ns3/socket.h"
#include "ns3/application.h"

namespace ns3 {

class RmcatReceiver: public Application
{
public:
    RmcatReceiver ();
    virtual ~RmcatReceiver ();

    void Setup (uint16_t port);

private:
    virtual void StartApplication ();
    virtual void StopApplication ();

    void RecvPacket (Ptr<Socket> socket);
    void AddFeedback (uint16_t sequence,
                      uint64_t recvTimestampUs);
    void SendFeedback (bool reschedule);

private:
    bool m_running;
    bool m_waiting;
    uint32_t m_ssrc;
    uint32_t m_remoteSsrc;
    Ipv4Address m_srcIp;
    uint16_t m_srcPort;
    Ptr<Socket> m_socket;
    CCFeedbackHeader m_header;
    EventId m_sendEvent;
    uint64_t m_periodUs;
};

}

#endif /* RMCAT_RECEIVER_H */
