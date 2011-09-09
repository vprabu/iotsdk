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
 * This is an example agent which will run as a standalone application.
 * It is a template for your own code, which should save you hours of work.
 *
 * It communicates to the proxyserver application through a socket.
 * You would use this in an Linux environment where you might have multiple
 * agent applications connecting to a single proxyserver which manages the
 * communications from a single point with the cloud.
 *
 * In a different type of software architecture, not shown in this example
 * template, you could get rid of the proxyserver completely and compile the
 * proxy into your agent.  The proxyserver has a proxyagent directory which is
 * an example of this.  You would deploy an application like that directly on
 * your single device that you are managing over the internet.
 *
 * The application_send() and application_receive() functions you'll see in this
 * file let your application decide whether it should bridge communications
 * directly to the proxy which is compiled together with the agent, or
 * to a socket connected to a proxyserver application running separately.
 *
 * You can use the "renameFromGadgetToYourDevice.sh" script to customize
 * all files in this tree to your device's name. Edit the script and run it...
 *
 *
 * It is your responsibility as the developer to implement 4 things:
 *
 *   1. How will you discover your devices? This is usually completely device-
 *      dependent.
 *
 *   2. What profile information do you want to include in your hourly
 *      device heartbeats to the server? (i.e. device IP addresses, version
 *      numbers, model information, etc.)
 *
 *   3. What measurement information do you want to send to the server from
 *      each device?
 *
 *   4. What commands do you want to implement from the server?
 *
 *
 * My example agent here manages some made-up smart outlets that can
 * measure and control power to a single socket.  In my example code,
 * the devices are discovered with a SSDP mechanism, and they are
 * controllable over an HTTP RESTful interface with JSON. I tried to write the
 * code as close as possible to a real device you might encounter, but
 * this device doesn't really exist.
 *
 * The main file below is responsible for connecting to the proxyserver,
 * setting the callback listener when a command is received, and then
 * endlessly looping through all the functions.  We check the time
 * on each iteration to see if it's time to send a heartbeat, do a measurement,
 * etc.  You can add whatever functionality your device needs.
 *
 * Walk through of other files that are important:
 *   > ./gadgetagent.h
 *         This header describes global constants for your device, most
 *         importantly the device type which is registered at the server for
 *         your device.
 *
 *   > manager/gadgetmanager.h
 *         This defines a struct that tracks the state of your devices and
 *         the total number of devices we want to track.
 *
 *   > discover/gadgetdiscovery.c
 *         This is 100% defined by the developer. You must implement whatever
 *         mechanism is required to discover your devices and add them
 *         into the gadgetmanager.c
 *
 *   > manager/gadgetmanager.c
 *         This tracks all the devices under the control of your agent.
 *
 *   > heartbeat/gadgetheartbeat.c
 *         Each gadget has some static, constant data associated with it.
 *         Things like the IP address, firmware version, model number, etc.
 *         The heartbeat is a notification to the server "I am alive" that
 *         also repeats the generally non-changing information about a device.
 *
 *   > measure/gadgetmeasure.c
 *         The developer can leverage this module to grab measurements from
 *         his device and form a message using the IOT API commands to
 *         send the message onto the server.
 *
 *   > control/gadgetcontrol.c
 *         The developer will use this module to execute commands from the
 *         server
 *
 * Enjoy.
 *
 * @author David Moss
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>

#include "ioterror.h"
#include "iotdebug.h"
#include "proxyserver.h"
#include "clientsocket.h"
#include "iotapi.h"

#include "gadgetagent.h"

/** Set to true to terminate the agent */
bool gTerminate;

/** Configurable heartbeat period */
static int heartbeatPeriod_sec = GADGET_HEARTBEAT_PERIOD_SEC;

/** Configurable measurement period */
static int measurementPeriod_sec = GADGET_MEASUREMENT_PERIOD_SEC;

/** Last discovery time */
static struct timeval lastDiscoveryTime;

/** Last heartbeat time */
static struct timeval lastHeartbeatTime;

/** Last measurement time */
static struct timeval lastMeasurementTime;


/***************** Functions ****************/
/**
 * Main function
 */
int main(int argc, char *argv[]) {
  SYSLOG_INFO("*************** GADGET Agent ***************");

  // Open a socket to the proxy server
  // DEFAULT_PROXY_PORT comes from proxyserver.h
  if(clientsocket_open("localhost", DEFAULT_PROXY_PORT) != SUCCESS) {
    SYSLOG_DEBUG("[gadget] Couldn't open client socket");
    exit(1);
  }

  printf("Running gadget agent\n");

  // Listen for commands
  iotxml_addCommandListener(&gadgetcontrol_execute);

  while(!gTerminate) {
    struct timeval curTime = { 0, 0 };

    while (!gTerminate) {
      gettimeofday(&curTime, NULL);

      // Discover devices periodically
      if ((curTime.tv_sec - lastDiscoveryTime.tv_sec) >= GADGET_DISCOVERY_PERIOD_SEC) {
        SYSLOG_INFO("[gadget] Attempting discovery");
        lastDiscoveryTime.tv_sec = curTime.tv_sec;

        gadgetdiscovery_runOnce();
      }

      // Send out heartbeats for our known devices periodically
      if ((curTime.tv_sec - lastHeartbeatTime.tv_sec) >= heartbeatPeriod_sec) {
        SYSLOG_INFO("[gadget] Heartbeat");
        lastHeartbeatTime.tv_sec = curTime.tv_sec;

        gadgetheartbeat_send();
      }

      // Take measurements and kill off stragglers periodically
      if ((curTime.tv_sec - lastMeasurementTime.tv_sec) >= measurementPeriod_sec) {
        SYSLOG_INFO("[gadget] Measure");
        lastMeasurementTime.tv_sec = curTime.tv_sec;

        gadgetmeasure_capture();

        gadgetmanager_garbageCollection();

        gadgetmeasure_send();
      }

      sleep(10);
    }
  }

  return 0;
}

/**
 * The application layer is responsible for routing raw messages received
 * from the server.
 *
 * We use a 'clientsocket' module to communicate back and forth with the
 * proxyserver's socket.  The clientsocket module creates a new thread to
 * receive commands at any time from the proxy server.
 *
 * The clientsocket module will execute this function upon receiving a message,
 * and it doesn't have this function implemented, so we must implement it
 * ourselves.
 *
 * Here in our application layer, we route the message to the appropriate
 * parser. Currently this parser is XML, but in the future it could be JSON.
 *
 * @param msg The received message
 * @param len The length of the message
 */
void application_receive(const char *msg, int len) {
  if(strstr(msg, "xml") != NULL) {
       iotxml_parse(msg, len);

    } else {
      SYSLOG_INFO("[client] Unknown message format: %s", msg);
    }
}

/**
 * After the XML generator is finished generating a message, it attempts to
 * send a message to this function.  The IOT XML generator does not have this
 * function implemented because the application layer is responsible for
 * routing the outgoing message to whatever communication mechanism we're using.
 * In our case, the communication mechanism is the 'clientsocket' module to
 * connect us remotely to a proxyserver's socket.
 *
 * @param msg The constructed message to send
 * @param len The size of the message
 * @return SUCCESS if the message is sent
 */
error_t application_send(const char *msg, int len) {
  SYSLOG_DEBUG("[gadget] Send message: %s", msg);
  return clientsocket_send(msg, len);
}

/**
 * Set the measurement period
 * @param seconds Seconds between measurements
 */
void gadgetagent_setMeasurementPeriod(int seconds) {
  measurementPeriod_sec = seconds;
}

/**
 * Set the heartbeat period
 * @param seconds Seconds between measurements
 */
void gadgetagent_setHeartbeatPeriod(int seconds) {
  heartbeatPeriod_sec = seconds;
}

/**
 * When we have a new device added to the system, get its measurements and
 * capture its schedule
 */
void gadgetagent_refreshDevices() {
  lastMeasurementTime.tv_sec = 0;
}
