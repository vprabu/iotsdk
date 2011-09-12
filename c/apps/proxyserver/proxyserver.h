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

#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#ifndef DEFAULT_PROXY_PORT
#define DEFAULT_PROXY_PORT 60110
#endif

#ifndef DEFAULT_PROXY_URL
#define DEFAULT_PROXY_URL "alpha2.peoplepowerco.com:8080/deviceio/ml"
#endif

#ifndef DEFAULT_ACTIVATION_URL
#define DEFAULT_ACTIVATION_URL "alpha2.peoplepowerco.com/espapi/rest/hubActivation"
#endif

#ifndef DEFAULT_PROXY_DEVICETYPE
#define DEFAULT_PROXY_DEVICETYPE "3"
#endif

#ifndef DEFAULT_PROXY_CONFIG_FILENAME
#define DEFAULT_PROXY_CONFIG_FILENAME "proxy.conf"
#endif

/** Name of the token in our config file that stores the device type */
#define CONFIGIO_PROXY_DEVICE_TYPE_TOKEN_NAME "PROXY_DEVICE_TYPE"

/** Token for the path to the local proxy SSL certificate */
#define CONFIGIO_PROXY_SSL_CERTIFICATE_PATH "PROXY_SSL_CERTIFICATE_PATH"

/** Token to store the number of times this proxy app has been rebooted */
#define CONFIGIO_PROXY_REBOOTS "PROXY_REBOOTS"

/** The user's activation key for the proxy, given by the cloud */
#define CONFIGIO_PROXY_ACTIVATION_KEY "USER_ACTIVATION_KEY"

/** Token for host address of the cloud, i.e. "developers.peoplepowerco.com" */
#define CONFIGIO_CLOUD_HOST "CLOUD_HOST"

/** Token for port address to the cloud, i.e "8080" */
#define CONFIGIO_CLOUD_PORT "CLOUD_PORT"

/** Token for URI of the cloud, i.e. "deviceio/ml" */
#define CONFIGIO_CLOUD_URI "CLOUD_URI"

/** Token for true or false to use SSL */
#define CONFIGIO_CLOUD_USE_SSL "CLOUD_USE_SSL"

/** Token to store the authentication key for the cloud */
#define CONFIGIO_CLOUD_ACTIVATION_KEY "CLOUD_ACTIVATION_KEY"



#endif
