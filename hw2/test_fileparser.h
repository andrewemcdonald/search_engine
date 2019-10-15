/*
 * Copyright ©2019 Justin Hsia.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2019 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#ifndef _HW2_TEST_FILEPARSER_H_
#define _HW2_TEST_FILEPARSER_H_

#include "gtest/gtest.h"

namespace hw2 {

class Test_FileParser : public ::testing::Test {
 protected:
  Test_FileParser() {
    // Do set-up work for each test here.
  }

  virtual ~Test_FileParser() {
    // Do clean-up work for each test here.
  }

  virtual void SetUp() {
    // Code here will be called after the constructor and
    // right before each test.
  }

  virtual void TearDown() {
    // Code here will be called after each test and
    // right before the destructor.
  }

  static void SetUpTestCase() {
    // Code here will be called once for the entire
    // text fixture.  Note it is a static member function
    // (i.e., a class method, not an object instance method).
  }

  static void TearDownTestCase() {
    // Code here will be called once for the entire
    // text fixture.  Note it is a static member function
    // (i.e., a class method, not an object instance method).
  }

  // Objects declared here can be used by all tests in
  // the test case.
};  // class Test_FileParser

}  // namespace hw2

#endif  // _HW2_TEST_FILEPARSER_H_