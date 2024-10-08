/****************************************************************************
 * video/videomode/vesagtf.c
 *
 * SPDX-License-Identifier: Apache-2.0
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

/* The logic in this file program was based on the Generalized Timing
 * Formula(GTF TM) Standard Version: 1.0, Revision: 1.0
 *
 * NOTES:
 *
 * The GTF allows for computation of "margins" (the visible border
 * surrounding the addressable video); on most non-overscan type
 * systems, the margin period is zero.  I've implemented the margin
 * computations but not enabled it because 1) I don't really have
 * any experience with this, and 2) neither XFree86 modelines nor
 * fbset fb.modes provide an obvious way for margin timings to be
 * included in their mode descriptions (needs more investigation).
 *
 * The GTF provides for computation of interlaced mode timings;
 * I've implemented the computations but not enabled them, yet.
 * I should probably enable and test this at some point.
 *
 * TODO:
 *
 * o Add support for interlaced modes.
 *
 * o Implement the other portions of the GTF: compute mode timings
 *   given either the desired pixel clock or the desired horizontal
 *   frequency.
 *
 * o It would be nice if this were more general purpose to do things
 *   outside the scope of the GTF: like generate double scan mode
 *   timings, for example.
 *
 * o Printing digits to the right of the decimal point when the
 *   digits are 0 annoys me.
 *
 * o Error checking.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/types.h>

#include <nuttx/video/videomode.h>
#include <nuttx/video/vesagtf.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define CELL_GRAN            8    /* Assumed character cell granularity */

/* c' and m' are part of the Blanking Duty Cycle computation
 *
 * #define C_PRIME           (((c - j) * k/256.0) + j)
 * #define M_PRIME           (k/256.0 * m)
 */

/* c' and m' multiplied by 256 to give integer math.  Make sure to
 * scale results using these back down, appropriately.
 */

#define C_PRIME256(p)       (((p->c - p->j) * p->k) + (p->j * 256))
#define M_PRIME256(p)       (p->k * p->m)

#define DIVIDE(x,y)         (((x) + ((y) / 2)) / (y))

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: vesagtf_mode_params
 *
 * Description:
 *   vesagtf_mode_params() - as defined by the GTF Timing Standard, compute
 *   the Stage 1 Parameters using the vertical refresh frequency.  In other
 *   words: input a desired resolution and desired refresh rate, and
 *   output the GTF mode timings.
 *
 ****************************************************************************/

void vesagtf_mode_params(unsigned int x, unsigned int y,
                         unsigned int refresh,
                         FAR struct vesagtf_params *params,
                         unsigned int flags,
                         FAR struct videomode_s *videomode)
{
  uint64_t h_period_est;
  uint64_t v_field_est;
  uint64_t h_period;
  uint64_t ideal_duty_cycle;

  unsigned int v_field_rqd;
  unsigned int top_margin;
  unsigned int bottom_margin;
  unsigned int interlace;
  unsigned int vsync_plus_bp;
  unsigned total_v_lines;
  unsigned int left_margin;
  unsigned int right_margin;
  unsigned int total_active_pixels;
  unsigned int h_blank;
  unsigned int h_pixels;
  unsigned int v_lines;
  unsigned int total_pixels;
  unsigned int pixel_freq;
  unsigned int h_sync;
  unsigned int h_front_porch;
  unsigned int v_odd_front_porch_lines;

#if 0 /* Unused, not needed */
  unsigned int v_field_rate;
  unsigned int v_back_porch;
  unsigned int v_frame_rate;
  unsigned int h_freq;
#endif

  /*  1. In order to give correct results, the number of horizontal
   *  pixels requested is first processed to ensure that it is divisible
   *  by the character size, by rounding it to the nearest character
   *  cell boundary:
   *
   *  [H PIXELS RND] = ((ROUND([H PIXELS]/[CELL GRAN RND],0))*[CELLGRAN RND])
   */

  h_pixels = DIVIDE(x, CELL_GRAN) * CELL_GRAN;

  /*  2. If interlace is requested, the number of vertical lines assumed
   *  by the calculation must be halved, as the computation calculates
   *  the number of vertical lines per field. In either case, the
   *  number of lines is rounded to the nearest integer.
   *
   *  [V LINES RND] = IF([INT RQD?]="y", ROUND([V LINES]/2,0),
   *                                     ROUND([V LINES],0))
   */

  v_lines = (flags & VESAGTF_FLAG_ILACE) ? DIVIDE(y, 2) : y;

  /*  3. Find the frame rate required:
   *
   *  [V FIELD RATE RQD] = IF([INT RQD?]="y", [I/P FREQ RQD]*2,
   *                                          [I/P FREQ RQD])
   */

  v_field_rqd = (flags & VESAGTF_FLAG_ILACE) ? (refresh * 2) : (refresh);

  /*  4. Find number of lines in Top margin:
   *  5. Find number of lines in Bottom margin:
   *
   *  [TOP MARGIN (LINES)] = IF([MARGINS RQD?]="Y",
   *          ROUND(([MARGIN%]/100*[V LINES RND]),0),
   *          0)
   *
   *  Ditto for bottom margin.  Note that instead of %, we use PPT, which
   *  is parts per thousand.  This helps us with integer math.
   */

  top_margin    = (flags & VESAGTF_FLAG_MARGINS) ?
                  DIVIDE(v_lines * params->margin_ppt, 1000) : 0;
  bottom_margin = top_margin;

  /*  6. If interlace is required, then set variable [INTERLACE]=0.5:
   *
   *  [INTERLACE]=(IF([INT RQD?]="y",0.5,0))
   *
   *  To make this integer friendly, we use some special hacks in step
   *  7 below.  Please read those comments to understand why I am using
   *  a whole number of 1.0 instead of 0.5 here.
   */

  interlace = (flags & VESAGTF_FLAG_ILACE) ? 1 : 0;

  /*  7. Estimate the Horizontal period
   *
   *  [H PERIOD EST] = ((1/[V FIELD RATE RQD]) - [MIN VSYNC+BP]/1000000) /
   *                    ([V LINES RND] + (2*[TOP MARGIN (LINES)]) +
   *                     [MIN PORCH RND]+[INTERLACE]) * 1000000
   *
   *  To make it integer friendly, we pre-multiply the 1000000 to get to
   *  usec.  This gives us:
   *
   *  [H PERIOD EST] = ((1000000/[V FIELD RATE RQD]) - [MIN VSYNC+BP]) /
   *                  ([V LINES RND] + (2 * [TOP MARGIN (LINES)]) +
   *                   [MIN PORCH RND]+[INTERLACE])
   *
   *  The other problem is that the interlace value is wrong.  To get
   *  the interlace to a whole number, we multiply both the numerator and
   *  divisor by 2, so we can use a value of either 1 or 0 for the interlace
   *  factor.
   *
   * This gives us:
   *
   * [H PERIOD EST] = ((2*((1000000/[V FIELD RATE RQD]) - [MIN VSYNC+BP])) /
   *                   (2*([V LINES RND] + (2*[TOP MARGIN (LINES)]) +
   *                    [MIN PORCH RND]) + [2*INTERLACE]))
   *
   * Finally we multiply by another 1000, to get value in picosec.
   * Why picosec?  To minimize rounding errors.  Gotta love integer
   * math and error propagation.
   */

  h_period_est = DIVIDE(((DIVIDE(2000000000000ULL, v_field_rqd)) -
                         (2000000 * params->min_vsbp)),
                        ((2 * (v_lines +
                         (2 * top_margin) + params->min_porch)) +
                         interlace));

  /*  8. Find the number of lines in V sync + back porch:
   *
   *  [V SYNC+BP] = ROUND(([MIN VSYNC+BP]/[H PERIOD EST]),0)
   *
   *  But recall that h_period_est is in psec. So multiply by 1000000.
   */

  vsync_plus_bp = DIVIDE(params->min_vsbp * 1000000, h_period_est);

#if 0 /* Not needed */
  /*  9. Find the number of lines in V back porch alone:
   *
   *  [V BACK PORCH] = [V SYNC+BP] - [V SYNC RND]
   *
   *  XXX is "[V SYNC RND]" a typo? should be [V SYNC RQD]?
   */

  v_back_porch = vsync_plus_bp - params->vsync_rqd;
#endif

  /*  10. Find the total number of lines in Vertical field period:
   *
   *  [TOTAL V LINES] = [V LINES RND] + [TOP MARGIN (LINES)] +
   *                    [BOT MARGIN (LINES)] + [V SYNC+BP] + [INTERLACE] +
   *                    [MIN PORCH RND]
   */

  total_v_lines = v_lines + top_margin + bottom_margin + vsync_plus_bp +
                  interlace + params->min_porch;

  /*  11. Estimate the Vertical field frequency:
   *
   *  [V FIELD RATE EST] = 1 / [H PERIOD EST] / [TOTAL V LINES] * 1000000
   *
   *  Again, we want to pre multiply by 10^9 to convert for nsec, thereby
   *  making it usable in integer math.
   *
   *  So we get:
   *
   *  [V FIELD RATE EST] = 1000000000 / [H PERIOD EST] / [TOTAL V LINES]
   *
   *  This is all scaled to get the result in uHz.  Again, we're trying to
   *  minimize error propagation.
   */

  v_field_est = DIVIDE(DIVIDE(1000000000000000ULL, h_period_est),
                       total_v_lines);

  /*  12. Find the actual horizontal period:
   *
   *  [H PERIOD] = [H PERIOD EST] / ([V FIELD RATE RQD] / [V FIELD RATE EST])
   */

  h_period = DIVIDE(h_period_est * v_field_est, v_field_rqd * 1000);

#if 0 /* Not needed */
  /*  13. Find the actual Vertical field frequency:
   *
   *  [V FIELD RATE] = 1 / [H PERIOD] / [TOTAL V LINES] * 1000000
   *
   *  And again, we convert to nsec ahead of time, giving us:
   *
   *  [V FIELD RATE] = 1000000 / [H PERIOD] / [TOTAL V LINES]
   *
   *  And another rescaling back to mHz.  Gotta love it.
   */

  v_field_rate = DIVIDE(1000000000000ULL, h_period * total_v_lines);

  /*  14. Find the Vertical frame frequency:
   *
   *  [V FRAME RATE] = (IF([INT RQD?]="y", [V FIELD RATE]/2, [V FIELD RATE]))
   *
   *  N.B. that the result here is in mHz.
   */

  v_frame_rate = (flags & VESAGTF_FLAG_ILACE) ?
                 v_field_rate / 2 : v_field_rate;
#endif

  /*  15. Find number of pixels in left margin:
   *  16. Find number of pixels in right margin:
   *
   *  [LEFT MARGIN (PIXELS)] = (IF( [MARGINS RQD?]="Y",
   *          (ROUND( ([H PIXELS RND] * [MARGIN%] / 100 /
   *                   [CELL GRAN RND]),0)) * [CELL GRAN RND],
   *          0))
   *
   *  Again, we deal with margin percentages as PPT (parts per thousand).
   *  And the calculations for left and right are the same.
   */

  left_margin = right_margin = (flags & VESAGTF_FLAG_MARGINS) ?
      DIVIDE(DIVIDE(h_pixels * params->margin_ppt, 1000),
          CELL_GRAN) * CELL_GRAN : 0;

  /*  17. Find total number of active pixels in image and left and right
   *  margins:
   *
   *  [TOTAL ACTIVE PIXELS] = [H PIXELS RND] + [LEFT MARGIN (PIXELS)] +
   *                          [RIGHT MARGIN (PIXELS)]
   */

  total_active_pixels = h_pixels + left_margin + right_margin;

  /*  18. Find the ideal blanking duty cycle from the blanking duty cycle
   *  equation:
   *
   *  [IDEAL DUTY CYCLE] = [c'] - ([m']*[H PERIOD]/1000)
   *
   *  However, we have modified values for [c'] as [256*c'] and
   *  [m'] as [256*m'].  Again the idea here is to get good scaling.
   *  We use 256 as the factor to make the math fast.
   *
   *  Note that this means that we have to scale it appropriately in
   *  later calculations.
   *
   *  The ending result is that our ideal_duty_cycle is 256000x larger
   *  than the duty cycle used by VESA.  But again, this reduces error
   *  propagation.
   */

  ideal_duty_cycle =
      ((C_PRIME256(params) * 1000) -
          (M_PRIME256(params) * h_period / 1000000));

  /*  19. Find the number of pixels in the blanking time to the nearest
   *  double character cell:
   *
   *  [H BLANK (PIXELS)] = (ROUND(([TOTAL ACTIVE PIXELS] *
   *                               [IDEAL DUTY CYCLE] /
   *                               (100-[IDEAL DUTY CYCLE]) /
   *                               (2*[CELL GRAN RND])), 0))
   *                       * (2*[CELL GRAN RND])
   *
   *  Of course, we adjust to make this rounding work in integer math.
   */

  h_blank = DIVIDE(DIVIDE(total_active_pixels * ideal_duty_cycle,
                       (256000 * 100ULL) - ideal_duty_cycle),
      2 * CELL_GRAN) * (2 * CELL_GRAN);

  /*  20. Find total number of pixels:
   *
   *  [TOTAL PIXELS] = [TOTAL ACTIVE PIXELS] + [H BLANK (PIXELS)]
   */

  total_pixels = total_active_pixels + h_blank;

  /*  21. Find pixel clock frequency:
   *
   *  [PIXEL FREQ] = [TOTAL PIXELS] / [H PERIOD]
   *
   *  We calculate this in Hz rather than MHz, to get a value that
   *  is usable with integer math.  Recall that the [H PERIOD] is in
   *  nsec.
   */

  pixel_freq = DIVIDE(total_pixels * 1000000, DIVIDE(h_period, 1000));

#if 0 /* Not needed */
  /*  22. Find horizontal frequency:
   *
   *  [H FREQ] = 1000 / [H PERIOD]
   *
   *  We calculate this in Hz rather than kHz, to avoid rounding
   *  errors.  Recall that the [H PERIOD] is in usec.
   */

  h_freq = 1000000000 / h_period;
#endif

  /* Stage 1 computations are now complete; I should really pass
   * the results to another function and do the Stage 2
   * computations, but I only need a few more values so I'll just
   * append the computations here for now.
   */

  /*  17. Find the number of pixels in the horizontal sync period:
   *
   *  [H SYNC (PIXELS)] =(ROUND(([H SYNC%] / 100 * [TOTAL PIXELS] /
   *                             [CELL GRAN RND]),0))*[CELL GRAN RND]
   *
   *  Rewriting for integer math:
   *
   *  [H SYNC (PIXELS)]=(ROUND((H SYNC%] * [TOTAL PIXELS] / 100 /
   *                             [CELL GRAN RND),0))*[CELL GRAN RND]
   */

  h_sync = DIVIDE(((params->hsync_pct * total_pixels) / 100), CELL_GRAN) *
                  CELL_GRAN;

  /*  18. Find the number of pixels in the horizontal front porch period:
   *
   *  [H FRONT PORCH (PIXELS)] = ([H BLANK (PIXELS)]/2)-[H SYNC (PIXELS)]
   *
   *  Note that h_blank is always an even number of characters (i.e.
   *  h_blank % (CELL_GRAN * 2) == 0)
   */

  h_front_porch = (h_blank / 2) - h_sync;

  /*  36. Find the number of lines in the odd front porch period:
   *
   *  [V ODD FRONT PORCH(LINES)]=([MIN PORCH RND]+[INTERLACE])
   *
   *  Adjusting for the fact that the interlace is scaled:
   *
   *  [V ODD FRONT PORCH(LINES)]=(([MIN PORCH RND] * 2) + [2*INTERLACE]) / 2
   */

  v_odd_front_porch_lines = ((2 * params->min_porch) + interlace) / 2;

  /* finally, pack the results in the mode struct */

  videomode->hsync_start = h_pixels + h_front_porch;
  videomode->hsync_end   = videomode->hsync_start + h_sync;
  videomode->htotal      = total_pixels;
  videomode->hdisplay    = h_pixels;

  videomode->vsync_start = v_lines + v_odd_front_porch_lines;
  videomode->vsync_end   = videomode->vsync_start + params->vsync_rqd;
  videomode->vtotal      = total_v_lines;
  videomode->vdisplay    = v_lines;

  videomode->dotclock    = pixel_freq;
}

/****************************************************************************
 * Name: vesagtf_mode
 *
 * Description:
 *   Use VESA GTF formula to generate monitor timings.  Assumes default
 *   GTF parameters, non-interlaced, and no margins.
 *
 ****************************************************************************/

void vesagtf_mode(unsigned int x, unsigned int y, unsigned int refresh,
                  FAR struct videomode_s *videomode)
{
  struct vesagtf_params params;

  params.margin_ppt = VESAGTF_MARGIN_PPT;
  params.min_porch  = VESAGTF_MIN_PORCH;
  params.vsync_rqd  = VESAGTF_VSYNC_RQD;
  params.hsync_pct  = VESAGTF_HSYNC_PCT;
  params.min_vsbp   = VESAGTF_MIN_VSBP;
  params.m          = VESAGTF_M;
  params.c          = VESAGTF_C;
  params.k          = VESAGTF_K;
  params.j          = VESAGTF_J;

  vesagtf_mode_params(x, y, refresh, &params, 0, videomode);
}
