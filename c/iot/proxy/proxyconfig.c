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

/**
 * This module manages configuration information for the proxy
 * @author David Moss
 */


#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <rpc/types.h>

#include "proxyconfig.h"
#include "ioterror.h"
#include "iotdebug.h"

/** Mutex to protect the command URL access */
static pthread_mutex_t sUrlMutex;

/** Mutex to protect upload interval access */
static pthread_mutex_t sUploadIntervalMutex;

/** Mutex to protect SSL flag access */
static pthread_mutex_t sUseSslMutex;

/** Mutex to protect certificate path */
static pthread_mutex_t sCertificatePathMutex;

/** Mutex to protect activation token */
static pthread_mutex_t sActivationTokenMutex;

/** Upload interval in seconds */
static long sUploadIntervalSec = PROXY_DEFAULT_UPLOAD_INTERVAL_SEC;

/** Server URL */
static char sUrl[PROXY_URL_SIZE];

/** Flag to use SSL for server connections */
static bool sUseSsl = false;

/** SSL certificate path */
static char sCertificatePath[PATH_MAX];

/** Cloud activation token */
static char sActivationToken[PROXY_MAX_ACTIVATION_TOKEN_SIZE];



/***************** Proxyconfig Public ****************/
/**
 * Start proxyconfig by initializing mutexes
 */
void proxyconfig_start() {
  pthread_mutex_init(&sUploadIntervalMutex, NULL);
  pthread_mutex_init(&sUrlMutex, NULL);
  pthread_mutex_init(&sUseSslMutex, NULL);
  pthread_mutex_init(&sCertificatePathMutex, NULL);
  pthread_mutex_init(&sActivationTokenMutex, NULL);
}

/**
 * Stop proxyconfig by destroying mutexes
 */
void proxyconfig_stop() {
  pthread_mutex_destroy(&sUploadIntervalMutex);
  pthread_mutex_destroy(&sUrlMutex);
  pthread_mutex_destroy(&sUseSslMutex);
  pthread_mutex_destroy(&sCertificatePathMutex);
  pthread_mutex_destroy(&sActivationTokenMutex);
}


/**
 * Get the upload interval in seconds
 * @return the upload interval in seconds
 */
long proxyconfig_getUploadIntervalSec() {
  long uploadInterval;

  // Must be protected since multiple threads are accessing it
  pthread_mutex_lock(&sUploadIntervalMutex);
  uploadInterval = sUploadIntervalSec;
  pthread_mutex_unlock(&sUploadIntervalMutex);

  if(uploadInterval == 0) {
    return PROXY_DEFAULT_UPLOAD_INTERVAL_SEC;
  }

  return uploadInterval;
}

/**
 * Set the upload interval in seconds
 * @param uploadIntervalSec
 */
void proxyconfig_setUploadIntervalSec(long uploadIntervalSec) {
  if(uploadIntervalSec == 0) {
    return;
  }

  pthread_mutex_lock(&sUploadIntervalMutex);
  sUploadIntervalSec = uploadIntervalSec;
  pthread_mutex_unlock(&sUploadIntervalMutex);
  SYSLOG_DEBUG("Upload interval set to %ld", uploadIntervalSec);
}

/**
 * Get the server URL
 * @param dest Buffer in which the URL will be stored
 * @param destLen Maximum size of the buffer
 */
void proxyconfig_getUrl(char *dest, int destLen) {
  int bytesWritten = 0;

  assert(dest);

  memset(dest, 0x0, destLen);

  // Must be protected since multiple threads are accessing it
  pthread_mutex_lock(&sUrlMutex);

  // Ensure we have an http(s)://
  if (strstr(sUrl, "http") == NULL) {
    if (sUseSsl == true && access(sCertificatePath, F_OK) == 0) {
      bytesWritten += snprintf(dest, destLen, "https://");
    } else {
      bytesWritten += snprintf(dest, destLen, "http://");
    }
  }

  strncpy(dest + bytesWritten, sUrl, destLen - bytesWritten);

  pthread_mutex_unlock(&sUrlMutex);
}

/**
 * Set the server URL
 * @param url The desired server URL
 * @return SUCCESS if the URL is set, FAIL if the URL is invalid
 */
error_t proxyconfig_setUrl(const char *url) {
  assert(url);

  if (*url) {
    // Must be protected since multiple threads are accessing it
    pthread_mutex_lock(&sUrlMutex);
    strncpy(sUrl, url, sizeof(sUrl));
    pthread_mutex_unlock(&sUrlMutex);
    SYSLOG_DEBUG("Server URL set to %s", url);
    return SUCCESS;
  }

  SYSLOG_DEBUG("URL is empty");
  return FAIL;
}

/**
 * Set the certificate path
 * @param certificatePath Pointer to a string containing the certificate path
 */
void proxyconfig_setCertificate(const char *certificatePath) {
  pthread_mutex_lock(&sCertificatePathMutex);
  strncpy(sCertificatePath, certificatePath, sizeof(sCertificatePath));
  pthread_mutex_unlock(&sCertificatePathMutex);

  SYSLOG_DEBUG("SSL certificate path set to %s", certificatePath);
}


/**
 * @return a pointer to the certificate path if we are using SSL, else NULL
 */
const char *proxyconfig_getCertificate() {
  const char *certificate = NULL;

  if (proxyconfig_getSsl() == true) {
    pthread_mutex_lock(&sCertificatePathMutex);
    if (access(sCertificatePath, F_OK) == 0) {
      certificate = sCertificatePath;
    }
    pthread_mutex_unlock(&sCertificatePathMutex);
  }

  return certificate;
}

/**
 * @return a pointer to the activation token
 */
const char *proxyconfig_getActivationToken() {
  const char *activation = NULL;

  pthread_mutex_lock(&sActivationTokenMutex);
  activation = sActivationToken;
  pthread_mutex_unlock(&sActivationTokenMutex);

  return activation;
}

/**
 * Set the activation token
 * @param token Pointer to a string containing the activation token
 */
void proxyconfig_setActivationToken(const char *token) {
  pthread_mutex_lock(&sActivationTokenMutex);
  strncpy(sActivationToken, token, sizeof(sActivationToken));
  pthread_mutex_unlock(&sActivationTokenMutex);
  SYSLOG_DEBUG("Activation token set to %s", token);
}

/**
 * @param ssl True to use SSL
 */
void proxyconfig_setSsl(bool ssl) {
  pthread_mutex_lock(&sUseSslMutex);
  sUseSsl = ssl;
  pthread_mutex_unlock(&sUseSslMutex);
  SYSLOG_DEBUG("Use SSL set to %d", ssl);
}

/**
 * @return True if we are to use SSL
 */
bool proxyconfig_getSsl() {
  bool ssl;

  pthread_mutex_lock(&sUseSslMutex);
  ssl = sUseSsl;
  pthread_mutex_unlock(&sUseSslMutex);

  return ssl;
}

