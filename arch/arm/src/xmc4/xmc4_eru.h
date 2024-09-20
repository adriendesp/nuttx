/****************************************************************************
 * arch/arm/src/xmc4/xmc4_eru.h
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
#include "hardware/xmc4_eru_pinmap.h"

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

typedef enum xmc4_eru_etl_output_trigger_channel
{
  XMC_ERU_ETL_OUTPUT_TRIGGER_OGU0 = 0U, /* Event from input ETLx triggers output OGU0 */
  XMC_ERU_ETL_OUTPUT_TRIGGER_OGU1 = 1U, /* Event from input ETLx triggers output OGU1 */
  XMC_ERU_ETL_OUTPUT_TRIGGER_OGU2 = 2U, /* Event from input ETLx triggers output OGU2 */
  XMC_ERU_ETL_OUTPUT_TRIGGER_OGU3 = 3U, /* Event from input ETLx triggers output OGU3 */
} xmc4_eru_etl_output_trigger_channel_t;

typedef enum xmc4_eru_etl_status_flag_mode
{
  XMC_ERU_ETL_STATUS_FLAG_MODE_SWCTRL = 0U,
  XMC_ERU_ETL_STATUS_FLAG_MODE_HWCTRL = 1U
} xmc4_eru_etl_status_flag_mode_t;

typedef enum xmc4_eru_ogu_service_request
{
  XMC_ERU_OGU_SERVICE_REQUEST_DISABLED = 0U,
  XMC_ERU_OGU_SERVICE_REQUEST_ON_TRIGGER = 1U,
  XMC_ERU_OGU_SERVICE_REQUEST_ON_TRIGGER_AND_PATTERN_MATCH = 2U,
  XMC_ERU_OGU_SERVICE_REQUEST_ON_TRIGGER_AND_PATTERN_MISMATCH = 3U
} xmc4_eru_ogu_service_request_t;

typedef struct xmc4_eru_etl_config
{
      uint8_t input_a;
      uint8_t input_b;
      bool enable_output_trigger;
      bool status_flag_mode;
      uint8_t edge_detection;
      uint8_t output_trigger_channel;
      uint8_t source;

} xmc4_eru_etl_config_t;

typedef struct xmc4_eru_ogu_config
{
    uint8_t peripheral_trigger;
    bool enable_pattern_detection;
    uint8_t service_request;
    uint8_t pattern_detection_input;
} xmc4_eru_ogu_config_t;

/****************************************************************************
 * Public Functions Prototypes
 ****************************************************************************/
void xmc4_eru_enable(xmc4_eru_t const eru);
void xmc4_eru_disable(xmc4_eru_t const eru);
int xmc4_eru_etl_initialize(xmc4_eru_t const eru, const uint8_t channel, const xmc4_eru_etl_config_t *const config_etl);
int xmc4_eru_ogu_initialize(xmc4_eru_t const eru, const uint8_t channel, const xmc4_eru_ogu_config_t *const config_ogu);
void xmc4_eru_etl_clear_status_flag(xmc4_eru_t const eru, const uint8_t channel);
bool xmc4_eru_etl_get_status_flag(xmc4_eru_t const eru, const uint8_t channel);
