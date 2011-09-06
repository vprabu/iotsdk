/*
 * Copyright (c) 2010 People Power Company
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
 * This module contains all the functionality needed to start the proxy
 * @author David Moss
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "libconfigio.h"

#include "ioterror.h"
#include "iotdebug.h"
#include "proxymanager.h"
#include "proxy.h"
#include "proxyserver.h"
#include "proxyconfig.h"
#include "proxycli.h"

/**************** Private Prototypes ****************/
char *_proxymanager_getUrlFromConfigFile(char *url, int maxsize);

char *_proxymanager_getProxyActivationKeyFromConfigFile(char *buffer, int maxsize);

bool _proxymanager_useSslFromConfigFile();

char *_proxymanager_getProxySslCertificateFromConfigFile(char *buffer, int maxsize);


/**************** Public Functions ****************/
/**
 * We extract configuration values from the configuration file and use those
 * values to start the proxy which connects to the cloud services
 */
void proxymanager_startProxy() {
  char buffer[PROXY_URL_SIZE];

  // Pull the activation key from the config file
  proxyconfig_setActivationToken(_proxymanager_getProxyActivationKeyFromConfigFile(buffer, sizeof(buffer)));

  // Set SSL to true or false
  proxyconfig_setSsl(_proxymanager_useSslFromConfigFile());

  // Set the SSL certificate path, which may or may not exist
  proxyconfig_setCertificate(_proxymanager_getProxySslCertificateFromConfigFile(buffer, sizeof(buffer)));

  // Start the proxy with our URL
  proxy_start(_proxymanager_getUrlFromConfigFile(buffer, sizeof(buffer)));

}

/**************** Private Functions ****************/

/**
 * Read the configuration file to extract the information that makes up the
 * URL of the server.  This is also the only function I've ever written
 * in my life where it made sense to use "goto".
 *
 * @param url Destination buffer for the URL
 * @param maxsize Maximum size of the URL buffer
 * @return the URL
 */
char * _proxymanager_getUrlFromConfigFile(char *url, int maxsize) {
  bzero(url, maxsize);

  if(libconfigio_read(proxycli_getConfigFilename(), CONFIGIO_CLOUD_HOST, url + strlen(url), maxsize - strlen(url)) == -1) {
    goto fail;
  }

  snprintf(url + strlen(url), maxsize - strlen(url), ":");

  if(libconfigio_read(proxycli_getConfigFilename(), CONFIGIO_CLOUD_PORT, url + strlen(url), maxsize - strlen(url)) == -1) {
    goto fail;
  }

  snprintf(url + strlen(url), maxsize - strlen(url), "/");

  if(libconfigio_read(proxycli_getConfigFilename(), CONFIGIO_CLOUD_URI, url + strlen(url), maxsize - strlen(url)) == -1) {
    goto fail;
  }

  return url;

  fail:
      SYSLOG_ERR("Couldn't read from config file %s. You need to activate your proxy.", proxycli_getConfigFilename());
      printf("Couldn't read from config file %s. You need to activate your proxy.\n", proxycli_getConfigFilename());
      strncpy(url, DEFAULT_PROXY_URL, maxsize);
      return url;
}


/**
 * Get the activation key from our configuration file
 * @param buffer Buffer to store the activation key in
 * @param maxsize Maximum size of the buffer
 * @return The buffer
 */
char *_proxymanager_getProxyActivationKeyFromConfigFile(char *buffer, int maxsize) {
  bzero(buffer, maxsize);
  if(libconfigio_read(proxycli_getConfigFilename(), CONFIGIO_CLOUD_ACTIVATION_KEY, buffer, maxsize) == -1) {
    SYSLOG_ERR("Please activate your proxy with the [-a (key)] option");
    printf("Please activate your proxy with the [-a (key)] option\n");
  }
  return buffer;
}

/**
 * Determine whether or not to use SSL from the configuration file
 * @return True if we are to use SSL, and an SSL certificate will also be needed
 */
bool _proxymanager_useSslFromConfigFile() {
  char buffer[8];
  int i;

  libconfigio_read(proxycli_getConfigFilename(), CONFIGIO_CLOUD_USE_SSL, buffer, sizeof(buffer));

  for(i = 0; buffer[i]; i++) {
    buffer[i] = tolower(buffer[i]);
  }

  return (strcmp(buffer, "true") == 0);
}

/**
 * Get the path to the SSL certificate from our configuration file
 * @param buffer Buffer to store the path in
 * @param maxsize Maximum size of the buffer
 * @return The buffer
 */
char *_proxymanager_getProxySslCertificateFromConfigFile(char *buffer, int maxsize) {
  bzero(buffer, maxsize);
  libconfigio_read(proxycli_getConfigFilename(), CONFIGIO_PROXY_SSL_CERTIFICATE_PATH, buffer, maxsize);
  return buffer;
}



