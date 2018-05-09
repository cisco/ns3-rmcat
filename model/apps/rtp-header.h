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
 * Header interface of RTP packets (RFC 3550) for ns3-rmcat.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#ifndef RTP_HEADER_H
#define RTP_HEADER_H

#include "ns3/header.h"
#include "ns3/type-id.h"

namespace ns3 {

//-------------------- RTP HEADER (RFC 3550) ----------------------//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P|X|  CC   |M|     PT      |       sequence number         |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                           timestamp                           |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |           synchronization source (SSRC) identifier            |
//  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//  |            contributing source (CSRC) identifiers             |
//  |                             ....                              |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define RTP_VERSION    2

class RtpHeader : public ns3::Header
{
public:
    RtpHeader ();
    virtual ~RtpHeader ();

    static ns3::TypeId GetTypeId ();
    virtual ns3::TypeId GetInstanceTypeId () const;
    virtual uint32_t GetSerializedSize () const;
    virtual void Serialize (ns3::Buffer::Iterator start) const;
    virtual uint32_t Deserialize (ns3::Buffer::Iterator start);
    virtual void Print (std::ostream &os) const;

    bool m_padding;
    bool m_extension;
    bool m_marker;
    uint8_t m_payloadType;
    uint16_t m_sequence;
    uint32_t m_timestamp;
    uint32_t m_ssrc;
    std::vector<uint32_t> m_csrc;

protected:
    static void SetBit (uint8_t& val, uint8_t pos, bool bit);
    static bool GetBit (uint8_t val, uint8_t pos);

};


// TODO (deferred): implement header format as described in
//                  draft-dt-rmcat-feedback-message

//--------------------- FEEDBACK HEADER ---------------------------//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                           flow_id                             |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                          sequence                             |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  receive t(ime)st(a)mp  (1)                   |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  receive t(ime)st(a)mp  (2)                   |
//  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
class FeedbackHeader : public ns3::Header
{
public:
    virtual ~FeedbackHeader ();

    static ns3::TypeId GetTypeId ();
    virtual ns3::TypeId GetInstanceTypeId () const;
    virtual uint32_t GetSerializedSize () const;
    virtual void Serialize (ns3::Buffer::Iterator start) const;
    virtual uint32_t Deserialize (ns3::Buffer::Iterator start);
    virtual void Print (std::ostream &os) const;

    uint32_t flow_id;
    uint32_t sequence;
    uint64_t receive_tstmp;
};

}

#endif /* RTP_HEADER_H */
