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

extern "C" {
#include "iotdebug.h"
#include "ioterror.h"
#include "proxylisteners_test.h"
#include "proxylisteners.h"
}

CPPUNIT_TEST_SUITE_REGISTRATION( ProxyListenersTest );


static char *originalMessage;

static int originalMessageLength;

static bool listener1GotTheMessage;

static bool listener2GotTheMessage;

void listener1(const char *message, int len) {
  listener1GotTheMessage = true;
  CPPUNIT_ASSERT_MESSAGE("listener1() - messages weren't the same", strcmp(originalMessage, message) == 0);
  CPPUNIT_ASSERT_MESSAGE("listener1() - message sizes weren't equal", originalMessageLength == len);
}

void listener2(const char *message, int len) {
  listener2GotTheMessage = true;
  CPPUNIT_ASSERT_MESSAGE("listener2() - messages weren't the same", strcmp(originalMessage, message) == 0);
  CPPUNIT_ASSERT_MESSAGE("listener2() - message sizes weren't equal", originalMessageLength == len);
}

void dummylistener(const char *message, int len) {
  CPPUNIT_ASSERT_MESSAGE("Dummy listener got a message!", false);
}

void ProxyListenersTest::testListeners(void) {
  char msg[] = "Hello";

  originalMessage = msg;
  originalMessageLength = sizeof(originalMessage);
  listener1GotTheMessage = false;
  listener2GotTheMessage = false;

  // Add listeners
  CPPUNIT_ASSERT_MESSAGE("Wrong number of listeners registered\n", proxylisteners_totalListeners() == 0);
  CPPUNIT_ASSERT_MESSAGE("Couldn't add listener1\n", proxylisteners_addListener(&listener1) == SUCCESS);
  CPPUNIT_ASSERT_MESSAGE("Wrong number of listeners registered\n", proxylisteners_totalListeners() == 1);
  CPPUNIT_ASSERT_MESSAGE("Couldn't add listener2\n", proxylisteners_addListener(&listener2) == SUCCESS);
  CPPUNIT_ASSERT_MESSAGE("Wrong number of listeners registered\n", proxylisteners_totalListeners() == 2);
  CPPUNIT_ASSERT_MESSAGE("Couldn't add listener1 again\n", proxylisteners_addListener(&listener1) == SUCCESS);
  CPPUNIT_ASSERT_MESSAGE("Wrong number of listeners registered\n", proxylisteners_totalListeners() == 2);
  CPPUNIT_ASSERT_MESSAGE("Couldn't add listener2 again\n", proxylisteners_addListener(&listener2) == SUCCESS);
  CPPUNIT_ASSERT_MESSAGE("Wrong number of listeners registered\n", proxylisteners_totalListeners() == 2);

  // Test different types of broadcasts
  CPPUNIT_ASSERT_MESSAGE("Empty string broadcast went through but shouldn't have\n", proxylisteners_broadcast("", originalMessageLength) == FAIL);
  CPPUNIT_ASSERT_MESSAGE("Empty size broadcast went through but shouldn't have\n", proxylisteners_broadcast("Hi", 0) == FAIL);
  CPPUNIT_ASSERT_MESSAGE("Broadcast failed\n", proxylisteners_broadcast(originalMessage, originalMessageLength) == SUCCESS);

  // Make sure the last actual broadcast went through
  CPPUNIT_ASSERT_MESSAGE("Listener1 didn't get the message\n", listener1GotTheMessage);
  CPPUNIT_ASSERT_MESSAGE("Listener2 didn't get the message\n", listener1GotTheMessage);

  // Remove listeners
  CPPUNIT_ASSERT_MESSAGE("Wrong number of listeners registered\n", proxylisteners_totalListeners() == 2);
  CPPUNIT_ASSERT_MESSAGE("Dummy listener was removed\n", proxylisteners_removeListener(&dummylistener) == FAIL);
  CPPUNIT_ASSERT_MESSAGE("Wrong number of listeners registered\n", proxylisteners_totalListeners() == 2);
  CPPUNIT_ASSERT_MESSAGE("Listener2 couldn't get removed\n", proxylisteners_removeListener(&listener2) == SUCCESS);
  CPPUNIT_ASSERT_MESSAGE("Wrong number of listeners registered\n", proxylisteners_totalListeners() == 1);
  CPPUNIT_ASSERT_MESSAGE("Listener2 was removed twice\n", proxylisteners_removeListener(&listener2) == FAIL);
  CPPUNIT_ASSERT_MESSAGE("Wrong number of listeners registered\n", proxylisteners_totalListeners() == 1);
  CPPUNIT_ASSERT_MESSAGE("Listener2 couldn't get removed\n", proxylisteners_removeListener(&listener1) == SUCCESS);
  CPPUNIT_ASSERT_MESSAGE("Wrong number of listeners registered\n", proxylisteners_totalListeners() == 0);
}


