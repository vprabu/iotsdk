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
 *  @file 	      pipe_comm.c
 *  @date        	  $Date$
 *  @version        $Revision$
 *
 *  @brief    General code that handles pipes (reading, writing and so on)
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <rpc/types.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "iotdebug.h"
#include "libpipecomm.h"

/**
 * @brief   Open named pipe for bidirectional communication
 *
 * @param	pipeName: name of the pipe to open
 * @return	return -1 for error, other return inFd
 */
int libpipecomm_open(const char* pipeName, bool_t isBlocking) {
  int fd = -1;

  if (pipeName != NULL) {
    fd = open(pipeName, O_RDWR);
    if (-1 == fd) {
      SYSLOG_ERR("open(%s, O_RDWR): %s", pipeName, strerror(errno));
    }
    if (isBlocking == FALSE) {
      if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        SYSLOG_ERR("fcntl, %s", strerror(errno));
      }
    }
  } else {
    SYSLOG_ERR("pipeName == NULL");
  }

  return fd;
}

/**
 * @brief   Generic function that writes into a pipe
 *
 * @param 	fd: pipe fd
 * @param 	msg: msg to send
 *
 * @return  number of written bytes
 */
int libpipecomm_write(int fd, const char *msg, uint16_t msgLen) {
  int bytesWritten = 0;
  char rawPacket[PIPE_BUF];

  if (msgLen <= sizeof(rawPacket) && msgLen > 0) {
    // all the bytes must be sent in one write command
    rawPacket[0] = (char) (msgLen & 0xFF);
    rawPacket[1] = (char) (msgLen >> 8);

    memcpy(rawPacket + 2, msg, msgLen);

    bytesWritten = write(fd, rawPacket, msgLen + 2);

    if (bytesWritten == msgLen + 2) {
    } else if (bytesWritten == -1) {
      SYSLOG_ERR("%s for fd %d", strerror(errno), fd);
    } else {
      SYSLOG_ERR("Wrote %d bytes to pipe (requested = %d), %s", bytesWritten,
          strlen(rawPacket), strerror(errno));
    }
  } else {
    SYSLOG_ERR("msg size is %d, max size = %d, %s", msgLen, sizeof(rawPacket),
        strerror(errno));
  }

  return bytesWritten;
}

/**
 * @brief   Generic function that reads from a pipe
 *
 * @param 	fd: pipe fd
 * @param 	msg: a null-terminated string with the next message received through the pipe
 * @param 	maxLen: maximum size of msg
 *
 * @return  number of read bytes
 */
int libpipecomm_read(int fd, char *msg, uint16_t maxLen) {
  uint16_t length;
  uint8_t temp;
  int bytesRead = 0;
  int i = 0;

  bytesRead = read(fd, &temp, 1);

  if (bytesRead == 1) {
    length = temp;
    bytesRead = read(fd, &temp, 1);

    if (bytesRead == 1) {
      length = temp * 0x100 + length;
      if (length > maxLen) {
        //flesh out data from the pipe
        for (i = 0; i < length; i++) {
          if (read(fd, &temp, 1) <= 0) {
            SYSLOG_ERR("can't read pipe, %s", strerror(errno));
            break;
          }
        }
        SYSLOG_ERR("length of %d larger than maxlen of %d", length, maxLen);
        bytesRead = 0;
      } else {
        bytesRead = read(fd, msg, length);
        // should never return 0 here.
        if (bytesRead <= 0) {
          SYSLOG_ERR("Read %d bytes, %s", bytesRead, strerror(errno));
        }
      }
    } else {
      // should never return 0 here
      SYSLOG_ERR("Read %d bytes instead of 1, %s", bytesRead, strerror(errno));
      bytesRead = 0;
    }
  }
  // if the pipe is empty,  should return 0 only in this case.
  else {
    if (errno != EAGAIN && errno != EINPROGRESS) {
      SYSLOG_ERR("%s", strerror(errno));
    }
    bytesRead = 0;
  }

  return bytesRead;
}
