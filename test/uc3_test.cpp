#include <gtest/gtest.h>

extern "C" {
#include "child_lock_types.h"
#include "uc3_child_lock_status_display.h"
}

struct TestContext {
    bool ackReceived;
    bool displaySuccess;
    int handleFailureCalls;
    LedCommand sentLedCommand;
};

static bool sendLedCommandStub(LedCommand ledCommand, void *context)
{
    TestContext *testContext = static_cast<TestContext *>(context);
    testContext->sentLedCommand = ledCommand;
    return true;
}

static bool checkLedAckStub(bool *ackReceived, bool *displaySuccess, void *context)
{
    TestContext *testContext = static_cast<TestContext *>(context);
    *ackReceived = testContext->ackReceived;
    *displaySuccess = testContext->displaySuccess;
    return true;
}

static bool handleLedFailureStub(ChildLockState, bool, void *context)
{
    TestContext *testContext = static_cast<TestContext *>(context);
    testContext->handleFailureCalls += 1;
    return true;
}

class Uc3Test : public ::testing::Test {
protected:
    void SetUp() override
    {
        context.ackReceived = true;
        context.displaySuccess = true;
        context.handleFailureCalls = 0;
        context.sentLedCommand = LED_COMMAND_OFF;

        dependencies.sendLedCommand = sendLedCommandStub;
        dependencies.checkLedAck = checkLedAckStub;
        dependencies.handleDisplayFailure = handleLedFailureStub;
        dependencies.context = &context;
    }

    TestContext context {};
    Uc3Dependencies dependencies {};
};

TEST_F(Uc3Test, OnStateSelectsLedOnCommand)
{
    LedCommand command = determineLedCommand(CHILD_LOCK_STATE_ON);
    EXPECT_EQ(command, LED_COMMAND_ON);
}

TEST_F(Uc3Test, OffStateSelectsLedOffCommand)
{
    LedCommand command = determineLedCommand(CHILD_LOCK_STATE_OFF);
    EXPECT_EQ(command, LED_COMMAND_OFF);
}

TEST_F(Uc3Test, AckAndDisplaySuccessCompletesNormally)
{
    context.ackReceived = true;
    context.displaySuccess = true;

    Uc3Status status = processChildLockStatusDisplayRequest(&dependencies, CHILD_LOCK_STATE_ON);

    EXPECT_EQ(status, UC3_STATUS_OK);
    EXPECT_EQ(context.sentLedCommand, LED_COMMAND_ON);
    EXPECT_EQ(context.handleFailureCalls, 0);
}

TEST_F(Uc3Test, MissingAckTriggersExceptionHandling)
{
    context.ackReceived = false;
    context.displaySuccess = false;

    Uc3Status status = processChildLockStatusDisplayRequest(&dependencies, CHILD_LOCK_STATE_ON);

    EXPECT_EQ(status, UC3_STATUS_ACK_TIMEOUT);
    EXPECT_EQ(context.handleFailureCalls, 1);
}

TEST_F(Uc3Test, DisplayFailureTriggersExceptionHandling)
{
    context.ackReceived = true;
    context.displaySuccess = false;

    Uc3Status status = processChildLockStatusDisplayRequest(&dependencies, CHILD_LOCK_STATE_OFF);

    EXPECT_EQ(status, UC3_STATUS_DISPLAY_FAILURE);
    EXPECT_EQ(context.handleFailureCalls, 1);
}
