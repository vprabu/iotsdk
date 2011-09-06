/*
 * Copyright (c) 2011 People Power Company
 * All rights reserved.
 *
 * This open source code was developed with fufnding from People Power Company
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
 * Library to read and write configuration information in a file
 * @author Yvan Castilloux
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <limits.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <rpc/types.h>
#include <unistd.h>

#include "libconfigio.h"
#include "ioterror.h"
#include "iotdebug.h"

/**
 * @brief   Generic function to write token into a file in the form (token=value)
 *
 * @param   fileName: file name for which token will be updated
 * @param   token: Token in file to update: example (ESP_HOST_NAME)
 * @param   value: value of token to update (see definition of max buffer sizes in libconfigio.h)
 *
 * @return  SUCCESS or FAIL
*/
error_t libconfigio_write(const char* fileName, const char* token, const char* value)
{
    error_t retVal = SUCCESS;
    FILE *configFd = NULL;
    FILE *tmpConfigFd = NULL; // of back up file
    char line[LINE_MAX];
    char *eofStatus;
    char *tmpString;
    long filePos = -1; ///where the current value is located
    long eofPos = -1; ///where the end of the file is
    char currentValue[LINE_MAX];
    int result = 0;
    char tmpFileName[256]; ///back up file name when copying the file

    assert(fileName);
    assert(token);
    assert(value);

    memset(line, 0, sizeof(line));

    umask(022); // setting permissions to be able to write, open files

    // note that the config file or a link to it must be in /opt/etc, the source file is in
    // hub/etc/
    if (access(fileName, F_OK) != 0)
    {
        SYSLOG_DEBUG("file %s does not exist -> will be created", fileName);
    }else if (access(fileName, W_OK) != 0)
    {
        SYSLOG_DEBUG("no write permission for file %s", fileName);
        return FAIL;
    }else
    {
        // see if the token already exists -> if so read it and gets its line position in the file.
        filePos = libconfigio_read (fileName, token, currentValue, sizeof(currentValue));

        if(strcmp (value, currentValue) == 0)
        {
            // found value and it is the same -> no need to write the flash -> we're done
            retVal = SUCCESS;
            goto out;
        }
    }

    if (filePos != -1) //if found the value
    {
        //used for backing up of data when rewriting values -> temporary file
        snprintf(tmpFileName, sizeof(tmpFileName), "%s.tmp", fileName);
        // TODO: use temp file tmpnam() from Linux
        tmpConfigFd = fopen(tmpFileName, "w+");
        if (tmpConfigFd == NULL)
        {
            SYSLOG_ERR("%s -> could not open %s for reading and writing", strerror(errno), tmpFileName);
            retVal = FAIL;
            goto out;
        }

        // file that we will update
        configFd = fopen(fileName, "r+");
        if (configFd == NULL)
        {
            SYSLOG_ERR("%s -> could not open %s for reading and writing", strerror(errno), fileName);
            retVal = FAIL;
            goto out;
        }

        //find which that existing value is
        fseek(configFd, filePos, SEEK_SET);

        //get the first line and then dump it.
        if (fgets(line, sizeof(line), configFd) == NULL)
        {
          SYSLOG_ERR("fgets of first line failed");
        }

        // now we need to cpy the following lines in the temporary file
        while (!feof (configFd))
        {
            if(fgets(line, sizeof(line), configFd) == NULL)
            {
                break;
            }
            else if(fputs(line, tmpConfigFd) == EOF)
            {
                SYSLOG_ERR("writing tmp file %s, %s", tmpFileName, strerror(errno));
            }
        }

        //go back to where that line is
        fseek(configFd, filePos, SEEK_SET);
        // write the new value
        fprintf(configFd, "%s=%s\n", token,value);
        // then recopy the tmp file where we are now
        //go back to where that line is
        fseek(tmpConfigFd, 0, SEEK_SET);

        // now we need to cpy the following lines in the temporary file
        while (!feof (tmpConfigFd))
        {
            if(fgets(line, sizeof(line), tmpConfigFd) == NULL)
            {
                break;
            }
            else
            {
                if(fputs(line, configFd) == EOF)
                {
                    SYSLOG_ERR("writing cur file %s, %s", fileName, strerror(errno));
                }
            }
        }
        //update so that we know the file size
        eofPos = ftell(configFd);
    }
    else
    {
        //create - write token + value in file if either value or file does not exist

        //find where that existing value is
        if(configFd != NULL)
        {
            fclose(configFd);
        }
        configFd = fopen(fileName, "a+");
        if (configFd == NULL)
        {
            SYSLOG_ERR("%s -> could not open %s for appending", strerror(errno), fileName);
            retVal = FAIL;
            goto out;
        }
        fprintf(configFd, "%s=%s\n", token,value);

    }

    out:
        if (configFd != NULL)
        {
            if (eofPos > 0)
            {
                //eliminate junk at the end.
                if(ftruncate(fileno(configFd), eofPos) < 0)
                {
                    SYSLOG_ERR("ftruncate failed: %s", strerror(errno));
                }
            }
            fflush(configFd);
            fclose(configFd);
        }

        if (tmpConfigFd != NULL)
        {
            fclose(tmpConfigFd);
            remove(tmpFileName);
        }
        return retVal;
}

/**
 * @brief   Generic function to read token from a file in the form (token=value)
 *
 * @param   fileName: file name for which token will be updated
 * @param   token: Token in file to read: example (ESP_HOST_NAME)
 * @param   value: ptr to which token will be written (see definition of max buffer sizes in libconfigio.h)
 * @param   valueSize: size of buffer pointed by value
 *
 * @return  offset in file of the line where the token reside in the file. -1 if not present or
 *              file does not exist
 **/
long libconfigio_read(const char* fileName, const char* token, char* value, int valueSize)
{
    long retVal = -1;
    FILE *configFd;
    char line[LINE_MAX];
    char *eofStatus;
    char *tmpString;
    int index = 0;

    assert(fileName);
    assert(token);
    assert(value);

    memset(line,0, sizeof(line));

    umask(022); // setting permissions to be able to read, write, open files

    if (access(fileName, F_OK) != 0)
    {
        SYSLOG_ERR("file %s does not exist", fileName);
        retVal = -1;
        goto out;
    }else if (access(fileName, R_OK) != 0)
    {
        SYSLOG_ERR("no read permission for file %s", fileName);
        retVal = -1;
        goto out;
    }else
    {
        //file exists
    }

    configFd = fopen(fileName, "r");
    if (configFd == NULL)
    {
        SYSLOG_ERR("%s -> could not open %s for reading", strerror(errno), fileName);
        retVal = -1;
        goto out;
    }

    // read file and see if token is already there
    while (feof(configFd) == 0)
    {
        eofStatus = fgets(line, sizeof(line), configFd);
        if( eofStatus == NULL ) //another indication of EOF
        {
            break;
        }

        tmpString = strstr(line, token);

        if(tmpString != NULL)
        {
            tmpString = strstr(tmpString, "=");
            if (tmpString == NULL)
            {
                retVal = -1;
                goto out;
            }

            tmpString++;
            while (isspace(*tmpString))    // skip leading spaces and tabs
            {
                tmpString++;
            }

            //copy value in string
            // stop when you get a control character.. new line, new feed
            index = 0;
            while (iscntrl(*tmpString) == 0 && index < valueSize)
            {
                value[index] = *tmpString;
                tmpString++;
                index++;
            }
            value[index] = '\0';

            retVal = ftell(configFd) - strlen(line); //get position in the file
            break;
        }
    }

    out:
        if (configFd != NULL)
        {
            fclose(configFd);
        }
        return retVal;

}
