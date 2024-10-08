/****************************************************************************
 * net/neighbor/neighbor_lookup.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <debug.h>
#include <string.h>

#include <nuttx/net/ip.h>
#include <nuttx/net/neighbor.h>

#include "netdev/netdev.h"
#include "neighbor/neighbor.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct neighbor_table_info_s
{
  net_ipv6addr_t              ni_ipaddr; /* IPv6 address for lookup */
  FAR struct neighbor_addr_s *ni_laddr;  /* Location to return the link
                                          * layer address */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: neighbor_match
 *
 * Description:
 *   This is a callback that checks if the network device has the
 *   indicated IPv6 address assigned to it.
 *
 ****************************************************************************/

static int neighbor_match(FAR struct net_driver_s *dev, FAR void *arg)
{
  FAR struct neighbor_table_info_s *info = arg;

  /* Check if the network device has been assigned the IP address of the
   * lookup.
   */

  if (!NETDEV_IS_MY_V6ADDR(dev, info->ni_ipaddr))
    {
      return 0;
    }

  /* Yes.. Return the matching link layer address if the caller of
   * neighbor_lookup() provided a non-NULL location.
   */

  if (info->ni_laddr != NULL)
    {
      info->ni_laddr->na_lltype = dev->d_lltype;
      info->ni_laddr->na_llsize = netdev_lladdrsize(dev);
      memcpy(&info->ni_laddr->u, &dev->d_mac, info->ni_laddr->na_llsize);
    }

  /* Return success in any event */

  return 1;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name:  neighbor_lookup
 *
 * Description:
 *   Find an entry in the Neighbor Table and return its link layer address.
 *
 * Input Parameters:
 *   ipaddr - The IPv6 address to use in the lookup;
 *   laddr  - Location to return the corresponding link layer address.
 *            This address may be NULL.  In that case, this function may be
 *            used simply to determine if the link layer address is
 *            available.
 *
 * Returned Value:
 *   Zero (OK) if the link layer address is returned.  A negated errno value
 *   is returned on any error.
 *
 ****************************************************************************/

int neighbor_lookup(FAR const net_ipv6addr_t ipaddr,
                    FAR struct neighbor_addr_s *laddr)
{
  FAR struct neighbor_entry_s *neighbor;
  struct neighbor_table_info_s info;

  /* Check if the IPv6 address is already in the neighbor table. */

  neighbor = neighbor_findentry(ipaddr);
  if (neighbor != NULL)
    {
      /* Yes.. return the link layer address if the caller has provided a
       * non-NULL address in 'laddr'.
       */

      if (laddr != NULL)
        {
          memcpy(laddr, &neighbor->ne_addr, sizeof(*laddr));
        }

      /* Return success in any case meaning that a valid link layer
       * address mapping is available for the IPv6 address.
       */

      return OK;
    }

  /* No.. check if the IPv6 address is the address assigned to a local
   * network device.  If so, return a mapping of that IPv6 address
   * to the linker layer address assigned to the network device.
   */

  net_ipv6addr_copy(info.ni_ipaddr, ipaddr);
  info.ni_laddr = laddr;

  if (netdev_foreach(neighbor_match, &info) != 0)
    {
      return OK;
    }

  /* Not found */

  return -ENOENT;
}
