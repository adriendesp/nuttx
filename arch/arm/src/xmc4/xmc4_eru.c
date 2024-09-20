/*****************************************************************************
 * arch / arm / src / xmc4 / xmc4_eru.c
 * Licensed to the Apache Software Foundation(ASF) under one or more
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
#include "hardware/xmc4_scu.h"

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
void xmc4_eru_disable(xmc4_eru_t const eru)
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
int xmc4_eru_etl_initialize(xmc4_eru_t const eru, const uint8_t channel, const xmc4_eru_etl_config_t *const config_etl)
{
    if (channel > 3)
    {
        return -EINVAL;
    }

    uint32_t exisel_regaddr = 0;

    if (eru == XMC_ERU0)
    {
        exisel_regaddr = (uint32_t)XMC4_ERU0_EXISEL;
    }
    else if (eru == XMC_ERU1)
    {
        exisel_regaddr = (uint32_t)XMC4_ERU1_EXISEL;
    }
    else
    {
        return -EINVAL;
    }

    uint32_t exiconx_regaddr = 0;

    if (eru == XMC_ERU0)
    {
        exiconx_regaddr = (uint32_t)(XMC4_ERU0_EXICON0 + channel * 0x04);
    }
    else if (eru == XMC_ERU1)
    {
        exiconx_regaddr = (uint32_t)(XMC4_ERU1_EXICON0 + channel * 0x04);
    }
    else
    {
        return -EINVAL;
    }

    /* EXICONx CONFIG
     * enable_output_trigger : Enables the generation of trigger pulse(PE), for the configured edge detection.
     * status_flag_mode:       Enables the status flag auto clear(LD), for the opposite edge of the configured event edge.
     * rising_edge_detection:  Configure the event trigger edge(FE, RE).
     * output_trigger_channel: Output OGUy select(OCS) for ETLx output trigger pulse.
     * source:                 Input path combination along with polarity for event generation.
     */

    uint32_t exiconx = 0;
    exiconx |= (uint32_t)(config_etl->enable_output_trigger << ERU_EXICON_PE_SHIFT);
    exiconx |= (uint32_t)(config_etl->status_flag_mode << ERU_EXICON_LD_SHIFT);
    exiconx |= (uint32_t)(config_etl->edge_detection << ERU_EXICON_RE_SHIFT);
    exiconx |= (uint32_t)(config_etl->output_trigger_channel << ERU_EXICON_OCS_SHIFT);
    exiconx |= (uint32_t)(config_etl->source << ERU_EXICON_SS_SHIFT);

    uint32_t config_input = (uint32_t)(config_etl->input_a);
    config_input |= (uint32_t)(config_etl->input_b << 2);

    xmc4_eru_enable(eru);

    /* Config EXISEL */

    uint32_t channel_mask = (uint32_t)((ERU_EXISEL_EXS0A_MASK | ERU_EXISEL_EXS0B_MASK) << (channel * 4UL));
    modifyreg32(exisel_regaddr, channel_mask, (uint32_t)(config_input << (channel * 4UL)));

    /* Configure channel x EXICONx */

    putreg32(exiconx, exiconx_regaddr, );

    return OK;
}

/****************************************************************************
 * Name: xmc4_eru_ogu_initialize
 *
 * Description:
 *  Initializes the selected ERU ETLx channel.
 *
 ****************************************************************************/
int xmc4_eru_ogu_initialize(xmc4_eru_t const eru, const uint8_t channel, const xmc4_eru_ogu_config_t *const config_ogu)
{
    if (channel > 3)
    {
        return -EINVAL;
    }

    uint32_t exoconx_regaddr = 0;

    if (eru == XMC_ERU0)
    {
        exoconx_regaddr = (uint32_t)(XMC4_ERU0_EXOCON0 + channel * 0x04);
    }
    else if (eru == XMC_ERU1)
    {
        exoconx_regaddr = (uint32_t)(XMC4_ERU1_EXOCON0 + channel * 0x04);
    }
    else
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
    exoconx |= (uint32_t)(config_ogu->peripheral_trigger << ERU_EXOCON_ISS_SHIFT);
    exoconx |= (uint32_t)(config_ogu->enable_pattern_detection << ERU_EXOCON_GEEN_SHIFT);
    exoconx |= (uint32_t)(config_ogu->service_request << ERU_EXOCON_GP_SHIFT);
    exoconx |= (uint32_t)(config_ogu->pattern_detection_input << ERU_EXOCON_IPEN0_SHIFT);

    putreg32(exoconx, exoconx_regaddr);

    return OK;
}

void xmc4_eru_etl_clear_status_flag(xmc4_eru_t const eru, const uint8_t channel)
{
    uint32_t exoconx_regaddr = 0;

    if (eru == XMC_ERU0)
    {
        exoconx_regaddr = (uint32_t)(XMC4_ERU0_EXOCON0 + channel * 0x04);
    }
    else if (eru == XMC_ERU1)
    {
        exoconx_regaddr = (uint32_t)(XMC4_ERU1_EXOCON0 + channel * 0x04);
    }
    else
    {
        return -EINVAL;
    }

    /* Set EXICONx.FL to 0 */

    modifyreg32(exiconx_regaddr, (uint32_t)ERU_EXICON_FL_MASK, 0);
}

bool xmc4_eru_etl_get_status_flag(xmc4_eru_t const eru, const uint8_t channel)
{
    uint32_t exoconx_regaddr = 0;

    if (eru == XMC_ERU0)
    {
        exoconx_regaddr = (uint32_t)(XMC4_ERU0_EXOCON0 + channel * 0x04);
    }
    else if (eru == XMC_ERU1)
    {
        exoconx_regaddr = (uint32_t)(XMC4_ERU1_EXOCON0 + channel * 0x04);
    }
    else
    {
        return -EINVAL;
    }

    /* Get EXICONx.FL */

    return ((getreg32(exiconx_regaddr) & ERU_EXICON_FL_MASK) != 0);
}
