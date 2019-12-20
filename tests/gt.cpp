#include "gtest/gtest.h"
#include "gmock/gmock.h"
using namespace testing;
#include "mock_HierarchyMaintainer.hh"
#include "mock_LayoutsFactory.hh"

#include <stdio.h>
#include <iostream>
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/StringQueueAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>

// For cmock
void setUp(void){}
void tearDown(void){}

static void CMock_Init(void)
{
  mock_HierarchyMaintainer_Init();
  mock_LayoutsFactory_Init();
}
static void CMock_Verify(void)
{
  mock_HierarchyMaintainer_Verify();
  mock_LayoutsFactory_Verify();
}
static void CMock_Destroy(void)
{
  mock_HierarchyMaintainer_Destroy();
  mock_LayoutsFactory_Destroy();
}

static void CmockPretest(void)
{
    const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    UnityBegin(test_info->test_suite_name());

    Unity.CurrentTestName = test_info->name();
//    Unity.CurrentTestLineNumber = (UNITY_LINE_TYPE)FuncLineNum;
    Unity.NumberOfTests++;
    UNITY_EXEC_TIME_START();
    CMock_Init();
    UNITY_CLR_DETAILS();
    if (TEST_PROTECT())
    {
        setUp();
//        Func();
    }
}

static void CmockPosttest(void)
{
    if (TEST_PROTECT())
    {
        tearDown();
        CMock_Verify();
    }
    CMock_Destroy();
    UNITY_EXEC_TIME_STOP();
    UnityConcludeTest();

    EXPECT_EQ(0, UnityEnd());
}

class CategoryTest : public ::testing::Test {
protected:
  // Per-test set-up logic
  virtual void SetUp()
  {
    CmockPretest();
  }

  // Per-test tear-down logic
  virtual void TearDown()
  {
    CmockPosttest();
  }
};

/*
TODO
	Cmock outputs test summary every time -> can we skip that or get failure output other way?
*/
TEST_F(CategoryTest, existsNo_cmock) {
  log4cpp::HierarchyMaintainer hm;

  // Brand new, so of course it doesn't exist
  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(hm);
  EXPECT_EQ(nullptr, log4cpp::Category::exists("foo"));

  log4cpp::LayoutsFactory lf;
  log4cpp_LayoutsFactory_getInstance_ExpectAndReturn(lf);
  log4cpp::LayoutsFactory& result = log4cpp::LayoutsFactory::getInstance();
  TEST_ASSERT_EQUAL_PTR(&result, &lf);
}

TEST_F(CategoryTest, existsYes_cmock) {
  log4cpp::HierarchyMaintainer hm, other;

  // Create foo
  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(hm);
  log4cpp::Category &cat = log4cpp::Category::getInstance("foo");

  // Check for foo
  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(hm);	// PASS
//  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(other);	// FAIL
  EXPECT_NE(nullptr, log4cpp::Category::exists("foo"));
}

class MockHierarchyMaintainer : public log4cpp::HierarchyMaintainer {
public:
  MOCK_METHOD(log4cpp::Category*, getExistingInstance, (const std::string& name), (override));
};

TEST_F(CategoryTest, existsNo_gmock) {
  StrictMock<MockHierarchyMaintainer> hm;

  // Configure default HM to report Category does not yet exist
  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(hm);
  EXPECT_CALL(hm, getExistingInstance(_)).WillOnce(Return(nullptr));

  EXPECT_EQ(nullptr, log4cpp::Category::exists("foo"));
}

TEST_F(CategoryTest, existsYes_gmock) {
  StrictMock<MockHierarchyMaintainer> hm;

  // Configure default HM to report Category found
  log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(hm);
  EXPECT_CALL(hm, getExistingInstance(_)).WillOnce(Return((log4cpp::Category *) 0x123));

  EXPECT_NE(nullptr, log4cpp::Category::exists("foo"));
}

class MockAppender : public log4cpp::OstreamAppender {
public:
  MockAppender(const char *name, std::ostream* stream): log4cpp::OstreamAppender(name, stream){}
  MOCK_METHOD(void, doAppend, (const log4cpp::LoggingEvent &event), (override));
};

TEST_F(CategoryTest, test1) {
log4cpp::HierarchyMaintainer h1;
log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(h1);

    MockAppender* appender = new MockAppender("appender", &std::cout);
    log4cpp::Layout* layout = new log4cpp::BasicLayout();
    appender->setLayout(layout);

    // add appender to root category
    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.setPriority(log4cpp::Priority::ERROR);

    // clear root's initial appender
    root.removeAllAppenders();

    root.addAppender(appender);

    // dump messages - should only process ERROR and higher
EXPECT_CALL(*appender, doAppend(Field(&log4cpp::LoggingEvent::priority, Eq(log4cpp::Priority::PriorityLevel::ERROR)))).Times(1);
EXPECT_CALL(*appender, doAppend(Field(&log4cpp::LoggingEvent::priority, Eq(log4cpp::Priority::PriorityLevel::CRIT)))).Times(1);
EXPECT_CALL(*appender, doAppend(Field(&log4cpp::LoggingEvent::priority, Eq(log4cpp::Priority::PriorityLevel::ALERT)))).Times(1);
// FATAL == EMERGE so we need additional details to distinguish
EXPECT_CALL(*appender, doAppend(AllOf(
	Field(&log4cpp::LoggingEvent::priority, Eq(log4cpp::Priority::PriorityLevel::FATAL)),
	Field(&log4cpp::LoggingEvent::message, StrEq("root fatal #1"))
))).Times(1);
EXPECT_CALL(*appender, doAppend(AllOf(
	Field(&log4cpp::LoggingEvent::priority, Eq(log4cpp::Priority::PriorityLevel::EMERG)),
	Field(&log4cpp::LoggingEvent::message, StrEq("root emerg #1"))
))).Times(1);
    root.debug("root debug #1");
    root.info("root info #1");
    root.notice("root notice #1");
    root.warn("root warn #1");
    root.error("root error #1");
    root.crit("root crit #1");
    root.alert("root alert #1");
    root.fatal("root fatal #1");
    root.emerg("root emerg #1");

    // clear all appenders
    root.removeAllAppenders();

    // dump a message - should not process it
    root.error("root error #2");
}

TEST_F(CategoryTest, test2) {
log4cpp::HierarchyMaintainer h1;
log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(h1);

    MockAppender* appender = new MockAppender("appender", &std::cout);
    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.removeAllAppenders();
    root.addAppender(appender);

EXPECT_NE(nullptr, log4cpp::Appender::getAppender("appender"));
EXPECT_EQ(nullptr, log4cpp::Appender::getAppender("noSuchNamedAppender"));
}

TEST_F(CategoryTest, test3) {
log4cpp::HierarchyMaintainer h1;
log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(h1);

    log4cpp::StringQueueAppender* appender = new log4cpp::StringQueueAppender("appender");
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%m");	// doesn't require mock to predict
    layout->setConversionPattern("%r %m%n");	// needs mock
    appender->setLayout(layout);

    // add appender to root category
    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.setPriority(log4cpp::Priority::ERROR);
    root.removeAllAppenders();
    root.addAppender(appender);
//sleep(1);
    // dump message
    root.fatal("root fatal #1");

    const std::string msg = appender->popMessage();
    ASSERT_THAT(msg, HasSubstr("root fatal #1"));

// TODO: mock TimeStamp::getStartTime().getSeconds();
//    ASSERT_STREQ(msg.c_str(), "root fatal #1");


    // clear all appenders
    root.removeAllAppenders();
}

TEST_F(CategoryTest, test4) {
log4cpp::HierarchyMaintainer h1;
log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(h1);

    EXPECT_EQ(nullptr, log4cpp::Appender::getAppender("appender"));
    log4cpp::Appender* appender = new log4cpp::OstreamAppender("appender", &std::cout);
    EXPECT_NE(nullptr, log4cpp::Appender::getAppender("appender"));

    log4cpp::Category& root = log4cpp::Category::getRoot();

    // clear root's initial appender
    root.removeAllAppenders();
    delete appender;
    EXPECT_EQ(nullptr, log4cpp::Appender::getAppender("appender"));

    appender = new log4cpp::OstreamAppender("appender", &std::cout);
    root.addAppender(appender);
    EXPECT_NE(nullptr, log4cpp::Appender::getAppender("appender"));

    root.removeAllAppenders();
    EXPECT_EQ(nullptr, log4cpp::Appender::getAppender("appender"));
}

TEST_F(CategoryTest, test5) {
log4cpp::HierarchyMaintainer h1;
log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(h1);
log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(h1);
log4cpp_HierarchyMaintainer_getDefaultMaintainer_ExpectAndReturn(h1);

	EXPECT_EQ(nullptr, log4cpp::Category::exists("foo"));
	log4cpp::Category &cat = log4cpp::Category::getInstance("foo");
	EXPECT_NE(nullptr, log4cpp::Category::exists("foo"));
}

