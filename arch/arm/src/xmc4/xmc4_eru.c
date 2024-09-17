*****************************************************************************arch / arm / src / xmc4 / xmc4_ccu4.c **Licensed to the Apache Software Foundation(ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * XMC CCU Driver
 *
 * For now, this file contains only helper methods mandatory for xmc tickless
 * feature. Contibutions are welcomed.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>

#include <assert.h>
#include <errno.h>
#include <debug.h>

#include "arm_internal.h"
#include "hardware/xmc4_eru.h"
#include "xmc4_eru.h"

/****************************************************************************
 * Private Datas
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: xmc4_eru_enable
 *
 * Description:
 *  Enable the clock and De-assert the ERU module from the reset state.
 *
 ****************************************************************************/
void xmc4_eru_enable(xmc4_eru_t const eru)
{
    if (eru == XMC_ERU1)
    {
#ifdef XMC4_SCU_GATING
        /* Check if peripheral is gated */

        if ((getreg32(XMC4_SCU_CGATSTAT0) && SCU_CGAT0_ERU1))
        {
            putreg32(SCU_CGAT0_ERU1, XMC4_SCU_CGATCLR0); /* Ungate it */
        }

#endif
        /* Check if peripheral reset is asserted */

        if ((getreg32(XMC4_SCU_PRSTAT0) && SCU_PR0_ERU1RS))
        {
            putreg32(SCU_PR0_ERU1RS, XMC4_SCU_PRCLR0); /* De-assert reset */
        }
    }
}

/****************************************************************************
 * Name: xmc4_eru_disable
 *
 * Description:
 *  Disable the clock and Reset the ERU module.
 *
 ****************************************************************************/
void xmc4_eru_disable(XMC_ERU_t *const eru)
{
    if (eru == XMC_ERU1)
    {
        /* Assert reset */

        putreg32(SCU_PR0_ERU1RS, XMC4_SCU_PRSET0);

#ifdef XMC4_SCU_GATING
        /* Gate clock */

        putreg32(SCU_PR0_ERU1RS, XMC4_SCU_CGATSET0);
#endif
    }
}

/****************************************************************************
 * Name: xmc4_eru_etl_initialize
 *
 * Description:
 *  Initializes the selected ERU ETLx channel.
 *
 ****************************************************************************/
int xmc4_eru_etl_initialize(XMC_ERU_t const eru, const uint8_t channel, const uint32_t config_input)
{
    if (channel > 3)
    {
        return -EINVAL;
    }

    /* EXICONx CONFIG
     * enable_output_trigger:  0      (disabled) Enables the generation of trigger pulse(PE), for the configured edge detection.
     * status_flag_mode:       0      (sticky flag) Enables the status flag auto clear(LD), for the opposite edge of the configured event edge.
     * rising_edge_detection:  0x01 (RE) Configure the event trigger edge(FE, RE).
     * output_trigger_channel: 0x000  (OGU1) Output channel select(OCS) for ETLx output trigger pulse.
     * source:                 0x0000 Input path combination along with polarity for event generation.
     */
    uint32_t exiconx = 0;
    exiconx |= (uint32_t)(0 << ERU_EXICON_PE_SHIFT);
    exiconx |= (uint32_t)(0 << ERU_EXICON_LD_SHIFT);
    exiconx |= (uint32_t)(XMC_ERU_ETL_EDGE_DETECTION_DISABLED << ERU_EXICON_RE_SHIFT);
    exiconx |= (uint32_t)(0 << ERU_EXICON_OCS_SHIFT);
    exiconx |= (uint32_t)(XMC_ERU_ETL_SOURCE_A << ERU_EXICON_SS_SHIFT);

    /* Defines input signal for path A or B of ERSx(Event request source, x = [0 to 3]) unit. */

    xmc4_eru_enable(eru);

    if (eru == XMC_ERU0)
    {
        /* Config EXISEL */
        uint32_t exisel = getreg32(XMC4_ERU0_EXISEL);
        uint32_t channel_mask = (uint32_t)((ERU_EXISEL_EXS0A_MASK | ERU_EXISEL_EXS0B_MASK) << (channel * 4UL));
        exisel &= ~channel_mask;
        exisel |= (uint32_t)(config_input << (channel * 4UL));
        putreg32(exisel, XMC4_ERU0_EXISEL);

        /* Configure channel x EXICONx */
        uint8_t exiconx_regaddr = (uint32_t)(XMC4_ERU0_EXICON0 + channel * 0x04);
        putreg32(exiconx, exiconx_regaddr);
    }
    else if (eru == XMC_ERU1)
    {
        /* Config EXISEL */
        uint32_t exisel = getreg32(XMC4_ERU1_EXISEL);
        uint32_t channel_mask = (uint32_t)((ERU_EXISEL_EXS0A_MASK | ERU_EXISEL_EXS0B_MASK) << (channel * 4UL));
        exisel &= ~channel_mask;
        exisel |= (uint32_t)(config_input << (channel * 4UL));
        putreg32(exisel, XMC4_ERU1_EXISEL);

        /* Configure channel x EXICONx */
        uint8_t exiconx_regaddr = (uint32_t)(XMC4_ERU1_EXICON0 + channel * 0x04);
        uint32_t exiconx = 0; /* Need config here */
        putreg32(exiconx, exiconx_regaddr);
    }

    return OK;
}

/****************************************************************************
 * Name: xmc4_eru_ogu_initialize
 *
 * Description:
 *  Initializes the selected ERU ETLx channel.
 *
 ****************************************************************************/
int xmc4_eru_ogu_initialize(XMC_ERU_t const eru, const uint8_t channel, const uint32_t config_input)
{
    if (channel > 3)
    {
        return -EINVAL;
    }

    /*
     * peripheral_trigger:       0 () Peripheral trigger(ISS) input selection.
     * enable_pattern_detection: 0 () Enable generation of(GEEN) event for pattern detection result change.
     * service_request:          0 () Gating(GP) on service request generation for pattern detection result.
     * pattern_detection_input:  0 () Enable input for the pattern detection(IPENx, x = [0 to 3]).
     */

    uint32_t exoconx = 0;


    /* Defines input signal for path A or B of ERSx(Event request source, x = [0 to 3]) unit. */

    if (eru == XMC_ERU0)
    {
        /* Configure channel x EXOCONx */
        uint8_t exoconx_regaddr = (uint32_t)(XMC4_ERU0_EXOCON0 + channel * 0x04);
        uint32_t exoconx = 0; /* Need config here */
        putreg32(exoconx, exoconx_regaddr);
    }
    else if (eru == XMC_ERU1)
    {
        /* Configure channel x EXOCONx */
        uint8_t exoconx_regaddr = (uint32_t)(XMC4_ERU1_EXOCON0 + channel * 0x04);
        uint32_t exoconx = 0; /* Need config here */
        putreg32(exoconx, exoconx_regaddr);
    }

    return OK;
}
