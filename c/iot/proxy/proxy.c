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
 * This file acts as the proxy between a server and the rest of the developer's
 * code.  It forms 2 types of connections with a cloud server:
 *
 *   1. Measurements are POST'd to the server as they are generated
 *   2. A GET channel is always open with the server to instantaneously receive
 *      commands from the server, through firewalls, etc.
 *
 * As a result of the persistent connection, this proxy must run in its own
 * thread.  Other clients in the system communicate with the proxy via
 * pipes.
 *
 * Another feature of this proxy is the ability to adapt to real-time user
 * interfaces. When a user is actively monitoring his UI, the cloud server is
 * able to detect this and also request continuous updates from this proxy and
 * its clients.
 *
 * We use libcurl (http://curl.haxx.se/libcurl/) as the client-side HTTP(S)
 * transfer library.
 *
 * @author John Teeter
 * @author Pradip De
 * @author Yvan Castilloux
 * @author David Moss
 */

#include <curl/curl.h>

#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <rpc/types.h>
#include <fcntl.h>

#include "libpipecomm.h"
#include "libhttpcomm.h"
#include "proxy.h"
#include "proxylisteners.h"
#include "proxyconfig.h"
#include "h2swrapper.h"
#include "eui64.h"
#include "ioterror.h"
#include "iotdebug.h"


/** Thread termination flag */
static bool gTerminate;

/** Thread */
static pthread_t sThreadId;

/** Thread attributes */
static pthread_attr_t sThreadAttr;

/** Mutex to protect router to server communications */
static pthread_mutex_t sProxyToServerMutex;

/** File descriptor to read from server */
static int sProxyToServerReadFd = -1;

/** File descriptor to write to server */
static int sProxyToServerWriteFd = -1;

/** Buffer allocating space for message(s) to send to the server */
static char sMsgToServer[PROXY_MAX_HTTP_SEND_MESSAGE_LEN];

/** Size of the message to send to the server */
static uint16_t sMsgToServerLen = 0;


/***************** Private Prototypes ***************/
static void *_serverCommThread(void *params);

static void _serverCommPush(CURLSH * curlHandle, char *message, char *response, int responseMaxLen);

static void _serverCommPoll(CURLSH * curlHandle, char *pollMsg, int pollMsgMaxLen);

int _httpProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);


/***************** Proxy Public ****************/
/**
 * Initialize and start the proxy thread
 * @param url The URL to communicate with
 * @return error_t
 */
error_t proxy_start(const char *url) {
	int pipeFds[2];

	proxyconfig_start();
	proxylisteners_start();
  pthread_mutex_init(&sProxyToServerMutex, NULL);

	if(proxyconfig_setUrl(url) != SUCCESS) {
	  SYSLOG_ERR("Couldn't set the URL");
	  return FAIL;
	}

	if(pipe(pipeFds)) {
		SYSLOG_ERR("pipe() 3, cause:(%s)", strerror(errno));
		return FAIL;
	}

	sProxyToServerReadFd = pipeFds[0];
	sProxyToServerWriteFd = pipeFds[1];

	// Make the read FD non-blocking so we can read until empty and keep going
	if (fcntl(sProxyToServerReadFd, F_SETFL, O_NONBLOCK) == -1) {
		SYSLOG_ERR("fcntl(sProxyToServerReadFd), %s", strerror(errno));
	}

	// Make the write FD non-blocking in case the FIFO becomes full
	if (fcntl(sProxyToServerWriteFd, F_SETFL, O_NONBLOCK) == -1) {
		SYSLOG_ERR("fcntl(sProxyToServerWriteFd), %s", strerror(errno));
	}

  curl_global_init(CURL_GLOBAL_ALL);

  // Initialize the thread
	pthread_attr_init(&sThreadAttr);

	// Will run detached from the main dispatcher thread
	pthread_attr_setdetachstate(&sThreadAttr, PTHREAD_CREATE_DETACHED);

	// Round robin schedule is fine when processing is extremely low
	pthread_attr_setschedpolicy(&sThreadAttr, SCHED_RR);

	// Create the thread
	if (pthread_create(&sThreadId, &sThreadAttr, &_serverCommThread, NULL)) {
		SYSLOG_ERR("Creating proxy thread failed: %s", strerror(errno));
		return FAIL;

	} else {
	  size_t stackSize = 0;
    pthread_attr_getstacksize(&sThreadAttr, &stackSize);

    if (stackSize < 65535) {
      // stack size must be at least 64K for curl to operate
      SYSLOG_WARNING("Stack size may be too small for curl: %u bytes", stackSize);
    }
  }

	return SUCCESS;
}

/**
 * Close all mutexes and threads
 */
void proxy_stop() {
  proxyconfig_stop();
  proxylisteners_stop();
  pthread_mutex_destroy(&sProxyToServerMutex);
  gTerminate = true;
}

/**
 * Add a listener to the messages sent by the server.  This is a convenience
 * function that simply forwards to the proxylisteners module.
 *
 * @param listener Function pointer to a function(const char *msg, int len)
 */
error_t proxy_addListener(proxylistener l) {
  return proxylisteners_addListener(l);
}

/**
 * Add a listener to the messages sent by the server.  This is a convenience
 * function that simply forwards to the proxylisteners module.
 *
 * @param listener Function pointer to a function(const char *msg, int len)
 */
error_t proxy_removeListener(proxylistener l) {
  return proxylisteners_removeListener(l);
}

/**
 * Use this function to send a message to the server.
 *
 * The process is to write the data into a pipe here, which is read out
 * in a different function and actually transmitted to the server later.
 * @param data Buffer of data to send
 * @param len Length of the data to send
 *
 * @return SUCCESS if the data is being sent to the server
 */
error_t proxy_send(const char *data, int len) {
  int bytesWritten = 0;

  if (len > 0) {
    pthread_mutex_lock(&sProxyToServerMutex);
    // Half-duplex pipe is protected for safety reasons on some deeply embedded platforms
    bytesWritten = libpipecomm_write(sProxyToServerWriteFd, data, len);
    pthread_mutex_unlock(&sProxyToServerMutex);
  }

  return SUCCESS;
}


/***************** Private Functions ****************/
/**
 * Main thread function for server communication
 */
static void *_serverCommThread(void *params) {
  char msgFromServer[PROXY_MAX_MSG_LEN];
  bool poll = true;
  int msgLen = 0;
  bool sentEmptyMsg = false;
  CURLSH *curlHandle = NULL; // curl handle shared across connections for DNS caching

  // Sleep briefly to obtain init messages from application
  sleep(5);

  // Initialize buffers, variables
  bzero(msgFromServer, sizeof(msgFromServer));
  bzero(sMsgToServer, sizeof(sMsgToServer));
  sMsgToServerLen = 0;

  // Initialize the shared curl library
  libhttpcomm_curlShareInit(curlHandle);

  // Main loop
  while (!gTerminate) {
    sentEmptyMsg = false;

    // Read until the pipe is empty or our buffer is full
    msgLen = 0;

    while (((sizeof(sMsgToServer) - sMsgToServerLen) >= PROXY_MAX_MSG_LEN)) {
      pthread_mutex_lock(&sProxyToServerMutex);
      msgLen = libpipecomm_read(sProxyToServerReadFd, sMsgToServer + sMsgToServerLen, sizeof(sMsgToServer) - sMsgToServerLen);
      pthread_mutex_unlock(&sProxyToServerMutex);

      if (msgLen > 0) {
        sMsgToServerLen += msgLen;

      } else {
        // pipe is empty or obtained an error reading it -> stop reading.
        break;
      }
      usleep(2000);
    }

    msgFromServer[0] = '\0';

    if (sMsgToServerLen > 0) {
      _serverCommPush(curlHandle, sMsgToServer, msgFromServer, sizeof(msgFromServer));
      bzero(sMsgToServer, sizeof(sMsgToServer));
      sMsgToServerLen = 0;

    } else if (poll == false) {

      /*
       * CONT, or "Continuous Mode", is signaled from the cloud server when the
       * detects a user is actively monitoring a UI.
       *
       * When in CONT mode, the router can't guarantee that a message will be
       * pushed often. Here we send an empty message if none are sent, so that
       * the server can send something to the UI. The persistent connection is
       * effectively disabled. This is important especially
       * when the use wants to control a device from the GUI and expects a quick
       * response from the system.
       */
      _serverCommPush(curlHandle, (char *) "", msgFromServer, sizeof(msgFromServer));
      sentEmptyMsg = true;
    }


    if (strlen(msgFromServer) > 0) {
      msgFromServer[sizeof(msgFromServer) - 1] = '\0';
      if (strstr(msgFromServer, "CONT") != NULL) {
        /*
         * When the server sends a CONT signal, it is telling the hub to close
         * the persistent connection (which is the GET connection) and start
         * POST'ing data often.
         */
        poll = false;

      } else if (strstr(msgFromServer, "ACK") != NULL) {
        /*
         * When sending an ACK message, the server is telling the hub to open
         * the persistent connection and only push when the connection times out
         * or when pushing data becomes a priority.
         */
        poll = true;
      }

      proxylisteners_broadcast(msgFromServer, strlen(msgFromServer));

      if (sentEmptyMsg == true) {
        sleep(1);
      }
    }

    // Dedicated GET connection
    if (poll == true) {
      msgFromServer[0] = '\0';
      // Only poll (GET) if the server wants you to.
      _serverCommPoll(curlHandle, msgFromServer, sizeof(msgFromServer));

      if (strlen(msgFromServer) > 0) {
        msgFromServer[sizeof(msgFromServer) - 1] = '\0';
        if (strstr(msgFromServer, "CONT") != NULL) {
          poll = false;

        } else if (strstr(msgFromServer, "ACK") != NULL) {
          poll = true;
        }

        proxylisteners_broadcast(msgFromServer, strlen(msgFromServer));
      }
    }
  }

  libhttpcomm_curlShareClose(curlHandle);
  SYSLOG_INFO("*** Exiting Proxy Thread ***");
  return NULL;
}


/**
 * Push message to the server
 *
 * @param curlHandle Curl handle shared across connection (mainly for DNS caching)
 * @param message Pointer to null-terminated message to send
 * @param messageLen length of the message
 * @param response pointer for storing message received by the server -> must exist
 * @param responseMaxLen max size of response in bytes.
 *
 * @return  none
 */
static void _serverCommPush(CURLSH *curlHandle, char *message, char *response, int responseMaxLen) {
  bool serverRetry = false;
  int wrappedMessageLen = 0;
  char wrappedMessage[PROXY_MAX_HTTP_SEND_MESSAGE_LEN];
  char url[PATH_MAX];
  int retries = 0;
  http_param_t params;

  assert(message);
  assert(response);

  bzero(wrappedMessage, sizeof(wrappedMessage));

  params.timeouts.connectTimeout = HTTPCOMM_DEFAULT_CONNECT_TIMEOUT_SEC;
  params.timeouts.transferTimeout = HTTPCOMM_DEFAULT_TRANSFER_TIMEOUT_SEC;
  params.verbose = false;

  do {
    if (serverRetry == true) {
      retries++;
      sleep(1);
    }

    wrappedMessageLen = h2swrapper_wrap(wrappedMessage, message, sizeof(wrappedMessage));

    SYSLOG_DEBUG("Wrapped: %s", wrappedMessage);

    proxyconfig_getUrl(url, sizeof(url));

    SYSLOG_DEBUG("POST URL: %s", url);

    if (libhttpcomm_sendMsg(curlHandle, CURLOPT_POST, url,
        proxyconfig_getCertificate(), proxyconfig_getActivationToken(), wrappedMessage,
        wrappedMessageLen, response, responseMaxLen, params, NULL) == SUCCESS) {

       serverRetry = (strlen(response) == 0) || (strstr(response, "ERR") != NULL);

       if(!serverRetry) {
         SYSLOG_DEBUG("Send to server SUCCESS");
       } else {
         SYSLOG_DEBUG("Error sending to server: %s", response);
       }

    } else {
      // Either the Internet or the server is down
      // If the Internet is down, buffer messages and do not lose data
      SYSLOG_DEBUG("Couldn't contact the server");
      retries = 0;
      serverRetry = true;
    }

  } while (serverRetry == true && retries < PROXY_MAX_HTTP_RETRIES);


}

/**
 * @brief   Polls server for new messages
 *
 * @param   curlHandle: curl handle shared across connections
 * @param   pollMsg: ptr for storing message received by the server -> must exist
 * @param   pollMsgMaxLen: max size of pollMsg in bytes.
 *
 * @return  none
 */
static void _serverCommPoll(CURLSH * curlHandle, char *pollMsg, int pollMsgMaxLen) {
  int urlOffset = 0;
  char url[PATH_MAX];
  char tempUrl[PATH_MAX];
  char localAddress[EUI64_STRING_SIZE];
  http_param_t params;

  eui64_toString(localAddress, sizeof(localAddress));

  proxyconfig_getUrl(tempUrl, sizeof(tempUrl));

  params.timeouts.connectTimeout = HTTPCOMM_DEFAULT_CONNECT_TIMEOUT_SEC;
  params.timeouts.transferTimeout = proxyconfig_getUploadIntervalSec();
  params.verbose = false;

  snprintf(url + urlOffset, sizeof(url) - urlOffset, "%s?id=%s&timeout=%lu",
      tempUrl, localAddress, params.timeouts.transferTimeout);

  // 30-second buffer to let server notify the timeout
  params.timeouts.transferTimeout += 30;

  SYSLOG_DEBUG("GET URL: %s", url);

  if (libhttpcomm_sendMsg(curlHandle, CURLOPT_HTTPGET, url,
      proxyconfig_getCertificate(), proxyconfig_getActivationToken(), NULL, 0, pollMsg, pollMsgMaxLen,
      params, _httpProgressCallback) == false) {
    sleep(1);
  }
}


/**
 * Monitors whether the push pipe is getting full.  If it is, we stop the GET
 * operation and starting pushing.
 *
 * @param See Curl progress callback documentation
 *
 * @return True to close the connection, false to take no action and keep it open
 */
int _httpProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
  int msgLen = 0;
  struct timeval curTime;
  static unsigned int lastTimeoutTime = 0;
  static int count = 0;

  // Periodic "Connected to server" notifications...
  if ((count++) >= PROXY_NUM_SERVER_CONNECTIONS_BEFORE_SYSLOG_NOTIFICATION) {
    count = 0;
    SYSLOG_DEBUG("Connected to server");
  }

  while (((sizeof(sMsgToServer) - sMsgToServerLen) >= PROXY_MAX_MSG_LEN)) {
    pthread_mutex_lock(&sProxyToServerMutex);
    msgLen = libpipecomm_read(sProxyToServerReadFd, sMsgToServer + sMsgToServerLen, sizeof(sMsgToServer) - sMsgToServerLen);
    pthread_mutex_unlock(&sProxyToServerMutex);

    if (msgLen > 0) {
      sMsgToServerLen += msgLen;
      SYSLOG_DEBUG("Read %d bytes", msgLen);
      SYSLOG_DEBUG("Message to the server length = %d bytes", sMsgToServerLen);

    } else {
      // Pipe is empty or had an error reading it... stop reading.
      break;
    }
    usleep(2000);
  }

  gettimeofday(&curTime, NULL);

  if ((sizeof(sMsgToServer) - sMsgToServerLen) < PROXY_MAX_MSG_LEN) {
    if (((unsigned int) curTime.tv_sec - lastTimeoutTime) > HTTPCOMM_DEFAULT_CONNECT_TIMEOUT_SEC) {
      lastTimeoutTime = (unsigned int) curTime.tv_sec;
      SYSLOG_DEBUG("Pipe is getting full -> need to push the data to the server");
      return true; // return non-zero to close the GET connection and start pushing the data
    } else {
      SYSLOG_WARNING("Waiting for minimum timeout of %u sec to occur", (unsigned int) HTTPCOMM_DEFAULT_CONNECT_TIMEOUT_SEC);
      return false;
    }
  }

  // keep the GET connection until the timeout occurs or the push buffer is full.
  return false;
}
