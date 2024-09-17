/****************************************************************************
 * arch/arm/src/xmc4/xmc4_gpio.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
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

#include "hardware/xmc4_eru.h"

/****************************************************************************
 * Public Types
 ****************************************************************************/
typedef enum xmc4_eru
{
    XMC_ERU0 = 0U,
    XMC_ERU1 = 1U
} xmc4_eru_t;

typedef enum xmc4_eru_etl_input_a
{
    XMC_ERU_ETL_INPUT_A0 = 0x0U, /* input A0 is selected */
    XMC_ERU_ETL_INPUT_A1 = 0x1U, /* input A1 is selected */
    XMC_ERU_ETL_INPUT_A2 = 0x2U, /* input A2 is selected */
    XMC_ERU_ETL_INPUT_A3 = 0x3U  /* input A3 is selected */
} xmc4_eru_etl_input_a_t;

typedef enum xmc4_eru_etl_input_b
{
    XMC_ERU_ETL_INPUT_B0 = 0x0U, /* input B0 is selected */
    XMC_ERU_ETL_INPUT_B1 = 0x1U, /* input B1 is selected */
    XMC_ERU_ETL_INPUT_B2 = 0x2U, /* input B2 is selected */
    XMC_ERU_ETL_INPUT_B3 = 0x3U  /* input B3 is selected */
} xmc4_eru_etl_input_b_t;

typedef enum xmc4_eru_etl_source{
  XMC_ERU_ETL_SOURCE_A = 0x0U,              /* (A) path as a event source */
  XMC_ERU_ETL_SOURCE_B = 0x1U,              /* (B) path as a event source */
  XMC_ERU_ETL_SOURCE_A_OR_B = 0x2U,         /* (A OR B) path as a event source */
  XMC_ERU_ETL_SOURCE_A_AND_B = 0x3U,        /* (A AND B) path as a event source */
  XMC_ERU_ETL_SOURCE_NOT_A = 0x4U,          /* (NOT A) path as a event source */
  XMC_ERU_ETL_SOURCE_NOT_A_OR_B = 0x6U,     /* (NOT A OR B) path as a event source */
  XMC_ERU_ETL_SOURCE_NOT_A_AND_B = 0x7U,    /* ( NOT A AND B) path as a event source */
  XMC_ERU_ETL_SOURCE_NOT_B = 0x9U,          /* ( NOT B) path as a event source */
  XMC_ERU_ETL_SOURCE_A_OR_NOT_B = 0xaU,     /* (A OR  NOT B) path as a event source */
  XMC_ERU_ETL_SOURCE_A_AND_NOT_B = 0xbU,    /* (A AND  NOT B) path as a event source */
  XMC_ERU_ETL_SOURCE_NOT_A_OR_NOT_B = 0xeU, /* ( NOT A OR  NOT B) path as a event source */
  XMC_ERU_ETL_SOURCE_NOT_A_AND_NOT_B = 0xfU /* ( NOT A AND  NOT B) path as a event source */
} xmc4_eru_etl_source_t;

typedef enum xmc4_eru_etl_edge
{
  XMC_ERU_ETL_EDGE_DETECTION_DISABLED = 0U, /* no event enabled */
  XMC_ERU_ETL_EDGE_DETECTION_RISING = 1U,   /* detection of rising edge generates the event */
  XMC_ERU_ETL_EDGE_DETECTION_FALLING = 2U,  /* detection of falling edge generates the event */
  XMC_ERU_ETL_EDGE_DETECTION_BOTH = 3U      /* detection of either edges generates the event */
} xmc4_eru_etl_edge_t;

/****************************************************************************
 * Public Functions Prototypes
 ****************************************************************************/
void xmc4_eru_enable(xmc4_eru_t const eru);
void xmc4_eru_disable(XMC_ERU_t *const eru);
int xmc4_eru_etl_initialize(XMC_ERU_t const eru, const uint8_t channel, const uint32_t config_input);
int xmc4_eru_ogu_initialize(XMC_ERU_t const eru, const uint8_t channel, const uint32_t config_input);