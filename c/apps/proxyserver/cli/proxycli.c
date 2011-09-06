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
 * Command line Interface for the proxy server
 * @author David Moss
 */

#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "proxycli.h"
#include "proxyserver.h"
#include "ioterror.h"
#include "iotdebug.h"


/** Port number */
static int port = DEFAULT_PROXY_PORT;

/** Pointer to the activation key string */
static char *activationKey;

/** Configuration file containing non-volatile proxy information */
static char configFilename[PROXYCLI_CONFIG_FILE_PATH_SIZE];


/***************** Private Prototypes ****************/
static void _proxycli_printUsage();
static void _proxycli_printVersion();

/***************** Public Functions ****************/
/**
 * Parse the command line arguments, to be retrieved by getter functions when
 * needed
 */
void proxycli_parse(int argc, char *argv[]) {
  int c;

  strncpy(configFilename, DEFAULT_PROXY_CONFIG_FILENAME, PROXYCLI_CONFIG_FILE_PATH_SIZE);

  while ((c = getopt(argc, argv, "c:p:a:v")) != -1) {
    switch (c) {
    case 'c':
      strncpy(configFilename, optarg, PROXYCLI_CONFIG_FILE_PATH_SIZE);
      printf("[cli] Proxy config file set to %s\n", configFilename);
      SYSLOG_INFO("[cli] Proxy config file set to %s", configFilename);
      break;

    case 'p':
      // Set the port number
      port = atoi(optarg);
      break;

    case 'a':
      // Register using the given activation code
      activationKey = optarg;
      printf("[cli] Activating with key %s\n", activationKey);
      SYSLOG_INFO("[cli] Activating with key %s", activationKey);
      break;

    case 'v':
      _proxycli_printVersion();
      exit(0);
      break;

    case '?':
      _proxycli_printUsage();
      exit(1);
      break;

    default:
      printf("[cli] Unknown argument character code 0%o\n", c);
      SYSLOG_ERR ("[cli] Unknown argument character code 0%o\n", c);
      _proxycli_printUsage();
      exit(1);
      break;
    }
  }
}

/**
 * @return the port number to open on
 */
int proxycli_getPort() {
  return port;
}

/**
 * @return the activation key, NULL if it was never set
 */
const char *proxycli_getActivationKey() {
  return activationKey;
}

/**
 * @return The configuration filename for the proxy
 */
const char *proxycli_getConfigFilename() {
  return configFilename;
}

/***************** Private Functions ****************/
/**
 * Instruct the user how to use the application
 */
static void _proxycli_printUsage() {
  char *usage = ""
    "Usage: ./proxyserver (options)\n"
    "\t[-p port] : Define the port to open the proxy on\n"
    "\t[-c filename] : The name of the configuration file for the proxy\n"
    "\t[-a key] : Activate this proxy using the given activation key and exit\n"
    "\t[-v] : Print version information\n"
    "\t[-?] : Print this menu\n";

  printf("%s", usage);
  SYSLOG_INFO("%s", usage);
}

/**
 * Print the version number
 */
static void _proxycli_printVersion() {
  printf("Built on %s at %s\n", __DATE__, __TIME__);
  printf("Git repository version %x\n", GIT_FIRMWARE_VERSION);
}
