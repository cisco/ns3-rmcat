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
 * Receiver application implementation for rmcat ns3 module.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#include "rmcat-receiver.h"
#include "rtp-header.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("RmcatReceiver");

namespace ns3 {
RmcatReceiver::RmcatReceiver ()
: m_running{false}
, m_waiting{false}
, m_ssrc{0}
, m_remoteSsrc{0}
, m_srcIp{}
, m_srcPort{}
, m_socket{NULL}
{}

RmcatReceiver::~RmcatReceiver () {}

void RmcatReceiver::Setup (uint16_t port)
{
    m_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    auto local = InetSocketAddress{Ipv4Address::GetAny (), port};
    auto ret = m_socket->Bind (local);
    NS_ASSERT (ret == 0);
    m_socket->SetRecvCallback (MakeCallback (&RmcatReceiver::RecvPacket,this));

    m_running = false;
    m_waiting = true;
}

void RmcatReceiver::StartApplication ()
{
    m_running = true;
    m_ssrc = rand ();
}

void RmcatReceiver::StopApplication ()
{
    m_running = false;
}

void RmcatReceiver::RecvPacket (Ptr<Socket> socket)
{
    if (!m_running) {
        return;
    }

    Address remoteAddr{};
    auto packet = m_socket->RecvFrom (remoteAddr);
    NS_ASSERT (packet);
    RtpHeader header{};
    NS_LOG_INFO ("RmcatReceiver::RecvPacket, " << packet->ToString ());
    packet->RemoveHeader (header);
    auto srcIp = InetSocketAddress::ConvertFrom (remoteAddr).GetIpv4 ();
    auto srcPort = InetSocketAddress::ConvertFrom (remoteAddr).GetPort ();
    if (m_waiting) {
        m_waiting = false;
        m_remoteSsrc = header.GetSsrc ();
        m_srcIp = srcIp;
        m_srcPort = srcPort;
    } else {
        // Only one flow supported
        NS_ASSERT (m_remoteSsrc == header.GetSsrc ());
        NS_ASSERT (m_srcIp == srcIp);
        NS_ASSERT (m_srcPort == srcPort);
    }

    uint64_t recvTimestampUs = Simulator::Now ().GetMicroSeconds ();
    SendFeedback (header.GetSequence (), recvTimestampUs);
}

void RmcatReceiver::SendFeedback (uint16_t sequence,
                                  uint64_t recvTimestampUs)
{
    // TODO (next patch): We need to aggregate feedback information
    //                    (for the moment, one feedback packet per media packet)
    //                     - add member of type feedback header
    //                     - add timeout (100 ms), upon timeout send header
    //                     - if TOO_LONG, send immediately

    CCFeedbackHeader header{};
    header.SetSendSsrc (m_ssrc);
    auto res = header.AddFeedback (m_remoteSsrc, sequence, recvTimestampUs);
    NS_ASSERT (res == CCFeedbackHeader::CCFB_NONE);
    auto packet = Create<Packet> ();
    packet->AddHeader (header);
    NS_LOG_INFO ("RmcatReceiver::SendFeedback, " << packet->ToString ());

    m_socket->SendTo (packet, 0, InetSocketAddress{m_srcIp, m_srcPort});
}

}

