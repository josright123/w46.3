/* Networking DHCPv4 client */

/*
 * Copyright (c) 2017 ARM Ltd.
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(dhcpv4_client_sample, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/linker/sections.h>
#include <errno.h>
#include <stdio.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/ethernet_mgmt.h>

#define DHCP_OPTION_NTP (42)

static uint8_t ntp_server[4];

static struct net_mgmt_event_callback mgmt_cb;
static struct net_mgmt_event_callback mgmt_cb_eth;

static struct net_dhcpv4_option_callback dhcp_cb;

static void start_dhcpv4_client(struct net_if *iface, void *user_data)
{
	ARG_UNUSED(user_data);

	LOG_INF("Start on %s: index=%d", net_if_get_device(iface)->name,
		net_if_get_by_iface(iface));
	net_dhcpv4_start(iface);
}

static void handler(struct net_mgmt_event_callback *cb,
		    uint64_t mgmt_event,
		    struct net_if *iface)
{
	int i = 0;
	const struct device *dev = net_if_get_device(iface);
	const char *ifname = dev ? dev->name : "?";
	int ifindex = net_if_get_by_iface(iface);
	char lladdr_buf[3 * 16];
	const struct net_linkaddr *lladdr = net_if_get_link_addr(iface);
	int llpos = 0;

	lladdr_buf[0] = '\0';
	if (lladdr /*&& lladdr->addr*/ && lladdr->len > 0) {
		for (size_t j = 0; j < lladdr->len && j < 16; j++) {
			llpos += snprintk(lladdr_buf + llpos,
					 sizeof(lladdr_buf) - llpos,
					 "%s%02x",
					 (j == 0) ? "" : ":",
					 lladdr->addr[j]);
			if (llpos >= sizeof(lladdr_buf)) {
				break;
			}
		}
	}

	if (mgmt_event == NET_EVENT_ETHERNET_CARRIER_ON) {
		printk("[TRACE] *** CARRIER ON *** on %s (index=%d) ll=%s\n",
		       ifname, ifindex, lladdr_buf);
		return;
	}

	if (mgmt_event == NET_EVENT_ETHERNET_CARRIER_OFF) {
		printk("[TRACE] *** CARRIER OFF *** on %s (index=%d) ll=%s\n",
		       ifname, ifindex, lladdr_buf);
		return;
	}

	/* if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD ||) */
	if (mgmt_event == NET_EVENT_IPV4_DHCP_BOUND) {
		//if (iface == NULL || iface->config.ip.ipv4 == NULL) {
		//	LOG_WRN("IPv4 addr add event for iface without IPv4 config");
		//	return;
		//}

		for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
			char buf[NET_IPV4_ADDR_LEN];

			if (iface->config.ip.ipv4->unicast[i].ipv4.addr_type !=
								NET_ADDR_DHCP) {
				continue;
			}

			LOG_INF("   Address[%d]: %s", net_if_get_by_iface(iface),
				net_addr_ntop(NET_AF_INET,
					&iface->config.ip.ipv4->unicast[i].ipv4.address.in_addr,
							buf, sizeof(buf)));
			LOG_INF("    Subnet[%d]: %s", net_if_get_by_iface(iface),
				net_addr_ntop(NET_AF_INET,
						&iface->config.ip.ipv4->unicast[i].netmask,
						buf, sizeof(buf)));
			LOG_INF("    Router[%d]: %s", net_if_get_by_iface(iface),
				net_addr_ntop(NET_AF_INET,
							&iface->config.ip.ipv4->gw,
							buf, sizeof(buf)));
			LOG_INF("Lease time[%d]: %u seconds", net_if_get_by_iface(iface),
				iface->config.dhcpv4.lease_time);
		}
	}
}

static void option_handler(struct net_dhcpv4_option_callback *cb,
			   size_t length,
			   enum net_dhcpv4_msg_type msg_type,
			   struct net_if *iface)
{
	char buf[NET_IPV4_ADDR_LEN];

	LOG_INF("DHCP Option %d: %s", cb->option,
		net_addr_ntop(NET_AF_INET, cb->data, buf, sizeof(buf)));
}

int main(void)
{
	static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

	if (gpio_is_ready_dt(&led)) {
		gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	}

	LOG_INF("Run dhcpv4 client");

	net_mgmt_init_event_callback(&mgmt_cb, handler,
					/*NET_EVENT_IPV4_ADDR_ADD | */
					NET_EVENT_IPV4_DHCP_BOUND);
	net_mgmt_add_event_callback(&mgmt_cb);
	net_mgmt_init_event_callback(&mgmt_cb_eth, handler,
				     NET_EVENT_ETHERNET_CARRIER_ON |
				     NET_EVENT_ETHERNET_CARRIER_OFF);
	net_mgmt_add_event_callback(&mgmt_cb_eth);

	net_dhcpv4_init_option_callback(&dhcp_cb, option_handler,
					DHCP_OPTION_NTP, ntp_server,
					sizeof(ntp_server));

	net_dhcpv4_add_option_callback(&dhcp_cb);

	net_if_foreach(start_dhcpv4_client, NULL);

	printk("[TRACE] Entering main LED blink loop\n");
	while (1) {
		if (gpio_is_ready_dt(&led)) {
			gpio_pin_toggle_dt(&led);
		}
		k_msleep(1000);
	}
	return 0;
}
