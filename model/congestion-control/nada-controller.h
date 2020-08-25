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
 * NADA controller interface for rmcat ns3 module.
 *
 * @version 0.1.1
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#ifndef NADA_CONTROLLER_H
#define NADA_CONTROLLER_H

#include "sender-based-controller.h"

namespace rmcat {

/**
 * This class corresponds to the congestion control scheme
 * named Network-Assisted Dynamic Adaptation (NADA). Details
 * of the algorithm are documented in the following IETF
 * draft (rmcat-nada):
 *
 * NADA: A Unified Congestion Control Scheme for Real-Time Media
 * https://tools.ietf.org/html/draft-ietf-rmcat-nada-04
 *
 */
class NadaController: public SenderBasedController {
public:
    /* class constructor */
    NadaController();

    /* class destructor */
    virtual ~NadaController();

    /**
     * Set the current bandwidth estimation. This can be useful in test environments
     * to temporarily disrupt the current bandwidth estimation
     *
     * @param [in] newBw Bandwidth estimation to overwrite the current estimation
     */
    virtual void setCurrentBw(float newBw);

    /**
     * NADA's implementation of the #reset virtual function;
     * resets internal states to initial values
     */
    virtual void reset();

    /** NADA's implementation of the #processSendPacket API */
    virtual bool processSendPacket(uint64_t txTimestampUs,
                                   uint16_t sequence,
                                   uint32_t size); // in Bytes

    /** NADA's implementation of the #processFeedback API */
    virtual bool processFeedback(uint64_t nowUs,
                                 uint16_t sequence,
                                 uint64_t rxTimestampUs,
                                 uint8_t ecn=0);

    /** NADA's implementation of the #processFeedbackBatch API */
    virtual bool processFeedbackBatch(uint64_t nowUs,
                                      const std::vector<FeedbackItem>& feedbackBatch);

    /** NADA's realization of the #getBandwidth API */
    virtual float getBandwidth(uint64_t nowUs) const;

private:

    /**
     * Function for retrieving updated estimates
     * (by the base class SenderBasedController) of
     * delay, loss, and receiving rate metrics and
     * copying them to local member variables
     */
    void updateMetrics();

    /**
     * Function for printing losses, delay, and rate
     * metrics to log in a pre-formatted manner
     * @param [in] nowUs current timestamp in microseconds
     */
    void logStats(uint64_t nowUs, uint64_t deltaUs) const;

    /**
     * Function for calculating the target bandwidth
     * following the NADA algorithm
     *
     * @param [in] deltaUs interval from last bandwidth calculation
     *                     in microseconds
     */
    void updateBw(uint64_t deltaUs);

    /**
     * Function for calculating the reference rate (r_ref)
     * during the gradual update mode. Typically this is during
     * the steay-state phase of the algorithm.
     *
     * See Section 4.3 and Eq.(5)-(7) in the rmcat-nada draft
     * for greater detail.
     *
     * @param [in] deltaUs interval from last bandwidth calculation in microseconds
     */
    void calcGradualRateUpdate(uint64_t deltaUs);

    /**
     * Function for calculating the reference rate (r_ref)
     * during the accelerated ramp-up mode. Typically this
     * is carried out during the congestion-free periods of
     * the flow, e.g., when a flow enters an empty link
     * or when existing background flows exit and opens up
     * more available bandwidth.
     *
     * See Section 4.3 and Eq. (3)-(4) in the rmcat-nada
     * draft for greater detail.
     *
     */
    void calcAcceleratedRampUp();

    /**
     * Function for determining wether the sender should
     * operate in accelerated ramp-up mode or gradual
     * update model.
     *
     * @retval 0 if the sender should operate in accelerated
     *         ramp-up mode (rmode == 0 as in draft-rmcat-nada)
     *         and 1 if the sender should operate in
     *         gradual update mode (rmode == 1 as in
     *         draft-rmcat-nada)
     */
    int getRampUpMode();

    /**
     * Function for calculating the aggregated congestion
     * signal (x_curr) based on packet statistics both
     * in terms of loss and delay.
     */
    void updateXcurr();

    /**
     * Function for calculating the non-linear warping
     * of queuing delay in ms (d_tilde in rmcat-nada),
     * when the NADA sender is operating in loss-based
     * mode
     *
     * @retval value of the warped delay-based congestion
     *         signal value in ms (d_tilde in rmcat-nada)
     */
    float calcDtilde() const;

    /**
     * Following are local member variables recording current
     * packet loss/delay information, as well as operational
     * mode of the NADA algorithm
     */
    uint32_t m_ploss; /**< packet loss count within configured window */
    float m_plr;     /**< packet loss ratio within packet history window */
    bool m_warpMode;  /**< whether to perform non-linear warping of queuing delay */

    /** timestamp of when r_ref is last calculated (t_last in rmcat-nada), in microseconds  */
    uint64_t m_lastTimeCalcUs;
    /** whether value m_lastTimeCalc is valid: not valid before first rate update */
    bool m_lastTimeCalcValid;

    float m_currBw; /**< calculated reference rate (r_ref in rmcat-nada) */

    uint64_t m_QdelayUs; /**< estimated queuing delay in microseconds */
    uint64_t m_RttUs; /**< estimated RTT value in microseconds */
    float m_Xcurr;  /**< aggregated congestion signal (x_curr in rmcat-nada) in ms */
    float m_Xprev;  /**< previous value of the aggregated congestion signal (x_prev in rmcat-nada), in ms */
    float m_RecvR;  /**< updated receiving rate in bps */
    float m_avgInt; /**< Average inter-loss interval in packets, according to RFC 5348 */
    uint32_t m_currInt; /**< Most recent (currently growing) inter-loss interval in packets; called I_0 in RFC 5348 */
    bool m_lossesSeen; /**< Whether packet losses/reorderings have been detected so far */
};

}

#endif /* NADA_CONTROLLER_H */
