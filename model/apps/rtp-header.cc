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
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#include "rtp-header.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RtpHeader);
NS_OBJECT_ENSURE_REGISTERED (RtcpHeader);
NS_OBJECT_ENSURE_REGISTERED (CCFeedbackHeader);

void RtpHdrSetBit (uint8_t& val, uint8_t pos, bool bit)
{
    NS_ASSERT (pos < 8);
    if (bit) {
        val |= (1u << pos);
    } else {
        val &= (~(1u << pos));
    }
}

bool RtpHdrGetBit (uint8_t val, uint8_t pos)
{
    return bool (val & (1u << pos));
}


RtpHeader::RtpHeader ()
: Header{}
, m_padding{false}
, m_extension{false}
, m_marker{false}
, m_payloadType{0}
, m_sequence{0}
, m_timestamp{0}
, m_ssrc{0}
, m_csrcs{}
{}

RtpHeader::RtpHeader (uint8_t payloadType)
: Header{}
, m_padding{false}
, m_extension{false}
, m_marker{false}
, m_payloadType{payloadType}
, m_sequence{0}
, m_timestamp{0}
, m_ssrc{0}
, m_csrcs{}
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
    NS_ASSERT (m_csrcs.size () <= 0x0f);
    return 2 + // First two octets
           sizeof (m_sequence)  +
           sizeof (m_timestamp) +
           sizeof (m_ssrc) +
           (m_csrcs.size () & 0x0f) * sizeof (decltype (m_csrcs)::value_type);
}

void RtpHeader::Serialize (Buffer::Iterator start) const
{
    NS_ASSERT (m_csrcs.size () <= 0x0f);
    NS_ASSERT (m_payloadType <= 0x7f);

    const uint8_t csrcCount = (m_csrcs.size () & 0x0f);
    uint8_t octet1 = 0;
    octet1 |= (RTP_VERSION << 6);
    RtpHdrSetBit (octet1, 5, m_padding);
    RtpHdrSetBit (octet1, 4, m_extension);
    octet1 |= uint8_t (csrcCount);
    start.WriteU8 (octet1);

    uint8_t octet2 = 0;
    RtpHdrSetBit (octet2, 7, m_marker);
    octet2 |= (m_payloadType & 0x7f);
    start.WriteU8 (octet2);

    start.WriteHtonU16 (m_sequence);
    start.WriteHtonU32 (m_timestamp);
    start.WriteHtonU32 (m_ssrc);
    for (const auto& csrc : m_csrcs) {
        start.WriteHtonU32 (csrc);
    }
}

uint32_t RtpHeader::Deserialize (Buffer::Iterator start)
{
    const auto octet1 = start.ReadU8 ();
    const uint8_t version = (octet1 >> 6);
    m_padding = RtpHdrGetBit (octet1, 5);
    m_extension = RtpHdrGetBit (octet1, 4);
    const uint8_t csrcCount = (octet1 & 0x0f);

    const auto octet2 = start.ReadU8 ();
    m_marker = RtpHdrGetBit (octet2, 7);
    m_payloadType = (octet2 & 0x7f);

    m_sequence = start.ReadNtohU16 ();
    m_timestamp = start.ReadNtohU32 ();
    m_ssrc = start.ReadNtohU32 ();
    m_csrcs.clear ();
    for (auto i = 0; i < csrcCount; ++i) {
        const uint32_t csrc = start.ReadNtohU32 ();
        NS_ASSERT (m_csrcs.count (csrc) == 0);
        m_csrcs.insert (csrc);
    }
    NS_ASSERT (version == RTP_VERSION);
    return GetSerializedSize ();
}

void RtpHeader::Print (std::ostream& os) const
{
    NS_ASSERT (m_csrcs.size () <= 0x0f);
    os << "RtpHeader - version = " << RTP_VERSION
       << ", padding =" << (m_padding ? "yes" : "no")
       << ", extension = " << (m_extension ? "yes" : "no")
       << ", CSRC count = " << m_csrcs.size ()
       << ", marker = " << (m_marker ? "yes" : "no")
       << ", payload type = " << m_payloadType
       << ", sequence = " << m_sequence
       << ", timestamp = " << m_timestamp
       << ", ssrc = " << m_ssrc;
    size_t i = 0;
    for (const auto& csrc : m_csrcs) {
        os << ", CSRC#" << i << " = " << csrc;
        ++i;
    }
    os << std::endl;
}

bool RtpHeader::IsPadding () const
{
    return m_padding;
}

void RtpHeader::SetPadding (bool padding)
{
    m_padding = padding;
}

bool RtpHeader::IsExtension () const
{
    return m_extension;
}

void RtpHeader::SetExtension (bool extension)
{
    m_extension = extension;
}

bool RtpHeader::IsMarker () const
{
    return m_marker;
}

void RtpHeader::SetMarker (bool marker)
{
    m_marker = marker;
}


uint8_t RtpHeader::GetPayloadType () const
{
    return m_payloadType;
}

void RtpHeader::SetPayloadType (uint8_t payloadType)
{
    m_payloadType = payloadType;
}

uint16_t RtpHeader::GetSequence () const
{
    return m_sequence;
}

void RtpHeader::SetSequence (uint16_t sequence)
{
    m_sequence = sequence;
}

uint32_t RtpHeader::GetSsrc () const
{
    return m_ssrc;
}

void RtpHeader::SetSsrc (uint32_t ssrc)
{
    m_ssrc = ssrc;
}

uint32_t RtpHeader::GetTimestamp () const
{
    return m_timestamp;
}

void RtpHeader::SetTimestamp (uint32_t timestamp)
{
    m_timestamp = timestamp;
}

const std::set<uint32_t>& RtpHeader::GetCsrcs () const
{
    return m_csrcs;
}

bool RtpHeader::AddCsrc (uint32_t csrc)
{
    if (m_csrcs.count (csrc) != 0) {
        return false;
    }
    m_csrcs.insert (csrc);
    return true;
}


RtcpHeader::RtcpHeader ()
: Header{}
, m_padding{false}
, m_typeOrCnt{0}
, m_packetType{0}
, m_length{1}
, m_sendSsrc{0}
{}

RtcpHeader::RtcpHeader (uint8_t packetType)
: Header{}
, m_padding{false}
, m_typeOrCnt{0}
, m_packetType{packetType}
, m_length{1}
, m_sendSsrc{0}
{}

RtcpHeader::RtcpHeader (uint8_t packetType, uint8_t subType)
: Header{}
, m_padding{false}
, m_typeOrCnt{subType}
, m_packetType{packetType}
, m_length{1}
, m_sendSsrc{0}
{
    NS_ASSERT (subType <= 0x1f);
}

RtcpHeader::~RtcpHeader () {}

TypeId RtcpHeader::GetTypeId ()
{
    static TypeId tid = TypeId ("RtcpHeader")
      .SetParent<Header> ()
      .AddConstructor<RtcpHeader> ()
    ;
    return tid;
}

TypeId RtcpHeader::GetInstanceTypeId () const
{
    return GetTypeId ();
}

uint32_t RtcpHeader::GetSerializedSize () const
{
    return 1 + // First octet
            sizeof (m_packetType) +
            sizeof (m_length)     +
            sizeof (m_sendSsrc);
}

void RtcpHeader::SerializeCommon (Buffer::Iterator& start) const
{
    NS_ASSERT (m_typeOrCnt <= 0x1f);
    uint8_t octet1 = 0;
    octet1 |= (RTP_VERSION << 6);
    RtpHdrSetBit (octet1, 5, m_padding);
    octet1 |= uint8_t (m_typeOrCnt & 0x1f);
    start.WriteU8 (octet1);

    start.WriteU8 (m_packetType);
    start.WriteHtonU16 (m_length);
    start.WriteHtonU32 (m_sendSsrc);
}

void RtcpHeader::Serialize (Buffer::Iterator start) const
{
    SerializeCommon (start);
}

uint32_t RtcpHeader::DeserializeCommon (Buffer::Iterator& start)
{
    const auto octet1 = start.ReadU8 ();
    const uint8_t version = (octet1 >> 6);
    m_padding = RtpHdrGetBit (octet1, 5);
    m_typeOrCnt = octet1 & 0x1f;

    m_packetType = start.ReadU8 ();
    m_length = start.ReadNtohU16 ();
    m_sendSsrc = start.ReadNtohU32 ();
    NS_ASSERT (version == RTP_VERSION);
    return GetSerializedSize ();

}

uint32_t RtcpHeader::Deserialize (Buffer::Iterator start)
{
    return DeserializeCommon (start);
}

void RtcpHeader::PrintN (std::ostream& os) const
{
    os << "Rtcp Common Header - version = " << RTP_VERSION
       << ", padding =" << (m_padding ? "yes" : "no")
       << ", type/count = " << m_typeOrCnt
       << ", packet type = " << m_packetType
       << ", length = " << m_length
       << ", ssrc of RTCP sender = " << m_sendSsrc;
}
void RtcpHeader::Print (std::ostream& os) const
{
    PrintN (os);
    os << std::endl;
}

bool RtcpHeader::IsPadding () const
{
    return m_padding;
}

void RtcpHeader::SetPadding (bool padding)
{
    m_padding = padding;
}

uint8_t RtcpHeader::GetTypeOrCount () const
{
    return m_typeOrCnt;
}

void RtcpHeader::SetTypeOrCount (uint8_t typeOrCnt)
{
    m_typeOrCnt = typeOrCnt;
}

uint8_t RtcpHeader::GetPacketType () const
{
    return m_packetType;
}

void RtcpHeader::SetPacketType (uint8_t packetType)
{
    m_packetType = packetType;
}

uint32_t RtcpHeader::GetSendSsrc () const
{
    return m_sendSsrc;
}

void RtcpHeader::SetSendSsrc (uint32_t sendSsrc)
{
    m_sendSsrc = sendSsrc;
}


CCFeedbackHeader::CCFeedbackHeader ()
: RtcpHeader{RTP_FB, RTCP_RTPFB_CC}
, m_reportBlocks{}
, m_latestTsUs{0}
{
    ++m_length; // report timestamp field
}

CCFeedbackHeader::~CCFeedbackHeader () {}

TypeId CCFeedbackHeader::GetTypeId ()
{
    static TypeId tid = TypeId ("CCFeedbackHeader")
      .SetParent<RtcpHeader> ()
      .AddConstructor<CCFeedbackHeader> ()
    ;
    return tid;
}

TypeId CCFeedbackHeader::GetInstanceTypeId () const
{
    return GetTypeId ();
}


CCFeedbackHeader::RejectReason
CCFeedbackHeader::AddFeedback (uint32_t ssrc, uint16_t seq, uint64_t timestampUs, uint8_t ecn)
{
    if (ecn > 0x03) {
        return CCFB_BAD_ECN;
    }
    auto& rb = m_reportBlocks[ssrc];
    if (rb.find (seq) != rb.end ()) {
        return CCFB_DUPLICATE;
    }
    auto& mb = rb[seq];
    mb.m_timestampUs = timestampUs;
    mb.m_ecn = ecn;
    if (!UpdateLength ()) {
        rb.erase (seq);
        if (rb.empty ()) {
            m_reportBlocks.erase (ssrc);
        }
        return CCFB_TOO_LONG;
    }
    m_latestTsUs = std::max (m_latestTsUs, timestampUs);
    return CCFB_NONE;
}

void CCFeedbackHeader::GetSsrcList (std::set<uint32_t>& rv) const
{
    rv.clear ();
    for (const auto& rb : m_reportBlocks) {
        rv.insert (rb.first);
    }
}

bool CCFeedbackHeader::GetMetricList (uint32_t ssrc,
                                      std::vector<std::pair<uint16_t, MetricBlock> >& rv) const
{
    const auto it = m_reportBlocks.find (ssrc);
    if (it == m_reportBlocks.end ()) {
        return false;
    }
    rv.clear ();
    const auto& rb = it->second;
    NS_ASSERT (!rb.empty ()); // at least one metric block
    const auto beginStop = CalculateBeginStopSeq (rb);
    const uint16_t beginSeq = beginStop.first;
    const uint16_t stopSeq = beginStop.second;
    for (uint16_t i = beginSeq; i != stopSeq; ++i) {
        const auto& mb_it = rb.find (i);
        const bool received = (mb_it != rb.end ());
        if (received) {
            const auto item = std::make_pair (i, mb_it->second);
            rv.push_back (item);
        }
    }
    return true;
}

uint32_t CCFeedbackHeader::GetSerializedSize () const
{
    NS_ASSERT (m_length >= 2);
    const auto commonHdrSize = RtcpHeader::GetSerializedSize ();
    return commonHdrSize + (m_length - 1) * 4;
}

void CCFeedbackHeader::Serialize (Buffer::Iterator start) const
{
    NS_ASSERT (m_length >= 2); // TODO: To authors: are 0 report blocks allowed?
    RtcpHeader::SerializeCommon (start);

    for (const auto& rb : m_reportBlocks) {
        start.WriteHtonU32 (rb.first);
        const auto beginStop = CalculateBeginStopSeq (rb.second);
        const uint16_t beginSeq = beginStop.first;
        const uint16_t stopSeq = beginStop.second;
        start.WriteHtonU16 (beginSeq);
        start.WriteHtonU16 (stopSeq - 1);
        NS_ASSERT (!rb.second.empty ()); // at least one metric block
        for (uint16_t i = beginSeq; i != stopSeq; ++i) {
            const auto& mb_it = rb.second.find (i);
            uint8_t octet1 = 0;
            uint8_t octet2 = 0;
            const bool received = (mb_it != rb.second.end ());
            RtpHdrSetBit (octet1, 7, received);
            if (received) {
                const auto& mb = mb_it->second;
                NS_ASSERT (mb.m_ecn <= 0x03);
                octet1 |= uint8_t ((mb.m_ecn & 0x03) << 5);
                const uint16_t ato = TsToAto (mb.m_timestampUs);
                NS_ASSERT (ato <= 0x1fff);
                octet1 |= uint8_t (ato >> 8);
                octet2 |= uint8_t (ato & 0xff);
            }
            start.WriteU8 (octet1);
            start.WriteU8 (octet2);
        }
        if (uint16_t (stopSeq - beginSeq) % 2 == 1) {
            start.WriteHtonU16 (0); //padding
        }
    }
    // TODO (next patch): convert from Us to NTP time
    start.WriteHtonU32 (m_latestTsUs);
}

uint32_t CCFeedbackHeader::Deserialize (Buffer::Iterator start)
{
    NS_ASSERT (m_length >= 2);
    (void) RtcpHeader::DeserializeCommon (start);
    NS_ASSERT (m_packetType == RTP_FB);
    NS_ASSERT (m_typeOrCnt == RTCP_RTPFB_CC);
    //length of all report blocks in 16-bit words
    size_t len_left = (size_t (m_length - 2 /* sender SSRC + Report Tstmp*/ )) * 2;
    while (len_left > 0) {
        NS_ASSERT (len_left >= 4); // SSRC + begin & end
        const auto ssrc = start.ReadNtohU32 ();
        auto& rb = m_reportBlocks[ssrc];
        const uint16_t beginSeq = start.ReadNtohU16 ();
        const uint16_t endSeq = start.ReadNtohU16 ();
        len_left -= 4;
        const uint32_t nMetricBlocks = uint32_t (endSeq - beginSeq) + 1; //this wraps properly
        NS_ASSERT (nMetricBlocks <= 0xffff);// length of 65536 not supported
        const uint32_t nPaddingBlocks = nMetricBlocks % 2;
        NS_ASSERT (len_left >= nMetricBlocks + nPaddingBlocks);
        uint16_t seq = beginSeq;
        for (auto i = 0; i < nMetricBlocks; ++i) {
            const auto octet1 = start.ReadU8 ();
            const auto octet2 = start.ReadU8 ();
            if (RtpHdrGetBit (octet1, 7)) {
                auto &mb = rb[seq];
                mb.m_ecn = (octet1 >> 5) & 0x03;
                uint16_t ato = (uint16_t (octet1) << 8) & 0x1f00;
                ato |= uint16_t (octet2);
                mb.m_ato = ato;
            }
            ++seq;
        }
        len_left -= nMetricBlocks;
        if (nPaddingBlocks == 1) {
            start.ReadNtohU16 (); //skip padding
            --len_left;
        }
    }
    // TODO (next patch): convert from NTP time to Us
    m_latestTsUs = start.ReadNtohU32 ();
    // Populate all timestamps once Report Timestamp is known
    // TODO: report to authors
    for (auto& rb : m_reportBlocks) {
        for (auto& mb : rb.second) {
            mb.second.m_timestampUs = AtoToTs (mb.second.m_ato);
        }
    }
    return GetSerializedSize ();
}

void CCFeedbackHeader::Print (std::ostream& os) const
{
    NS_ASSERT (m_length >= 2);
    RtcpHeader::PrintN (os);
    size_t i = 0;
    for (const auto& rb : m_reportBlocks) {
        const auto beginStop = CalculateBeginStopSeq (rb.second);
        const uint16_t beginSeq = beginStop.first;
        const uint16_t stopSeq = beginStop.second;
        os << ", report block #" << i << " = "
           << "{ SSRC = " << rb.first
           << " [" << beginSeq << ".." << (stopSeq - 1) << "] --> ";
        for (uint16_t j = beginSeq; j != stopSeq; ++j) {
            const auto& mb_it = rb.second.find (j);
            const bool received = (mb_it != rb.second.end ());
            os << "<L=" << uint8_t (received);
            if (received) {
                const auto& mb = mb_it->second;
                os << ", ECN=0x" << std::hex << mb.m_ecn
                   << ", ATO=0x" << TsToAto (mb.m_timestampUs);
            }
            os << ">,";
        }
        os << " }, ";
        ++i;
    }
    // TODO (next patch): convert from Us to NTP time
    os << "report timestamp = " << m_latestTsUs << std::endl;
}

std::pair<uint16_t, uint16_t>
CCFeedbackHeader::CalculateBeginStopSeq (const ReportBlock_t& rb)
{
    NS_ASSERT (!rb.empty ()); // at least one metric block
    auto mb_it = rb.begin ();
    const uint16_t first = mb_it->first;
    if (rb.size () == 1) {
        return std::make_pair (first, first + 1);
    }
    //calculate biggest gap
    uint16_t low = first;
    ++mb_it;
    uint16_t high = mb_it->first;
    uint16_t max_lo = low;
    uint16_t max_hi = high;
    for (; mb_it != rb.end (); ++mb_it) {
        low = high;
        high = mb_it->first;
        if ((high - low) > (max_hi - max_lo)) {
            max_lo = low;
            max_hi = high;
        }
    }
    //check the gap across wrapping
    if ((first - high) > (max_hi - max_lo)) {
        max_lo = high;
        max_hi = first;
    }
    ++max_lo;
    NS_ASSERT (max_hi != max_lo); // length of 65536 not supported
    return std::make_pair (max_hi, max_lo);
}

bool CCFeedbackHeader::UpdateLength ()
{
    size_t len = 1; // SSRC of packet sender
    for (const auto& rb : m_reportBlocks) {
        ++len; // SSRC
        ++len; // begin & end seq
        const auto beginStop = CalculateBeginStopSeq (rb.second);
        const uint16_t beginSeq = beginStop.first;
        const uint16_t stopSeq = beginStop.second;
        const uint16_t nMetricBlocks = stopSeq - beginSeq; //this wraps properly
        const uint16_t nPaddingBlocks = nMetricBlocks % 2;
        len += (nMetricBlocks + nPaddingBlocks) / 2; // metric blocks are 16 bits long
    }
    ++len; // report timestamp field
    if (len > 0xffff) {
        return false;
    }
    m_length = len;
    return true;
}

uint16_t CCFeedbackHeader::TsToAto (uint64_t tsUs) const
{
    NS_ASSERT (tsUs <= m_latestTsUs);
    const uint64_t offsetUs = m_latestTsUs - tsUs;
    uint64_t ato = offsetUs * 1024 / 1000 / 1000;
    ato = std::min<uint64_t> (ato, MetricBlock::m_overrange);
    return uint16_t (ato);
}

uint64_t CCFeedbackHeader::AtoToTs (uint16_t ato) const
{
    // ato contains offset measured in 1/1024 seconds
    if (ato == MetricBlock::m_overrange) {
        //print warning
    }
    const uint64_t offsetUs = uint64_t (ato) * 1000 * 1000 / 1024;
    return m_latestTsUs - offsetUs;
}

}
