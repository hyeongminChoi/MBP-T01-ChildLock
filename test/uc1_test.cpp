#include <gtest/gtest.h>

extern "C" {
#include "child_lock_types.h"
#include "uc1_child_lock_state_change.h"
}

struct TestContext {
    bool releaseAllowed;
    bool applyRequestSent;
    bool ackReceived;
    bool applySuccess;
    bool displayRequestSent;
    int validateCalls;
    int notificationCalls;
    int applyCalls;
    int checkAckCalls;
    int displayCalls;
    ChildLockState appliedState;
    ChildLockState displayedState;
};

static bool validateReleaseConditionStub(void *context)
{
    TestContext *testContext = static_cast<TestContext *>(context);
    testContext->validateCalls += 1;
    return testContext->releaseAllowed;
}

static bool requestNotificationStub(ChildLockState, void *context)
{
    TestContext *testContext = static_cast<TestContext *>(context);
    testContext->notificationCalls += 1;
    return true;
}

static bool applyRearDoorStub(ChildLockState targetState, void *context)
{
    TestContext *testContext = static_cast<TestContext *>(context);
    testContext->applyCalls += 1;
    testContext->appliedState = targetState;
    return testContext->applyRequestSent;
}

static bool checkRearDoorAckStub(bool *ackReceived, bool *applySuccess, void *context)
{
    TestContext *testContext = static_cast<TestContext *>(context);
    testContext->checkAckCalls += 1;
    *ackReceived = testContext->ackReceived;
    *applySuccess = testContext->applySuccess;
    return true;
}

static bool requestDisplayStub(ChildLockState finalState, void *context)
{
    TestContext *testContext = static_cast<TestContext *>(context);
    testContext->displayCalls += 1;
    testContext->displayedState = finalState;
    return testContext->displayRequestSent;
}

class Uc1Test : public ::testing::Test {
protected:
    void SetUp() override
    {
        context.releaseAllowed = false;
        context.applyRequestSent = true;
        context.ackReceived = true;
        context.applySuccess = true;
        context.displayRequestSent = true;
        context.validateCalls = 0;
        context.notificationCalls = 0;
        context.applyCalls = 0;
        context.checkAckCalls = 0;
        context.displayCalls = 0;
        context.appliedState = CHILD_LOCK_STATE_OFF;
        context.displayedState = CHILD_LOCK_STATE_OFF;

        dependencies.validateReleaseCondition = validateReleaseConditionStub;
        dependencies.requestUnlockRejectedNotification = requestNotificationStub;
        dependencies.applyStateToRearDoorModule = applyRearDoorStub;
        dependencies.checkRearDoorModuleAck = checkRearDoorAckStub;
        dependencies.requestChildLockLedStatusDisplay = requestDisplayStub;
        dependencies.context = &context;
    }

    TestContext context {};
    Uc1Dependencies dependencies {};
};

TEST_F(Uc1Test, OffStateToggleTurnsOn)
{
    ChildLockController controller {};
    controller.currentState = CHILD_LOCK_STATE_OFF;

    Uc1Status status = processChildLockToggleRequest(&controller, &dependencies, true);

    EXPECT_EQ(status, UC1_STATUS_OK);
    EXPECT_EQ(controller.currentState, CHILD_LOCK_STATE_ON);
    EXPECT_EQ(context.appliedState, CHILD_LOCK_STATE_ON);
}

TEST_F(Uc1Test, OnStateWithReleaseAllowedTurnsOff)
{
    ChildLockController controller {};
    controller.currentState = CHILD_LOCK_STATE_ON;
    context.releaseAllowed = true;

    Uc1Status status = processChildLockToggleRequest(&controller, &dependencies, true);

    EXPECT_EQ(status, UC1_STATUS_OK);
    EXPECT_EQ(controller.currentState, CHILD_LOCK_STATE_OFF);
    EXPECT_EQ(context.validateCalls, 1);
}

TEST_F(Uc1Test, OnStateWithReleaseDeniedKeepsState)
{
    ChildLockController controller {};
    controller.currentState = CHILD_LOCK_STATE_ON;
    context.releaseAllowed = false;

    Uc1Status status = processChildLockToggleRequest(&controller, &dependencies, true);

    EXPECT_EQ(status, UC1_STATUS_OK);
    EXPECT_EQ(controller.currentState, CHILD_LOCK_STATE_ON);
}

TEST_F(Uc1Test, ReleaseDeniedRequestsUc4Notification)
{
    ChildLockController controller {};
    controller.currentState = CHILD_LOCK_STATE_ON;
    context.releaseAllowed = false;

    Uc1Status status = processChildLockToggleRequest(&controller, &dependencies, true);

    EXPECT_EQ(status, UC1_STATUS_OK);
    EXPECT_EQ(context.notificationCalls, 1);
}

TEST_F(Uc1Test, RearDoorAckAndApplySuccessSavesState)
{
    ChildLockController controller {};
    controller.currentState = CHILD_LOCK_STATE_OFF;
    context.ackReceived = true;
    context.applySuccess = true;

    Uc1Status status = processChildLockToggleRequest(&controller, &dependencies, true);

    EXPECT_EQ(status, UC1_STATUS_OK);
    EXPECT_EQ(controller.currentState, CHILD_LOCK_STATE_ON);
    EXPECT_EQ(context.checkAckCalls, 1);
}

TEST_F(Uc1Test, RearDoorAckMissingTriggersException)
{
    ChildLockController controller {};
    controller.currentState = CHILD_LOCK_STATE_OFF;
    context.ackReceived = false;
    context.applySuccess = false;

    Uc1Status status = processChildLockToggleRequest(&controller, &dependencies, true);

    EXPECT_EQ(status, UC1_STATUS_ACK_TIMEOUT);
    EXPECT_EQ(controller.currentState, CHILD_LOCK_STATE_OFF);
}

TEST_F(Uc1Test, RearDoorApplyFailureKeepsPreviousState)
{
    ChildLockController controller {};
    controller.currentState = CHILD_LOCK_STATE_OFF;
    context.ackReceived = true;
    context.applySuccess = false;

    Uc1Status status = processChildLockToggleRequest(&controller, &dependencies, true);

    EXPECT_EQ(status, UC1_STATUS_APPLY_FAILURE);
    EXPECT_EQ(controller.currentState, CHILD_LOCK_STATE_OFF);
}

TEST_F(Uc1Test, SuccessRequestsUc3Display)
{
    ChildLockController controller {};
    controller.currentState = CHILD_LOCK_STATE_OFF;

    Uc1Status status = processChildLockToggleRequest(&controller, &dependencies, true);

    EXPECT_EQ(status, UC1_STATUS_OK);
    EXPECT_EQ(context.displayCalls, 1);
    EXPECT_EQ(context.displayedState, CHILD_LOCK_STATE_ON);
}
