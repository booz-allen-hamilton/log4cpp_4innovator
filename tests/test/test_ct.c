// unity test Framework
#include "unity.h"

// gmock utilities
#include "gmock/gmock.h"
using namespace testing;

// mocked static methods of a C++ class
#include "mock_HierarchyMaintainer.hh"
#include "mock_LayoutsFactory.hh"

// instruct CMock to link these files
TEST_FILE("../googletest/googletest/src/gtest-all.cc")
TEST_FILE("../googletest/googlemock/src/gmock-all.cc")
TEST_FILE("HierarchyMaintainer.cpp")
TEST_FILE("Category.cpp")
TEST_FILE("Appender.cpp")
TEST_FILE("StringUtil.cpp")
TEST_FILE("LoggingEvent.cpp")
TEST_FILE("NDC.cpp")
TEST_FILE("TimeStamp.cpp")
TEST_FILE("PThreads.cpp")
TEST_FILE("CategoryStream.cpp")

#include <stdio.h>

// the code under test
#include <log4cpp/Category.hh>

// For cmock
void setUp(void) {}
void tearDown(void) {}

class NotifyCMock : public ::testing::EmptyTestEventListener {
  // Called after a failed assertion or a SUCCESS().
  virtual void OnTestPartResult(const ::testing::TestPartResult& test_part_result) {
    // Would prefer to just call UnityFail, but UNITY_FAIL_AND_BAIL at end results in hanging the test suite
    //RETURN_IF_FAIL_OR_IGNORE;
    if (test_part_result.failed())
    {
      // Print in style mimicking CMock so test_summary.rb parses it
      UnityPrint(Unity.TestFile);
      UnityPrint(":");
      UnityPrintNumber(test_part_result.line_number());
      UnityPrint(":");
      UnityPrint(Unity.CurrentTestName);
      UnityPrint(":");
      UnityPrint(UnityStrFail);
      UnityPrint(": gtest/gmock");
// TBD UnityPrint does not print multi-line nicely
//      UnityPrint(":");
//      UnityPrint(test_part_result.summary());

      // Notify unity that the current test failed
      Unity.CurrentTestFailed = 1;
    }
  }
};

void suiteSetUp(void) {
  ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
  // Register listener for gtest failures (so we can notify CMock)
  listeners.Append(new NotifyCMock);
}

void test_Category_existsNo_cmock(void) {
  log4cpp::HierarchyMaintainer hm;

  // Brand new, so of course it doesn't exist
  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(hm);
  TEST_ASSERT_NULL(log4cpp::Category::exists("foo"));

  log4cpp::LayoutsFactory lf;
  log4cpp_LayoutsFactory_getInstance_ExpectAndReturn(lf);
  log4cpp::LayoutsFactory& result = log4cpp::LayoutsFactory::getInstance();
  TEST_ASSERT_EQUAL_PTR(&result, &lf);
}

void test_Category_existsYes_cmock(void) {
  log4cpp::HierarchyMaintainer hm, other;

  // Create foo
  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(hm);
  log4cpp::Category &cat = log4cpp::Category::getInstance("foo");

  // Check for foo
  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(hm);	// PASS
//  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(other);	// FAIL
  TEST_ASSERT_NOT_NULL(log4cpp::Category::exists("foo"));
}

class MockHierarchyMaintainer : public log4cpp::HierarchyMaintainer {
public:
  MOCK_METHOD(log4cpp::Category*, getExistingInstance, (const std::string& name), (override));
};

void test_Category_existsNo_gmock(void) {
  StrictMock<MockHierarchyMaintainer> hm;

  // Configure default HM to report Category does not yet exist
  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(hm);
  EXPECT_CALL(hm, getExistingInstance(_)).WillOnce(Return(nullptr));
//  EXPECT_CALL(hm, getExistingInstance(_)).Times(2).WillRepeatedly(Return(nullptr));	// FAIL

  TEST_ASSERT_NULL(log4cpp::Category::exists("foo"));
//  TEST_ASSERT_NOT_NULL(log4cpp::Category::exists("foo"));	// FAIL
}

void test_Category_existsYes_gmock(void) {
  StrictMock<MockHierarchyMaintainer> hm;

  // Configure default HM to report Category found
  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(hm);
  EXPECT_CALL(hm, getExistingInstance(_)).WillOnce(Return((log4cpp::Category *) 0x123));

  TEST_ASSERT_NOT_NULL(log4cpp::Category::exists("foo"));
//  TEST_ASSERT_NULL(log4cpp::Category::exists("foo"));	// FAIL
}

