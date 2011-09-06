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

#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <rpc/types.h>
#include <errno.h>

#include "cppunit/CompilerOutputter.h"
#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestListener.h"
#include "cppunit/TextTestProgressListener.h"
#include "cppunit/TestRunner.h"
#include "cppunit/TestResult.h"
#include "cppunit/TextTestRunner.h"
#include "cppunit/TextTestResult.h"
#include "cppunit/TestResultCollector.h"
#include "cppunit/TestSuite.h"
#include "cppunit/ui/text/TestRunner.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/XmlOutputter.h"
#include "cppunit/TextOutputter.h"

#include "eui64_test.h"

extern "C" {
#include "iotdebug.h"
#include "ioterror.h"
#include "eui64.h"
}

CPPUNIT_TEST_SUITE_REGISTRATION( Eui64Test );


void Eui64Test::testGetString(void) {
  char address[EUI64_STRING_SIZE];
  CPPUNIT_ASSERT_MESSAGE("Didn't get an EUI64 string\n", eui64_toString(address, sizeof(address)) == SUCCESS);
}

void Eui64Test::testGetTooManyChars(void) {
  char address[EUI64_STRING_SIZE - 1];
  CPPUNIT_ASSERT_MESSAGE("EUI64 chars copied into too small of a buffer\n", eui64_toString(address, sizeof(address)) != SUCCESS);
}

void Eui64Test::testGetBytes(void) {
  uint8_t address[EUI64_BYTES_SIZE];
  CPPUNIT_ASSERT_MESSAGE("Didn't get EUI64 bytes\n", eui64_toBytes(address, sizeof(address)) == SUCCESS);
}

void Eui64Test::testGetTooManyBytes(void) {
  uint8_t address[EUI64_BYTES_SIZE - 1];
  CPPUNIT_ASSERT_MESSAGE("EUI64 bytes copied into too small of a buffer\n", eui64_toBytes(address, sizeof(address)) != SUCCESS);
}


