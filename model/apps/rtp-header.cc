/******************************************************************************
 * Copyright 2016-2018 Cisco Systems, Inc.                                    *
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
 * Header implementation of RTP packets (RFC 3550) for ns3-rmcat.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#include "rtp-header.h"

namespace ns3 {

RtpHeader::RtpHeader ()
: m_padding{false}
, m_extension{false}
, m_marker{false}
, m_payloadType{0}
, m_sequence{0}
, m_timestamp{0}
, m_ssrc{0}
, m_csrc{}
{}

RtpHeader::~RtpHeader () {}

TypeId RtpHeader::GetTypeId ()
{
    static TypeId tid = TypeId ("RtpHeader")
      .SetParent<Header> ()
      .AddConstructor<RtpHeader> ()
    ;
    return tid;
}

TypeId RtpHeader::GetInstanceTypeId () const
{
    return GetTypeId ();
}

uint32_t RtpHeader::GetSerializedSize () const
{
    NS_ASSERT (m_csrc.size () <= 0x0f);
    return 2 + // First two octets
           sizeof (m_sequence)  +
           sizeof (m_timestamp) +
           sizeof (m_ssrc) +
           (m_csrc.size () & 0x0f) * sizeof (decltype (m_csrc)::value_type);
}

void RtpHeader::Serialize (Buffer::Iterator start) const
{
    NS_ASSERT (m_csrc.size () <= 0x0f);
    NS_ASSERT (m_payloadType <= 0x7f);

    const uint8_t csrcCount = (m_csrc.size () & 0x0f);
    uint8_t octet1 = 0;
    octet1 |= (RTP_VERSION << 6);
    SetBit (octet1, 5, m_padding);
    SetBit (octet1, 4, m_extension);
    octet1 |= uint8_t (csrcCount);
    start.WriteU8 (octet1);

    uint8_t octet2 = 0;
    SetBit (octet2, 7, m_marker);
    octet2 |= (m_payloadType & 0x7f);
    start.WriteU8 (octet2);

    start.WriteHtonU16 (m_sequence);
    start.WriteHtonU32 (m_timestamp);
    start.WriteHtonU32 (m_ssrc);
    for (size_t i = 0; i < csrcCount; ++i) {
        start.WriteHtonU32 (m_csrc[i]);
    }
}

uint32_t RtpHeader::Deserialize (Buffer::Iterator start)
{
    const auto octet1 = start.ReadU8 ();
    const uint8_t version = (octet1 >> 6);
    m_padding = GetBit (octet1, 5);
    m_extension = GetBit (octet1, 4);
    const uint8_t csrcCount = (octet1 & 0x0f);

    const auto octet2 = start.ReadU8 ();
    m_marker = GetBit (octet2, 7);
    m_payloadType = (octet2 & 0x7f);

    m_sequence = start.ReadNtohU16 ();
    m_timestamp = start.ReadNtohU32 ();
    m_ssrc = start.ReadNtohU32 ();
    m_csrc.clear ();
    for (size_t i = 0; i < csrcCount; ++i) {
        m_csrc.push_back (start.ReadNtohU32 ());
    }
    NS_ASSERT (version == RTP_VERSION);
    return GetSerializedSize ();
}

void RtpHeader::Print (std::ostream &os) const
{
    NS_ASSERT (m_csrc.size () <= 0x0f);
    os << "RtpHeader - version = " << RTP_VERSION
       << ", padding =" << (m_padding ? "yes" : "no")
       << ", extension = " << (m_extension ? "yes" : "no")
       << ", CSRC count = " << m_csrc.size ()
       << ", marker = " << (m_marker ? "yes" : "no")
       << ", payload type = " << m_payloadType
       << ", sequence = " << m_sequence
       << ", timestamp = " << m_timestamp
       << ", ssrc = " << m_ssrc;
    for (size_t i = 0; i < (m_csrc.size () & 0x0f); ++i) {
        os << ", CSRC#" << i << " = " << m_csrc[i];
    }
    os << std::endl;
}

void RtpHeader::SetBit (uint8_t& val, uint8_t pos, bool bit)
{
    if (bit) {
        val |= (1 << pos);
    } else {
        val &= (~(1u << pos));
    }
}

bool RtpHeader::GetBit (uint8_t val, uint8_t pos)
{
    return bool (val & (1 << pos));
}



FeedbackHeader::~FeedbackHeader () {}

TypeId FeedbackHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("FeedbackHeader")
      .SetParent<Header> ()
      .AddConstructor<FeedbackHeader> ()
    ;
    return tid;
}

TypeId FeedbackHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

uint32_t FeedbackHeader::GetSerializedSize (void) const
{
    return sizeof (flow_id)      +
           sizeof (sequence)     +
           sizeof (receive_tstmp);
}

void FeedbackHeader::Serialize (Buffer::Iterator start) const
{
    start.WriteHtonU32 (flow_id);
    start.WriteHtonU32 (sequence);
    start.WriteHtonU64 (receive_tstmp);
}

uint32_t FeedbackHeader::Deserialize (Buffer::Iterator start)
{
    flow_id = start.ReadNtohU32 ();
    sequence = start.ReadNtohU32 ();
    receive_tstmp = start.ReadNtohU64 ();
    return GetSerializedSize ();
}

void FeedbackHeader::Print (std::ostream &os) const
{
    os << "FeedbackHeader - flow_id = " << flow_id
       << ", sequence = " << sequence
       << ",receive_tstmp = " << receive_tstmp;
}

}
