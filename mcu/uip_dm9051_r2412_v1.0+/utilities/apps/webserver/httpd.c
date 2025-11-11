/**
 * \addtogroup apps
 * @{
 */

/**
 * \defgroup httpd Web server
 * @{
 * The uIP web server is a very simplistic implementation of an HTTP
 * server. It can serve web pages and files from a read-only ROM
 * filesystem, and provides a very small scripting language.

 */

/**
 * \file
 *         Web server
 * \author
 *         Adam Dunkels <adam@sics.se>
 */


/*
 * Copyright (c) 2004, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: httpd.c,v 1.2 2006/06/11 21:46:38 adam Exp $
 */

#include "uip.h"
#include "httpd.h"
#include "httpd-fs.h"
#include "httpd-cgi.h"
#include "http-strings.h"

#include <string.h>
#include "web_led.h"
#include <string.h>
#include <stdio.h>
#define STATE_WAITING 0
#define STATE_OUTPUT  1

#define ISO_nl      0x0a
#define ISO_space   0x20
#define ISO_bang    0x21
#define ISO_percent 0x25
#define ISO_period  0x2e
#define ISO_slash   0x2f
#define ISO_colon   0x3a
#define ISO_question    0x3f

//#include "sl_simple_led.h"
//#include "sl_simple_led_instances.h"
#include "RttPrintf.h"

#ifndef LED1_INSTANCE
#define LED1_INSTANCE               sl_led_led1
#endif

extern int Web_LED_FLASH;

/*---------------------------------------------------------------------------*/
static unsigned short generate_part_of_file(void *state)
{
    struct httpd_state *s = (struct httpd_state *)state;

    if (s->file.len > uip_mss())
    {
        s->len = uip_mss();
    }
    else
    {
        s->len = s->file.len;
    }

    memcpy(uip_appdata, s->file.data, s->len);

    return s->len;
}
/*---------------------------------------------------------------------------*/
static PT_THREAD(send_file(struct httpd_state *s))
{
    PSOCK_BEGIN(&s->sout);

    do
    {
        PSOCK_GENERATOR_SEND(&s->sout, generate_part_of_file, s);
        s->file.len -= s->len;
        s->file.data += s->len;
    } while (s->file.len > 0);

    PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static PT_THREAD(send_part_of_file(struct httpd_state *s))
{
    PSOCK_BEGIN(&s->sout);

    PSOCK_SEND(&s->sout, s->file.data, s->len);

    PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static PT_THREAD(send_led_file(struct httpd_state *s))
{
    char tbuf[18];

    PSOCK_BEGIN(&s->sout);

    PSOCK_SEND_STR(&s->sout, "<html><body bgcolor=\"white\"><center>DAVICOM & JLINK M08G16 Ethernet Demo<form method=\"get\" action=\" \"><table width=\"600\" border=\"0\">");
    //  PSOCK_SEND_STR(&s->sout, "<table width=\"600\" border=\"0\">");
    PSOCK_SEND_STR(&s->sout, "<tr><td>Chip: </td><td>DM9051(DAVICOM)</td></tr><tr><td>MAC Address: </td><td>");
    //  PSOCK_SEND_STR(&s->sout, "<tr><td>MAC Address: </td><td>");
    sprintf(tbuf, "%02x:%02x:%02x:%02x:%02x:%02x</td></tr>", (unsigned int)uip_ethaddr.addr[0], (unsigned int)uip_ethaddr.addr[1], \
            (unsigned int)uip_ethaddr.addr[2], (unsigned int)uip_ethaddr.addr[3], (unsigned int)uip_ethaddr.addr[4], (unsigned int)uip_ethaddr.addr[5]);
    PSOCK_SEND_STR(&s->sout, tbuf);
    PSOCK_SEND_STR(&s->sout, "<tr><td>IP: </td><td>");
    sprintf(tbuf, "%d.%d.%d.%d</td></tr>", (uip_hostaddr[0] & 0xff), ((uip_hostaddr[0] >> 8) & 0xff), \
            (uip_hostaddr[1] & 0xff), ((uip_hostaddr[1] >> 8) & 0xff));
    PSOCK_SEND_STR(&s->sout, tbuf);
    PSOCK_SEND_STR(&s->sout, "<tr><td>Gateway: </td><td>");
    sprintf(tbuf, "%d.%d.%d.%d</td></tr>", (uip_draddr[0] & 0xff), ((uip_draddr[0] >> 8) & 0xff), \
            (uip_draddr[1] & 0xff), ((uip_draddr[1] >> 8) & 0xff));
    PSOCK_SEND_STR(&s->sout, tbuf);
    PSOCK_SEND_STR(&s->sout, "<tr><td>MASK: </td><td>");
    sprintf(tbuf, "%d.%d.%d.%d</td></tr>", (uip_netmask[0] & 0xff), ((uip_netmask[0] >> 8) & 0xff), \
            (uip_netmask[1] & 0xff), ((uip_netmask[1] >> 8) & 0xff));
    PSOCK_SEND_STR(&s->sout, tbuf);
    PSOCK_SEND_STR(&s->sout, "<tr><td>LED light</td><td><input type=\"radio\" name=\"switch\" value=1 ");

    if (Web_LED_FLASH)
    {
        PSOCK_SEND_STR(&s->sout, "checked />LED ON");
    }
    else
    {
        PSOCK_SEND_STR(&s->sout, " />LED ON");
    }

    PSOCK_SEND_STR(&s->sout, "<input type=\"radio\" name=\"switch\" value=0 ");

    if (Web_LED_FLASH)
    {
        PSOCK_SEND_STR(&s->sout, " />LED OFF");
    }
    else
    {
        PSOCK_SEND_STR(&s->sout, "checked />LED OFF");
    }

		//  PSOCK_SEND_STR(&s->sout, "<input type=\"submit\" value=\"�T�w\" /></td></tr>");
    PSOCK_SEND_STR(&s->sout, "<input type=\"submit\" value=\"Confirm\" /></td></tr>");
		//  PSOCK_SEND_STR(&s->sout, "</table></form></center></body></html>");

    PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static void next_scriptstate(struct httpd_state *s)
{
    char *p;
    p = strchr(s->scriptptr, ISO_nl) + 1;
    s->scriptlen -= (unsigned short)(p - s->scriptptr);
    s->scriptptr = p;
}
/*---------------------------------------------------------------------------*/
static PT_THREAD(handle_script(struct httpd_state *s))
{
    char *ptr;

    PT_BEGIN(&s->scriptpt);


    while (s->file.len > 0)
    {

        /* Check if we should start executing a script. */
        if (*s->file.data == ISO_percent &&
                *(s->file.data + 1) == ISO_bang)
        {
            s->scriptptr = s->file.data + 3;
            s->scriptlen = s->file.len - 3;

            if (*(s->scriptptr - 1) == ISO_colon)
            {
                httpd_fs_open(s->scriptptr + 1, &s->file);
                PT_WAIT_THREAD(&s->scriptpt, send_file(s));
            }
            else
            {
                PT_WAIT_THREAD(&s->scriptpt,
                               httpd_cgi(s->scriptptr)(s, s->scriptptr));
            }

            next_scriptstate(s);

            /* The script is over, so we reset the pointers and continue
            sending the rest of the file. */
            s->file.data = s->scriptptr;
            s->file.len = s->scriptlen;
        }
        else
        {
            /* See if we find the start of script marker in the block of HTML
            to be sent. */

            if (s->file.len > uip_mss())
            {
                s->len = uip_mss();
            }
            else
            {
                s->len = s->file.len;
            }

            if (*s->file.data == ISO_percent)
            {
                ptr = strchr(s->file.data + 1, ISO_percent);
            }
            else
            {
                ptr = strchr(s->file.data, ISO_percent);
            }

            if (ptr != NULL &&
                    ptr != s->file.data)
            {
                s->len = (int)(ptr - s->file.data);

                if (s->len >= uip_mss())
                {
                    s->len = uip_mss();
                }
            }

            PT_WAIT_THREAD(&s->scriptpt, send_part_of_file(s));
            s->file.data += s->len;
            s->file.len -= s->len;

        }
    }

    PT_END(&s->scriptpt);
}
/*---------------------------------------------------------------------------*/
static PT_THREAD(send_headers(struct httpd_state *s, const char *statushdr))
{
    char *ptr;

    PSOCK_BEGIN(&s->sout);

    PSOCK_SEND_STR(&s->sout, statushdr);

    ptr = strrchr(s->filename, ISO_period);

    if (ptr == NULL)
    {
        PSOCK_SEND_STR(&s->sout, http_content_type_binary);
    }
    else if (strncmp(http_html, ptr, 5) == 0 ||
             strncmp(http_shtml, ptr, 6) == 0)
    {
        PSOCK_SEND_STR(&s->sout, http_content_type_html);
    }
    else if (strncmp(http_css, ptr, 4) == 0)
    {
        PSOCK_SEND_STR(&s->sout, http_content_type_css);
    }
    else if (strncmp(http_png, ptr, 4) == 0)
    {
        PSOCK_SEND_STR(&s->sout, http_content_type_png);
    }
    else if (strncmp(http_gif, ptr, 4) == 0)
    {
        PSOCK_SEND_STR(&s->sout, http_content_type_gif);
    }
    else if (strncmp(http_jpg, ptr, 4) == 0)
    {
        PSOCK_SEND_STR(&s->sout, http_content_type_jpg);
    }
    else
    {
        PSOCK_SEND_STR(&s->sout, http_content_type_plain);
    }

    PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static PT_THREAD(handle_output(struct httpd_state *s))
{
    char *ptr;

    PT_BEGIN(&s->outputpt);

    if (strncmp(s->filename, "/ledtest.html", 13) == 0)
    {
        //data_ledtest_html(LED_flag, &s->file);
        PT_WAIT_THREAD(&s->outputpt,
                       send_headers(s,
                                    http_header_200));
        PT_WAIT_THREAD(&s->outputpt,
                       send_led_file(s));
        goto output_done;
    }
    if (!httpd_fs_open(s->filename, &s->file))
    {
        httpd_fs_open(http_404_html, &s->file);
        strcpy(s->filename, http_404_html);
        PT_WAIT_THREAD(&s->outputpt,
                       send_headers(s,
                                    http_header_404));
        PT_WAIT_THREAD(&s->outputpt,
                       send_file(s));
    }
    else
    {
        PT_WAIT_THREAD(&s->outputpt,
                       send_headers(s,
                                    http_header_200));
        ptr = strchr(s->filename, ISO_period);

        if (ptr != NULL && strncmp(ptr, http_shtml, 6) == 0)
        {
            PT_INIT(&s->scriptpt);
            PT_WAIT_THREAD(&s->outputpt, handle_script(s));
        }
        else
        {
            PT_WAIT_THREAD(&s->outputpt,
                           send_file(s));
        }
    }

output_done:
    PSOCK_CLOSE(&s->sout);
    PT_END(&s->outputpt);
}
/*---------------------------------------------------------------------------*/
static PT_THREAD(handle_input(struct httpd_state *s))
{
    PSOCK_BEGIN(&s->sin);

    PSOCK_READTO(&s->sin, ISO_space);

    if (strncmp(s->inputbuf, http_get, 4) != 0)
    {
        PSOCK_CLOSE_EXIT(&s->sin);
    }

    PSOCK_READTO(&s->sin, ISO_space);

    if (s->inputbuf[0] != ISO_slash)
    {
        PSOCK_CLOSE_EXIT(&s->sin);
    }

    /* Use ledtest.html replace index.html, if "/". */
    /* If you want index.html, you can key in "/index.html". */
    if ((s->inputbuf[1] == ISO_space) || (s->inputbuf[1] == ISO_question))
    {
        //strncpy(s->filename, http_index_html, sizeof(s->filename));
        strncpy(s->filename, "/ledtest.html", 14);
#if 1

        if (strncmp(&s->inputbuf[1], "?switch", 7) == 0)
        {
            if (s->inputbuf[9] == '1')
            {
                // LEN ON
                //         DrvGPIO_ClrBit (E_PORT4, E_PIN5);       // Turn ON
//                sl_led_turn_on(&LED1_INSTANCE);
                printf("LED ON \r\n");
                Web_LED_FLASH = 1;

            }
            else
            {
                //LED OFF
                //          DrvGPIO_SetBit (E_PORT4, E_PIN5);       //Turn Off LED
                Web_LED_FLASH = 0;
//                sl_led_turn_off(&LED1_INSTANCE);
                printf("LED OFF \r\n");
            }
        }

#endif  //0    
    }
    else
    {
        s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
        strncpy(s->filename, &s->inputbuf[0], sizeof(s->filename));
    }

    /*  httpd_log_file(uip_conn->ripaddr, s->filename);*/

    s->state = STATE_OUTPUT;

    while (1)
    {
        PSOCK_READTO(&s->sin, ISO_nl);

        if (strncmp(s->inputbuf, http_referer, 8) == 0)
        {
            s->inputbuf[PSOCK_DATALEN(&s->sin) - 2] = 0;
            /*      httpd_log(&s->inputbuf[9]);*/
        }
    }

    PSOCK_END(&s->sin);
}
/*---------------------------------------------------------------------------*/
static void handle_connection(struct httpd_state *s)
{
    handle_input(s);

    if (s->state == STATE_OUTPUT)
    {
        handle_output(s);
    }
}
/*---------------------------------------------------------------------------*/
void httpd_appcall(void)
{
    struct httpd_state *s = (struct httpd_state *) & (uip_conn->appstate);

    if (uip_closed() || uip_aborted() || uip_timedout())
    {
    }
    else if (uip_connected())
    {
        PSOCK_INIT(&s->sin, s->inputbuf, sizeof(s->inputbuf) - 1);
        PSOCK_INIT(&s->sout, s->inputbuf, sizeof(s->inputbuf) - 1);
        PT_INIT(&s->outputpt);
        s->state = STATE_WAITING;
        /*    timer_set(&s->timer, CLOCK_SECOND * 100);*/
        s->timer = 0;
        handle_connection(s);
    }
    else if (s != NULL)
    {
        if (uip_poll())
        {
            ++s->timer;

            if (s->timer >= 20)
            {
                uip_abort();
            }
        }
        else
        {
            s->timer = 0;
        }

        handle_connection(s);
    }
    else
    {
        uip_abort();
    }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Initialize the web server
 *
 *             This function initializes the web server and should be
 *             called at system boot-up.
 */
void httpd_init(void)
{
    uip_listen(HTONS(80));
}
/*---------------------------------------------------------------------------*/
/** @} */
