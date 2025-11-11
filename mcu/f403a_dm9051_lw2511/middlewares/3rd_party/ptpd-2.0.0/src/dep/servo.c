#include "../ptpd.h"
#include "developer_conf.h"


void initClock(PtpClock *ptpClock)
{
    DBG("initClock\n");
    /* clear vars */
    ptpClock->Tms.seconds = ptpClock->Tms.nanoseconds = 0;
    ptpClock->observedDrift = 0;  /* clears clock servo accumulator (the I term) */

    /* one way delay */
    ptpClock->owd_filt.n = 0;
    ptpClock->owd_filt.s = ptpClock->servo.sDelay;

    /* offset from master */
    ptpClock->ofm_filt.n = 0;
    ptpClock->ofm_filt.s = ptpClock->servo.sOffset;

	/* scaled log variance */
    if (DEFAULT_PARENTS_STATS)
    {
        ptpClock->slv_filt.n = 0;
	    ptpClock->slv_filt.s = 6;
	    ptpClock->offsetHistory[0] = 0;
	    ptpClock->offsetHistory[1] = 0;
    }

    ptpClock->waitingForFollowUp = FALSE;

    ptpClock->waitingForPDelayRespFollowUp = FALSE;

    ptpClock->pdelay_t1.seconds = ptpClock->pdelay_t1.nanoseconds = 0;
    ptpClock->pdelay_t2.seconds = ptpClock->pdelay_t2.nanoseconds = 0;
    ptpClock->pdelay_t3.seconds = ptpClock->pdelay_t3.nanoseconds = 0;
    ptpClock->pdelay_t4.seconds = ptpClock->pdelay_t4.nanoseconds = 0;


    /* reset parent statistics */
    ptpClock->parentDS.parentStats = FALSE;
    ptpClock->parentDS.observedParentClockPhaseChangeRate = 0;
    ptpClock->parentDS.observedParentOffsetScaledLogVariance = 0;


    /* level clock */

    if (!ptpClock->servo.noAdjust) {
		printf("[PTPD] initClock adj %d\r\n", 0);
        adjFreq(0);
	}

    netEmptyEventQ(&ptpClock->netPath);
}

static Integer32 order(Integer32 n)
{
    if (n < 0) {
        n = -n;
    }
    if (n == 0) {
        return 0;
    }
    return floorLog2(n);
}

/* exponencial smoothing */
static void filter(Integer32 * nsec_current, Filter * filt)
{
    Integer32 s, s2;

    /*
        using floatingpoint math
        alpha = 1/2^s
        y[1] = x[0]
        y[n] = alpha * x[n-1] + (1-alpha) * y[n-1]
        
        or equivalent with integer math
        y[1] = x[0]
        y_sum[1] = y[1] * 2^s
        y_sum[n] = y_sum[n-1] + x[n-1] - y[n-1]
        y[n] = y_sum[n] / 2^s
    */

    filt->n++; /* increment number of samples */
    
    /* if it is first time, we are running filter, initialize it*/
    if (filt->n == 1) {
        filt->y_prev = *nsec_current;
        filt->y_sum = *nsec_current;
        filt->s_prev = 0;
    }

    s = filt->s;
    /* speedup filter, if not 2^s > n */
    if ((1<<s) > filt->n) {
        /* lower the filter order */
        s = order(filt->n);
    } else {
        /* avoid overflowing of n */
        filt->n = 1<<s;
    }
    /* avoid overflowing of filter. 30 is because using signed 32bit integers */
    s2 = 30 - order(max(filt->y_prev, *nsec_current));

    /* use the lower filter order, higher will overflow */
    s = min(s, s2);

    /* if the order of the filter changed, change also y_sum value */
    if(filt->s_prev > s) {
      filt->y_sum >>= (filt->s_prev - s);
    } else if (filt->s_prev < s) {
      filt->y_sum <<= (s - filt->s_prev);
    }

    /* compute the filter itself */
    filt->y_sum += *nsec_current - filt->y_prev;
    filt->y_prev = filt->y_sum >> s;

    /* save previous order of the filter */
    filt->s_prev = s;

    DBGV("filter: %d -> %d (%d)\n", *nsec_current, filt->y_prev, s);

    /* actualize target value */
    *nsec_current = filt->y_prev;
}

/* 11.2 */
void updateOffset(PtpClock *ptpClock, const TimeInternal *syncEventIngressTimestamp,
                  const TimeInternal *preciseOriginTimestamp, const TimeInternal *correctionField)
{
    DBGV("updateOffset\n");
    /*  <offsetFromMaster> = <syncEventIngressTimestamp> - <preciseOriginTimestamp>
       - <meanPathDelay>  -  correctionField  of  Sync  message
       -  correctionField  of  Follow_Up message. */

    /* compute offsetFromMaster */
    subTime(&ptpClock->Tms, syncEventIngressTimestamp, preciseOriginTimestamp);
    subTime(&ptpClock->Tms, &ptpClock->Tms, correctionField);

    ptpClock->currentDS.offsetFromMaster = ptpClock->Tms;

    switch (ptpClock->portDS.delayMechanism)
    {
    case E2E:
        subTime(&ptpClock->currentDS.offsetFromMaster, &ptpClock->currentDS.offsetFromMaster, &ptpClock->currentDS.meanPathDelay);
        break;

    case P2P:
        subTime(&ptpClock->currentDS.offsetFromMaster, &ptpClock->currentDS.offsetFromMaster, &ptpClock->portDS.peerMeanPathDelay);
        break;

    default:
        break;
    }

    if (0 != ptpClock->currentDS.offsetFromMaster.seconds)
    {
        if (ptpClock->portDS.portState == PTP_SLAVE)
        {
            setFlag(ptpClock->events, SYNCHRONIZATION_FAULT);
        }

        DBGV("updateOffset: cannot filter seconds\n");

        return;
    }

    /* filter offsetFromMaster */
    filter(&ptpClock->currentDS.offsetFromMaster.nanoseconds, &ptpClock->ofm_filt);

    /* check results */
    if (abs(ptpClock->currentDS.offsetFromMaster.nanoseconds) < DEFAULT_CALIBRATED_OFFSET_NS)
    {
        if (ptpClock->portDS.portState == PTP_UNCALIBRATED)
        {
            setFlag(ptpClock->events, MASTER_CLOCK_SELECTED);
        }
    }
    else if (abs(ptpClock->currentDS.offsetFromMaster.nanoseconds) > DEFAULT_UNCALIBRATED_OFFSET_NS)
    {
        if (ptpClock->portDS.portState == PTP_SLAVE)
        {
            setFlag(ptpClock->events, SYNCHRONIZATION_FAULT);
        }
    }
}

/* 11.3 */
void updateDelay(PtpClock * ptpClock, const TimeInternal *delayEventEgressTimestamp,
                 const TimeInternal *recieveTimestamp, const TimeInternal *correctionField)
{
    /* Tms valid ? */

    if (0 == ptpClock->ofm_filt.n)
    {
        DBGV("updateDelay: Tms is not valid");
        return;
    }

    subTime(&ptpClock->Tsm, recieveTimestamp, delayEventEgressTimestamp);

    subTime(&ptpClock->Tsm, &ptpClock->Tsm, correctionField);

    addTime(&ptpClock->currentDS.meanPathDelay, &ptpClock->Tms, &ptpClock->Tsm);
    div2Time(&ptpClock->currentDS.meanPathDelay);

    /* filter delay */

    if (0 != ptpClock->currentDS.meanPathDelay.seconds)
    {
        DBGV("updateDelay: cannot filter with seconds");
    }
    else
    {
        filter(&ptpClock->currentDS.meanPathDelay.nanoseconds, &ptpClock->owd_filt);
    }

}

void updatePeerDelay(PtpClock *ptpClock, const TimeInternal *correctionField, Boolean twoStep)
{
    DBGV("updatePeerDelay\n");

    if (twoStep)
    {
        TimeInternal Tab, Tba;
        subTime(&Tab, &ptpClock->pdelay_t2 , &ptpClock->pdelay_t1);
        subTime(&Tba, &ptpClock->pdelay_t4, &ptpClock->pdelay_t3);

        addTime(&ptpClock->portDS.peerMeanPathDelay, &Tab, &Tba);
    }
    else /*One step clock*/
    {
        subTime(&ptpClock->portDS.peerMeanPathDelay, &ptpClock->pdelay_t4, &ptpClock->pdelay_t1);
    }

    subTime(&ptpClock->portDS.peerMeanPathDelay, &ptpClock->portDS.peerMeanPathDelay, correctionField);

    div2Time(&ptpClock->portDS.peerMeanPathDelay);

    /* filter delay */

    if (0 != ptpClock->portDS.peerMeanPathDelay.seconds)
    {
        DBGV("updatePeerDelay: cannot filter with seconds");
        return;
    }
    else
    {
        filter(&ptpClock->portDS.peerMeanPathDelay.nanoseconds, &ptpClock->owd_filt);
    }
}

//[mqtt help debug!]
// 所有的回調函數
#define MQTT_PUB_PAYLOAD_BUFFER_SIZE					256
void publish_topic(const char *topic, int qos, char *payload);

#ifndef USE_CJSON_LIBRARY
/* ============================================================================
 * JSON Implementation: snprintf() - Manual JSON Construction (DEFAULT)
 * ============================================================================
 * Memory usage: Stack ~512 bytes, Heap 0 bytes
 * Execution time: ~100 μs
 * Advantages: High memory efficiency, no external dependencies
 * Disadvantages: Verbose code, difficult to maintain
 * ============================================================================
 */

/**
 * @brief  將 PTP 狀態資訊格式化為 JSON 字串（使用 snprintf 實作）
 * @param  status: PTP 狀態資訊結構
 * @param  json_buffer: 輸出 JSON 字串的緩衝區
 * @param  buffer_size: 緩衝區大小
 * @retval 實際寫入的字元數，失敗時返回 -1
 * @note   產生緊湊的 JSON 格式以節省記憶體
 * @note   此為預設實作，使用 snprintf() 手動建構 JSON
 */
int ptp_format_status_json(PtpClock *ptpClock, char *json_buffer, int buffer_size)
{
  int written = 0;

//  printf("[PTP-MQTT] Formatting PTP status to JSON using snprintf method\r\n");
//  if (!status || !json_buffer || buffer_size <= 0) {
//    return -1;
//  }
	
	#if 0
        sprintf(payload, "{ %s : \"offset from master:%10d s %11d ns, drift: %10d ns, path delay: %11d ns   %s\" }",
			PTPD_HEADER_MCU_DESC,
            ptpClock->currentDS.offsetFromMaster.seconds,
            ptpClock->currentDS.offsetFromMaster.nanoseconds,
            ptpClock->observedDrift,
            ptpClock->currentDS.meanPathDelay.nanoseconds, pbff);
	#endif

  /* 格式化 JSON 字串 - 使用緊湊格式節省空間 */
  written = snprintf(json_buffer, buffer_size,
    "{"
      "\"device\":{"
        "\"mcu\":\"%s\","
        "\"offset from master s\":\"%10d\","
        "\"offset from master ns\":\"%11d\","
        "\"observedDrift ns\":\"%10d\","
        "\"path delay ns\":\"%11d\""
      "}"
    "}",
	PTPD_HEADER_MCU_DESC,
	ptpClock->currentDS.offsetFromMaster.seconds,
	ptpClock->currentDS.offsetFromMaster.nanoseconds,
	ptpClock->observedDrift,
	ptpClock->currentDS.meanPathDelay.nanoseconds /*, pbff*/
#if 0
    "{"
      "\"device\":{"
        "\"id\":\"%s\","
        "\"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\","
        "\"ip\":\"%u.%u.%u.%u\""
      "},"
      "\"ptpd\":{"
        "\"state\":%u,"
        "\"state_str\":\"%s\","
        "\"offset\":{"
          "\"s\":%d,"
          "\"ns\":%d"
        "},"
        "\"drift_ns\":%d,"
        "\"path_delay_ns\":%d"
      "},"
      "\"timestamp\":{"
        "\"rtc_ns\":%llu,"
        "\"ptp_ns\":%llu,"
        "\"diff_ns\":%lld,"
        "\"system_ns\":%llu"
      "},"
      "\"stats\":{"
        "\"sync_count\":%u,"
        "\"is_synced\":%s"
      "}"
    "}",
    /* 裝置資訊 */
    status->device.device_id,
    status->device.mac_addr[0], status->device.mac_addr[1],
    status->device.mac_addr[2], status->device.mac_addr[3],
    status->device.mac_addr[4], status->device.mac_addr[5],
    (uint8_t)(status->device.ip_addr & 0xFF),
    (uint8_t)((status->device.ip_addr >> 8) & 0xFF),
    (uint8_t)((status->device.ip_addr >> 16) & 0xFF),
    (uint8_t)((status->device.ip_addr >> 24) & 0xFF),
    /* PTP 資訊 */
    status->port_state,
    ptp_get_state_string(status->port_state),
    status->offset_sec,
    status->offset_ns,
    status->drift_ns,
    status->path_delay_ns,
    /* 時間戳資訊 */
    status->rtc_time_ns,
    status->ptp_time_ns,
    status->time_diff_ns,
    status->system_time_ns,
    /* 統計資訊 */
    status->sync_count,
    status->is_synchronized ? "true" : "false"
#endif
  );

  /* 檢查是否超出緩衝區 */
  if (written >= buffer_size) {
    printf("[PTP-MQTT] Warning: JSON buffer overflow (needed: %d, available: %d)\r\n",
           written, buffer_size);
    return -1;
  }

  return written;
}

#endif /* !USE_CJSON_LIBRARY */

void updateClock(PtpClock *ptpClock)
{
    Integer32 adj;
    TimeInternal timeTmp;
    Integer32 offsetNorm;
	char pbff[68];
	char *ps[] = {
		"unknow",
		"setTime.initClock",
		"secs adjFreq",
		"pi ctrlr adjFreq",
	};
	int path = 0;
	strcpy(pbff, ps[path]);

    DBGV("updateClock\n");

    if (0 != ptpClock->currentDS.offsetFromMaster.seconds 
        || abs(ptpClock->currentDS.offsetFromMaster.nanoseconds) > MAX_ADJ_OFFSET_NS )
    {
        /* if secs, reset clock or set freq adjustment to max */
        if (!ptpClock->servo.noAdjust)
        {
            if (!ptpClock->servo.noResetClock)
            {
                getTime(&timeTmp);
                subTime(&timeTmp, &timeTmp, &ptpClock->currentDS.offsetFromMaster);
//				printf("updateClock.setTime.initClock\r\n");
                setTime(&timeTmp);
                initClock(ptpClock);
				path = 1;
				strcpy(pbff, ps[path]);
            }
            else
            {
                adj = ptpClock->currentDS.offsetFromMaster.nanoseconds > 0 ? ADJ_FREQ_MAX : -ADJ_FREQ_MAX;
//				printf("secs adjFreq %d\r\n", -adj);
                adjFreq(-adj);
				path = 2;
				sprintf(pbff, "%s %d", ps[path], -adj);
            }
        }
    }
    else
    {
        /* the PI controller */
  
        /* normalize offset to 1s sync interval -> response of the servo will
         * be same for all sync interval values, but faster/slower 
         * (possible lost of precision/overflow but much more stable) */
        offsetNorm = ptpClock->currentDS.offsetFromMaster.nanoseconds;
        if (ptpClock->portDS.logSyncInterval > 0) {
            offsetNorm >>= ptpClock->portDS.logSyncInterval;
        } else if (ptpClock->portDS.logSyncInterval < 0) {
            offsetNorm <<= -ptpClock->portDS.logSyncInterval;
        }
        
        /* the accumulator for the I component */
        ptpClock->observedDrift += offsetNorm / ptpClock->servo.ai;

        /* clamp the accumulator to ADJ_FREQ_MAX for sanity */

        if (ptpClock->observedDrift > ADJ_FREQ_MAX)
            ptpClock->observedDrift = ADJ_FREQ_MAX;
        else if (ptpClock->observedDrift < -ADJ_FREQ_MAX)
            ptpClock->observedDrift = -ADJ_FREQ_MAX;

        /* apply controller output as a clock tick rate adjustment */
        if (!ptpClock->servo.noAdjust)
        {
            adj = offsetNorm / ptpClock->servo.ap + ptpClock->observedDrift;
//			printf("pi ctrlr adjFreq %d\r\n", -adj);
            adjFreq(-adj);
			path = 3;
			sprintf(pbff, "%s %d", ps[path], -adj);
        }

	    if (DEFAULT_PARENTS_STATS)
	    {
			int a, scaledLogVariance;
    	    ptpClock->parentDS.parentStats = TRUE;
	        ptpClock->parentDS.observedParentClockPhaseChangeRate = 1100 * ptpClock->observedDrift;

			a = (ptpClock->offsetHistory[1]
				-2*ptpClock->offsetHistory[0]
				+ptpClock->currentDS.offsetFromMaster.nanoseconds);
			ptpClock->offsetHistory[1] = ptpClock->offsetHistory[0];
			ptpClock->offsetHistory[0] = ptpClock->currentDS.offsetFromMaster.nanoseconds;


			scaledLogVariance = order(a*a)<<8;
			filter(&scaledLogVariance, &ptpClock->slv_filt);
			ptpClock->parentDS.observedParentOffsetScaledLogVariance = 17000 + scaledLogVariance;
			DBGV("updateClock: observed scalled log variance: 0x%x\n", ptpClock->parentDS.observedParentOffsetScaledLogVariance);
    	}
    }

    switch (ptpClock->portDS.delayMechanism)
    {
    case E2E:
        DBG("updateClock: one-way delay averaged (E2E):  %10ds %11dns\n",
            ptpClock->currentDS.meanPathDelay.seconds, ptpClock->currentDS.meanPathDelay.nanoseconds);
        break;

    case P2P:
        DBG("updateClock: one-way delay averaged (P2P):  %10ds %11dns\n",
            ptpClock->portDS.peerMeanPathDelay.seconds, ptpClock->portDS.peerMeanPathDelay.nanoseconds);
        break;

    default:
        DBG("updateClock: one-way delay not computed\n");
    }

    DBG("updateClock: offset from master:      %10ds %11dns\n",

        ptpClock->currentDS.offsetFromMaster.seconds, ptpClock->currentDS.offsetFromMaster.nanoseconds);

    DBG("updateClock: observed drift:          %10d\n", ptpClock->observedDrift);

	char payload[MQTT_PUB_PAYLOAD_BUFFER_SIZE];
    switch (ptpClock->portDS.delayMechanism)
    {
    case E2E:
		#if 0
        sprintf(payload, "{ %s : \"offset from master:%10d s %11d ns, drift: %10d ns, path delay: %11d ns   %s\" }",
			PTPD_HEADER_MCU_DESC,
            ptpClock->currentDS.offsetFromMaster.seconds,
            ptpClock->currentDS.offsetFromMaster.nanoseconds,
            ptpClock->observedDrift,
            ptpClock->currentDS.meanPathDelay.nanoseconds, pbff);
		#else
		ptp_format_status_json(ptpClock, payload, sizeof(payload));
		#endif
//        printf("%s: offset from master:%10d s %11d ns, drift: %10d ns, path delay: %11d ns   %s\r\n",
//			PTPD_HEADER_MCU_DESC,
//            ptpClock->currentDS.offsetFromMaster.seconds,
//            ptpClock->currentDS.offsetFromMaster.nanoseconds,
//            ptpClock->observedDrift,
//            ptpClock->currentDS.meanPathDelay.nanoseconds, pbff);
		printf("{ %s : \"%10d s %11d ns, dly: %11d ns\" }\r\n",
			PTPD_HEADER_MCU_DESC,
            ptpClock->currentDS.offsetFromMaster.seconds,
            ptpClock->currentDS.offsetFromMaster.nanoseconds,
            ptpClock->currentDS.meanPathDelay.nanoseconds);
        break;

    case P2P:
        sprintf(payload, "{ %s : \"offset from master:%10d s %11d ns, drift: %10d ns, peer delay: %11d ns   %s\" }",
			PTPD_HEADER_MCU_DESC,
            ptpClock->currentDS.offsetFromMaster.seconds,
            ptpClock->currentDS.offsetFromMaster.nanoseconds,
            ptpClock->observedDrift,
            ptpClock->portDS.peerMeanPathDelay.nanoseconds, pbff);
//        printf("%s: offset from master:%10d s %11d ns, drift: %10d ns, peer delay: %11d ns   %s\r\n",
//			PTPD_HEADER_MCU_DESC,
//            ptpClock->currentDS.offsetFromMaster.seconds,
//            ptpClock->currentDS.offsetFromMaster.nanoseconds,
//            ptpClock->observedDrift,
//            ptpClock->portDS.peerMeanPathDelay.nanoseconds, pbff);
        break;

    default:
        sprintf(payload, "{ %s : \"offset from master:%10d s %11d ns, drift: %10d ns, delay: N/A   %s\" }",
			PTPD_HEADER_MCU_DESC,
            ptpClock->currentDS.offsetFromMaster.seconds,
            ptpClock->currentDS.offsetFromMaster.nanoseconds,
            ptpClock->observedDrift, pbff);
//        printf("%s: offset from master:%10d s %11d ns, drift: %10d ns, delay: N/A   %s\r\n",
//			PTPD_HEADER_MCU_DESC,
//            ptpClock->currentDS.offsetFromMaster.seconds,
//            ptpClock->currentDS.offsetFromMaster.nanoseconds,
//            ptpClock->observedDrift, pbff);
        break;
    }
	publish_topic(ptpd_pub_topic[OFFSET_INDEX].topic, ptpd_pub_topic[OFFSET_INDEX].qos, payload); //"/a1fwVnPM8wU/dm9051_mqttbks/f403a/ptpdaemon/offset"
}
