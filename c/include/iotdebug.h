/*
 * Copyright (c) 2011 People Power Company
 * All rights reserved.
 *
 * This open source code was developed with funding from People Power Company
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the People Power Corporation nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * PEOPLE POWER CO. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE
 */

#ifndef IOTDEBUG_H
#define IOTDEBUG_H

#include <syslog.h>

/** Dummy function for SYSLOG_* macros to use when we optimize out text */
static void __attribute__((unused)) dummySyslog(__const char *__fmt, ...) { }

#define SYSLOG_LEVEL_DEBUG 0
#define SYSLOG_LEVEL_INFO 1
#define SYSLOG_LEVEL_NOTICE 2
#define SYSLOG_LEVEL_WARNING 3
#define SYSLOG_LEVEL_ERR 4
#define SYSLOG_LEVEL_CRIT 5
#define SYSLOG_LEVEL_ALERT 6

#ifndef SYSLOG_LEVEL
#define SYSLOG_LEVEL SYSLOG_LEVEL_DEBUG
#endif


#if SYSLOG_LEVEL <= SYSLOG_LEVEL_ALERT
#define SYSLOG_ALERT(formatString, ...) syslog (LOG_ALERT, "%s(): "formatString, __FUNCTION__, ##__VA_ARGS__)
#else
#define SYSLOG_ALERT(formatString, ...) dummySyslog(""formatString, ##__VA_ARGS__);
#endif


#if SYSLOG_LEVEL <= SYSLOG_LEVEL_CRIT
#define SYSLOG_CRIT(formatString, ...) syslog (LOG_CRIT, "%s(): "formatString, __FUNCTION__, ##__VA_ARGS__)
#else
#define SYSLOG_CRIT(formatString, ...) dummySyslog(""formatString, ##__VA_ARGS__);
#endif

#if SYSLOG_LEVEL <= SYSLOG_LEVEL_ERR
#define SYSLOG_ERR(formatString, ...) syslog (LOG_ERR, "%s(): "formatString, __FUNCTION__, ##__VA_ARGS__)
#else
#define SYSLOG_ERR(formatString, ...) dummySyslog(""formatString, ##__VA_ARGS__);
#endif

#if SYSLOG_LEVEL <= SYSLOG_LEVEL_WARNING
#define SYSLOG_WARNING(formatString, ...) syslog (LOG_WARNING, "%s(): "formatString, __FUNCTION__, ##__VA_ARGS__)
#else
#define SYSLOG_WARNING(formatString, ...) dummySyslog(""formatString, ##__VA_ARGS__);
#endif

#if SYSLOG_LEVEL <= SYSLOG_LEVEL_NOTICE
#define SYSLOG_NOTICE(formatString, ...) syslog (LOG_NOTICE, "%s(): "formatString, __FUNCTION__, ##__VA_ARGS__)
#else
#define SYSLOG_NOTICE(formatString, ...) dummySyslog(""formatString, ##__VA_ARGS__);
#endif

#if SYSLOG_LEVEL <= SYSLOG_LEVEL_INFO
#define SYSLOG_INFO(formatString, ...) syslog (LOG_INFO, "%s(): "formatString, __FUNCTION__, ##__VA_ARGS__)
#else
#define SYSLOG_INFO(formatString, ...) dummySyslog(""formatString, ##__VA_ARGS__);
#endif

#if SYSLOG_LEVEL <= SYSLOG_LEVEL_DEBUG
#define SYSLOG_DEBUG(formatString, ...) syslog (LOG_DEBUG, "%s(): "formatString, __FUNCTION__, ##__VA_ARGS__)
#else
#define SYSLOG_DEBUG(formatString, ...) dummySyslog(""formatString, ##__VA_ARGS__);
#endif

#endif
