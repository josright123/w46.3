/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

/* original
 */
#if 0
#endif

/* find some thing
 */
#if 0
//... #define LWIP_PLATFORM_ASSERT(x) do {printf("Assertion \"%s\" failed at line %d in %s\n", \
//...                                     x, __LINE__, __FILE__); } while(0)
#endif

/* new add use
 */
#if 1
//typedef int sys_prot_t;

//#define LWIP_RAND() ((u32_t)rand())

/** Platform specific assertion handling.\n
 * Note the default implementation pulls in printf, fflush and abort, which may
 * in turn pull in a lot of standard libary code. In resource-constrained
 * systems, this should be defined to something less resource-consuming.
 */
#define LWIP_PLATFORM_ASSERT(x) do {printf("      Assertion \"%s\" failed at line %d in %s\r\n", \
                                     x, __LINE__, __FILE__); /*fflush(NULL);*/ /* abort();*/} while(0)
#endif

/* Keil MDK-ARM compiler struct packing */
#define PACK_STRUCT_BEGIN __packed

#define PACK_STRUCT_STRUCT 
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_FLD_8(x) PACK_STRUCT_FIELD(x)

#endif /* LWIP_ARCH_CC_H */
