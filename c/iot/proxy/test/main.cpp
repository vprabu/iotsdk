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

using namespace std;

class MyProgressListener: public CppUnit::TextTestProgressListener {
  void startTest(CppUnit::Test *test) {
    cout << "Running: " << test->getName().c_str() << endl;
  }
};


int main(int argc, char *argv[]) {
  /// Define the file that will store the XML output.
  ofstream outputFile("./unittest_output.xml");

  // Create the event manager and test controller
  CppUnit::TestResult controller;

  // Add a listener that collects test result
  CppUnit::TestResultCollector result;
  controller.addListener(&result);

  // Get the top level suite from the registry
  CppUnit::TestRunner runner;

  CppUnit::XmlOutputter xmlOutputter(&result, outputFile);

  CppUnit::TextOutputter consoleOutputter(&result, std::cout);

  // Specify XML output and inform the test runner of this format.
  // First, we retrieve the instance of the TestFactoryRegistry :
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();

  // Then, we obtain and add a new TestSuite created by the TestFactoryRegistry that contains
  // all the test suite registered using CPPUNIT_TEST_SUITE_REGISTRATION().
  runner.addTest(registry.makeTest());

  // Add a listener that print test name as test runs.
  MyProgressListener progress;
  controller.addListener(&progress);

  std::string str("");

  runner.run(controller, str); // Run all tests and wait

  xmlOutputter.write();
  consoleOutputter.write();

  outputFile.close();

  return result.wasSuccessful() ? 0 : 1;
}
