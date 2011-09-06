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
 * This is a stand-alone proxy server application.  It accepts socket connections
 * which communicates messages bi-directionally with the cloud server.
 *
 * @author Andrey Malashenko
 * @author David Moss
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#include <curl/curl.h>
#include <libxml/parser.h>

#include "libconfigio.h"

#include "ioterror.h"
#include "iotdebug.h"
#include "proxy.h"
#include "proxyconfig.h"
#include "proxyserver.h"
#include "proxyclientmanager.h"
#include "proxyagent.h"
#include "proxycli.h"
#include "proxyactivation.h"
#include "proxymanager.h"



/** Process termination flag */
static int gTerminate = false;

/** Client socket FD */
static int clientSocketFd;

/***************** Prototypes ***************/
void _proxyserver_processMessage(int clientSocketFd);

void _proxyserver_listener(const char *message, int len);


/***************** Functions *****************/
/**
 * Main function
 */
int main(int argc, char *argv[]) {
  int sockfd;
  int pid;
  socklen_t clientLen;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;

  // Ignore the SIGCHLD signal to get rid of zombies
  signal(SIGCHLD, SIG_IGN);

  // Parse the command line arguments
  proxycli_parse(argc, argv);

  // If the CLI tells us to activate this proxy, then activate it and exit now.
  if(proxycli_getActivationKey() != NULL) {
    if(proxyactivation_activate(proxycli_getActivationKey()) == SUCCESS) {
      printf("Activated! Exiting.\n\n");
      SYSLOG_INFO("Activated! Exiting.\n\n");
      exit(0);

    } else {
      printf("Activation failed. Exiting.\n\n");
      SYSLOG_ERR("Activation failed. Exiting.\n\n");
      exit(1);
    }
  }

  // Start up the proxy
  proxymanager_startProxy();

  // Start our proxy agent to take commands from the server to control the proxy
  if(proxyagent_start() != SUCCESS) {
    exit(1);
  }

  // Add a listener to the proxy so we can forward commands from the server to other clients / agents
  if (proxylisteners_addListener(&_proxyserver_listener) != SUCCESS) {
    SYSLOG_ERR("[%d]: Proxy is out of listener slots", getpid());
    exit(1);
  }

  // Setup a socket so external clients can connect to this proxy server
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    SYSLOG_ERR("ERROR opening socket");
  }

  bzero((char *) &serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(proxycli_getPort());

  if (bind(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
    SYSLOG_ERR("ERROR on binding");
    printf("Could not bind to port\n");
    exit(1);
  }

  listen(sockfd, 5);
  clientLen = sizeof(clientAddress);

  // Finally, the following loop accepts new client socket connections
  SYSLOG_INFO("Proxy running; port=%d; pid=%d\n", proxycli_getPort(), getpid());
  printf("Proxy running; port=%d; pid=%d\n", proxycli_getPort(), getpid());

  while (!gTerminate) {
    clientSocketFd = accept(sockfd, (struct sockaddr *) &clientAddress, &clientLen);

    if (clientSocketFd < 0) {
      SYSLOG_ERR("ERROR on accept");
      continue;
    }

    if (proxyclientmanager_add(clientSocketFd) != SUCCESS) {
      SYSLOG_ERR("[%d]: Out of client elements to track sockets", getpid());
      close(clientSocketFd);

    } else {
      pid = fork();
      if (pid < 0) {
        SYSLOG_ERR("ERROR on fork");

      } else if (pid == 0) {
        // Child process
        SYSLOG_INFO("[%d]: New client listener created", getpid());
        close(sockfd);
        while (true) {
          // Read messages from the client until the socket is closed
          _proxyserver_processMessage(clientSocketFd);
          sleep(1);
        }

        proxyclientmanager_remove(clientSocketFd);
        SYSLOG_INFO("[%d]: Destroyed", getpid());
        exit(0);

      } else {
        // Parent process

      }
    }
  }

  xmlCleanupParser();
  xmlMemoryDump();

  close(sockfd);
  pthread_exit(NULL);
  return 0;
}


/**
 * Registered listener to the proxy. This broadcasts received server messages
 * to all client sockets
 *
 * @param message Message from the server
 * @param len Length of the message from the server
 */
void _proxyserver_listener(const char *message, int len) {
  int i;
  int clients = 0;
  proxy_client_t *client;

  for(i = 0; i < proxyclientmanager_size(); i++) {
    client = proxyclientmanager_get(i);
    if(client->inUse) {
      if (write(client->fd, message, len) < 0) {
        SYSLOG_ERR("ERROR writing to socket %d, closing socket", client->fd);
        proxyclientmanager_remove(client->fd);
        close(client->fd);

      } else {
        clients++;
      }
    }
  }

  SYSLOG_DEBUG("Broadcast message to %d sockets", clients);
}

/**
 * Receives message from socket and passes it to the pipe.  The message will
 * be picked up by another thread and sent to the server
 *
 * @param clientSocketFd The client socket file descriptor
 */
void  _proxyserver_processMessage(int clientSocketFd) {
  int n;
  char buffer[PROXY_MAX_MSG_LEN];
  bzero(buffer, PROXY_MAX_MSG_LEN);

  if ((n = read(clientSocketFd, buffer, PROXY_MAX_MSG_LEN)) > 0) {
    proxy_send(buffer, n);

  } else if(n == 0) {
    SYSLOG_ERR("[%d]: Socket killed, destroying thread", getpid());
    proxyclientmanager_remove(clientSocketFd);
    close(clientSocketFd);
    exit(0);

  } else {
    SYSLOG_ERR("[%d]: Error reading from socket, destroying thread", getpid());
    proxyclientmanager_remove(clientSocketFd);
    close(clientSocketFd);
    exit(0);
  }

}

