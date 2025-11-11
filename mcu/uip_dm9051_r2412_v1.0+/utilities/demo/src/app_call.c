#include "stdio.h"
#include "uip.h"
#include "uip_arp.h"
#include "tapdev.h"

#include "app_call.h"

#include "dhcpc.h"
//#include "udpb.h"
//#include "telnetd.h"
#include "httpd.h"
//#include "tcp_demo.h"
//#include "iperf.h"

void tcp_appcall(void)
{
#if 1

    /* Local Port */
    switch (uip_conn->lport)
    {
        case HTONS(80):
            httpd_appcall();
            break;

        /*case HTONS(23):
                telnetd_appcall();
            break;
        case HTONS(1200):
                tcp_server_demo_appcall();
            break;
        case HTONS(5001):
                iperf_server_appcall();
          break;*/
        default:
            break;
    }

    /* Remote Port */
    switch (uip_conn->rport)
    {
        /*case HTONS(1400):
                tcp_client_demo_appcall();
            break;*/
        default:
            break;
    }

#endif //0
}

void udp_appcall(void)
{
    /* Remote Port  */
    switch (uip_udp_conn->rport)
    {
#ifdef __DHCPC_H__
        case HTONS(67): //Transmite UDP listen port
            dhcpc_appcall();
            break;

        case HTONS(68): //Received UDP listen port
            dhcpc_appcall();
            break;
#endif

        default:
            break;
    }
}
