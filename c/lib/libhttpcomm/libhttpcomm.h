/*
 * Copyright (c) 2011 People Power Co.
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

#ifndef LIBHTTPCOMM_H
#define LIBHTTPCOMM_H

#include <curl/curl.h>
#include <limits.h>
#include <rpc/types.h>
#include <stdbool.h>

/** Maximum time for an HTTP connection, including name resolving */
#define HTTPCOMM_DEFAULT_CONNECT_TIMEOUT_SEC 30

/** Maximum time for an HTTP transfer */
#define HTTPCOMM_DEFAULT_TRANSFER_TIMEOUT_SEC 60

/** Maximum amount of time before resolving the name again */
#define HTTPCOMM_DEFAULT_DNS_CACHING_TIMEOUT_SEC 120

/** Size of a buffer needed to hold a string describing a port */
#define HTTPCOMM_PORT_STRING_SIZE 6

/** Size of a string needed to hold "true" or "false" for SSL */
#define HTTPCOMM_SSL_STRING_SIZE 6

/** Size of a string needed to hold a authentication token */
#define HTTPCOMM_AUTHENTICATION_STRING_SIZE 128


typedef struct http_timeout_t {
  long connectTimeout;
  long transferTimeout;
} http_timeout_t;


typedef struct http_param_t {
  http_timeout_t timeouts;
  bool verbose;
} http_param_t;


/***************** Public Prototypes *****************/
void libhttpcomm_curlShareInit(CURLSH *curlHandle);

void libhttpcomm_curlShareClose(CURLSH *curHandle);

int libhttpcomm_getMsg(CURLSH * shareCurlHandle, const char *url,
    const char *sslCertPath, const char *authToken, char *rxBuffer,
    int maxRxBufferSize, http_param_t params, int(*ProgressCallback)(
        void *clientp, double dltotal, double dlnow, double ultotal,
        double ulnow));

int libhttpcomm_sendMsg(CURLSH * shareCurlHandle, CURLoption httpMethod,
    const char *url, const char *sslCertPath, const char *authToken,
    char *msgToSendPtr, int msgToSendSize, char *rxBuffer, int maxRxBufferSize,
    http_param_t params, int(*ProgressCallback)(void *clientp, double dltotal,
        double dlnow, double ultotal, double ulnow));

int libhttpcomm_getFile(CURLSH * shareCurlHandle, const char *url,
    const char *sslCertPath, const char *authToken, FILE *rxfile,
    int maxRxFileSize, http_timeout_t timeouts, int(*ProgressCallback)(
        void *clientp, double dltotal, double dlnow, double ultotal,
        double ulnow));

int libhttpcomm_sendFile(const char *url, const char *sslCertPath,
    const char *authToken, char *fileName, char *rxBuffer, int maxRxBufferSize,
    http_timeout_t timeouts);

#endif
