//app_call.h

#ifndef UIP_APPCALL
    #define UIP_APPCALL tcp_appcall
#endif
#define UIP_UDP_APPCALL udp_appcall

void tcp_appcall(void);
void udp_appcall(void);
