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
 * Dummy controller (CBR) implementation for rmcat ns3 module.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */
#include "dummy-controller.h"
#include <sstream>
#include <cassert>

namespace rmcat {

DummyController::DummyController() :
    SenderBasedController{},
    m_lastTimeCalcUs{0},
    m_lastTimeCalcValid{false},
    m_QdelayUs{0},
    m_ploss{0},
    m_plr{0.f},
    m_RecvR{0.} {}

DummyController::~DummyController() {}

void DummyController::setCurrentBw(float newBw) {
    m_initBw = newBw;
}

void DummyController::reset() {
    m_lastTimeCalcUs = 0;
    m_lastTimeCalcValid = false;

    m_QdelayUs = 0;
    m_ploss = 0;
    m_plr = 0.f;
    m_RecvR = 0.;

    SenderBasedController::reset();
}

bool DummyController::processFeedback(uint64_t nowUs,
                                      uint16_t sequence,
                                      uint64_t rxTimestampUs,
                                      uint8_t ecn) {
    // First of all, call the superclass
    const bool res = SenderBasedController::processFeedback(nowUs, sequence,
                                                            rxTimestampUs, ecn);
    const uint64_t calcIntervalUs = 200 * 1000;
    if (m_lastTimeCalcValid) {
        assert(lessThan(m_lastTimeCalcUs, nowUs + 1));
        if (nowUs - m_lastTimeCalcUs >= calcIntervalUs) {
            updateMetrics();
            logStats(nowUs);
            m_lastTimeCalcUs = nowUs;
        }
    } else {
        m_lastTimeCalcUs = nowUs;
        m_lastTimeCalcValid = true;
    }
    return res;
}

float DummyController::getBandwidth(uint64_t nowUs) const {

    return m_initBw;
}

void DummyController::updateMetrics() {
    uint64_t qdelayUs;
    bool qdelayOK = getCurrentQdelay(qdelayUs);
    if (qdelayOK) m_QdelayUs = qdelayUs;

    float rrate;
    bool rrateOK = getCurrentRecvRate(rrate);
    if (rrateOK) m_RecvR = rrate;

    uint32_t nLoss;
    float plr;
    bool plrOK = getPktLossInfo(nLoss, plr);
    if (plrOK) {
        m_ploss = nLoss;
        m_plr = plr;
    }
}

void DummyController::logStats(uint64_t nowUs) const {

    std::ostringstream os;
    os << std::fixed;
    os.precision(RMCAT_LOG_PRINT_PRECISION);

    os  << " algo:dummy " << m_id
        << " ts: "     << (nowUs / 1000)
        << " loglen: " << m_packetHistory.size()
        << " qdel: "   << (m_QdelayUs / 1000)
        << " ploss: "  << m_ploss
        << " plr: "    << m_plr
        << " rrate: "  << m_RecvR
        << " srate: "  << m_initBw;
    logMessage(os.str());
}

}
