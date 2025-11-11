/* sys.c */

#include "../ptpd.h"
//#include "lwip_ethif/nosys/ethif.h"
#if (EDRIVER_ADDING_PTP && LWIP_PTP)
#include "dm9051_edriver_extend/dm9051a_ptp.h" /* eth api */ //......bdbdbbbbbbbbbbd.....43t3gh............
#endif

void getTime(TimeInternal *time)
{
#if (EDRIVER_ADDING_PTP && LWIP_PTP)
    struct ptptime_t timestamp;

	dm9051_ptptime_gettime(&timestamp); //dm9051dev.ptptime_gettime(&timestamp);
    time->seconds = timestamp.tv_sec;
    time->nanoseconds = timestamp.tv_nsec;
#endif
}

void setTime(const TimeInternal *time)
{
#if (EDRIVER_ADDING_PTP && LWIP_PTP)
    struct ptptime_t ts;
    ts.tv_sec = time->seconds;
    ts.tv_nsec = time->nanoseconds;

    dm9051_ptptime_settime(&ts); //dm9051dev.ptptime_settime(&ts);
#endif
    DBG("resetting system clock to %ds %dns\n", time->seconds, time->nanoseconds);
}

void updateTime(const TimeInternal *time)
{
#if (EDRIVER_ADDING_PTP && LWIP_PTP)
    struct ptptime_t timeoffset;

    DBGV("updateTime: %ds %dns\n", time->seconds, time->nanoseconds);

    timeoffset.tv_sec = -time->seconds;
    timeoffset.tv_nsec = -time->nanoseconds;

	/* Coarse update method */
    dm9051_ptptime_updateoffset(&timeoffset); //dm9051dev.ptptime_updateoffset(&timeoffset);
#endif
    DBGV("updateTime: updated\n");
}

//UInteger32 getRand(UInteger32 randMax)
//{
//    return rand() % randMax;
//}

UInteger32 getRand(UInteger32 randMax)
{
    return (rand() % randMax) + 25;
}

Boolean adjFreq(Integer32 adj)
{
    DBGV("adjFreq %d\n", adj);

#if (EDRIVER_ADDING_PTP && LWIP_PTP)
    if (adj > ADJ_FREQ_MAX)
        adj = ADJ_FREQ_MAX;
    else if (adj < -ADJ_FREQ_MAX)
        adj = -ADJ_FREQ_MAX;

    /* Fine update method */
	dm9051_ptptime_adjfreq(adj); //dm9051dev.ptptime_adjfreq(adj);
#endif
    return TRUE;
}
