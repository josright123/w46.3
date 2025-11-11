/**
 * \author van Kempen Alexandre
 * \mainpage PTPd v2 Documentation
 * \version 2.0.1
 * \date 17 nov 2010
 * \section implementation Implementation
 * PTPd is full implementation of IEEE 1588 - 2008 standard of ordinary clock.
*/



/**
*\file
* \brief Main functions used in ptpdv2
*
* This header file includes all others headers.
* It defines functions which are not dependant of the operating system.
 */

#ifndef PTPD_V51_H_
#define PTPD_V51_H_


#include "constants.h"
#include "dep/constants_dep.h"
#include "dep/datatypes_dep.h"
#include "datatypes.h"
#include "dep/ptpd_dep.h"

#define DM_ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))

extern const char *zEventMode[];
int evt_index(UInteger16 mask);

PtpClock *netif2ptpclock(struct netif *inp);

int	PTPCLOCK_IS_V51(PtpClock *ptpClock);

const char *zNetifString(struct netif *nif);
const char *zClockString(PtpClock *ptpClock); //TransClockFrom()
u8_t TransPtpclockIndex(PtpClock *ptpClock);
char *parse_buf_ptp_packet(uint8_t *p, int off);
char *parse_buf_ptp_packet1(uint8_t *p, int off);
char *parse_pbuf_ptp_packet(struct pbuf *p, int in_offset);
char *parse_pbuf_ptp_packet1(struct pbuf *p, int in_offset);

//const char *TransStateName(Enumeration8 state);
extern const char *zStateMode[];

//const char *TransMechanismName(Enumeration8 delayMechanism);
extern const char *SyncMechMode[];

void V51_JJ_test_ptptime(char *head);
void emac_JJ_test_ptptime(struct ptptime_t *ptimestamp, char *head);

extern uint8_t g_time_buf[8];
//extern struct ptptime_t g_timestamp;

void v51_getTime(TimeInternal *time);
void v51_setTime(const TimeInternal *time);

void xxx_v51_ptptime_adjfreq(int32_t adj);
void v51_ptptime_adjfreq(char *head, int32_t adj, char *limi);
//void v51_ptptime_adjfreq_pps(/*char *head,*/ int64_t signed_addend/*, char *limi*/);
//int32_t v51_ptptime_getrate(int32_t *pRateValue);
//int32_t _v51_getPtpClockRate(int32_t *rateValuePtr);
Boolean v51_adjFreq(char *head, Integer32 adj);
Boolean v51_adjFreq_state(PtpClock *ptpClock, /*char *head,*/ Integer32 adj);

#if 0
/** \name arith.c
 * -Timing management and arithmetic*/
/**\{*/
/* arith.c */

/**
 * \brief Convert scaled nanoseconds into TimeInternal structure
 */
void scaledNanosecondsToInternalTime(const Integer64*, TimeInternal*);
/**
 * \brief Convert TimeInternal into Timestamp structure (defined by the spec)
 */
void fromInternalTime(const TimeInternal*, Timestamp*);

/**
 * \brief Convert Timestamp to TimeInternal structure (defined by the spec)
 */
void toInternalTime(TimeInternal*, const Timestamp*);

/**
 * \brief Add two TimeInternal structure and normalize
 */
void addTime(TimeInternal*, const TimeInternal*, const TimeInternal*);

/**
 * \brief Substract two TimeInternal structure and normalize
 */
void subTime(TimeInternal*, const TimeInternal*, const TimeInternal*);

/**
 * \brief Divide the TimeInternal by 2 and normalize
 */
void div2Time(TimeInternal*);

/**
 * \brief Returns the floor form of binary logarithm for a 32 bit integer.
 * -1 is returned if ''n'' is 0.
 */
Integer32 floorLog2(UInteger32);

/**
 * \brief return maximum of two numbers
 */
static __INLINE Integer32 max(Integer32 a, Integer32 b)
{
	return a > b ? a : b;
}

/**
 * \brief return minimum of two numbers
 */
static __INLINE Integer32 min(Integer32 a, Integer32 b)
{
	return a > b ? b : a;
}

/** \}*/
#endif

#if 0
/** \name bmc.c
 * -Best Master Clock Algorithm functions*/
/**\{*/
/* bmc.c */
/**
 * \brief Compare data set of foreign masters and local data set
 * \return The recommended state for the port
 */
UInteger8 bmc(PtpClock*);

/**
 * \brief When recommended state is Master, copy local data into parent and grandmaster dataset
 */
void m1(PtpClock*);

/**
 * \brief When recommended state is Passive
 */
void p1(PtpClock*);

/**
 * \brief When recommended state is Slave, copy dataset of master into parent and grandmaster dataset
 */
void s1(PtpClock*, const MsgHeader*, const MsgAnnounce*);

/**
 * \brief Initialize datas
 */
void initData(PtpClock*);

/**
 * \brief Compare two port identities
 */
Boolean isSamePortIdentity(const PortIdentity*, const PortIdentity*);

/**
 * \brief Add foreign record defined by announce message
 */
void addForeign(PtpClock*, const MsgHeader*, const MsgAnnounce*);


/** \}*/
#endif

#if 0
/** \name protocol.c
 * -Execute the protocol engine*/
/**\{*/
/**
 * \brief Protocol engine
 */
/* protocol.c */

/**
 * \brief Run PTP stack in current state
 */
void doState(PtpClock*);

/**
 * \brief Change state of PTP stack
 */
void toState(PtpClock*, UInteger8);
/** \}*/
#endif

/**
 * \brief Initialize PTP stack
 */
int PTPd_Init_v51(void);

/**
 * \brief PTP stack periodic handler while not using OS
 */
void ptpd_Periodic_Handle_v51(__IO UInteger32 localtime);

#endif /*PTPD_V51_H_*/
