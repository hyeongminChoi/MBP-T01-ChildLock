#include <gtest/gtest.h>

extern "C" {
#include "child_lock_types.h"
#include "uc2_emergency_release.h"
}

struct Uc2TestContext {
    bool applyRequestSent;
    bool ackReceived;
    bool applySuccess;
    bool ledDisplaySent;
    bool notificationSent;
    bool failureHandled;
    int applyCalls;
    int checkAckCalls;
    int ledDisplayCalls;
    int notificationCalls;
    int failureCalls;
    ChildLockState appliedState;
    ChildLockState displayedState;
};

static bool applyRearDoorStub(ChildLockState targetState, void *context)
{
    Uc2TestContext *ctx = static_cast<Uc2TestContext *>(context);
    ctx->applyCalls += 1;
    ctx->appliedState = targetState;
    return ctx->applyRequestSent;
}

static bool checkRearDoorAckStub(bool *ackReceived, bool *applySuccess, void *context)
{
    Uc2TestContext *ctx = static_cast<Uc2TestContext *>(context);
    ctx->checkAckCalls += 1;
    *ackReceived = ctx->ackReceived;
    *applySuccess = ctx->applySuccess;
    return true;
}

static bool requestLedDisplayStub(ChildLockState finalState, void *context)
{
    Uc2TestContext *ctx = static_cast<Uc2TestContext *>(context);
    ctx->ledDisplayCalls += 1;
    ctx->displayedState = finalState;
    return ctx->ledDisplaySent;
}

static bool requestEmergencyNotifStub(ChildLockState, void *context)
{
    Uc2TestContext *ctx = static_cast<Uc2TestContext *>(context);
    ctx->notificationCalls += 1;
    return ctx->notificationSent;
}

static bool handleRearDoorFailureStub(ChildLockState, bool, void *context)
{
    Uc2TestContext *ctx = static_cast<Uc2TestContext *>(context);
    ctx->failureCalls += 1;
    return ctx->failureHandled;
}

class Uc2Test : public ::testing::Test {
protected:
    void SetUp() override
    {
        context.applyRequestSent = true;
        context.ackReceived = true;
        context.applySuccess = true;
        context.ledDisplaySent = true;
        context.notificationSent = true;
        context.failureHandled = true;
        context.applyCalls = 0;
        context.checkAckCalls = 0;
        context.ledDisplayCalls = 0;
        context.notificationCalls = 0;
        context.failureCalls = 0;
        context.appliedState = CHILD_LOCK_STATE_ON;
        context.displayedState = CHILD_LOCK_STATE_ON;

        dependencies.applyStateToRearDoorModule = applyRearDoorStub;
        dependencies.checkRearDoorModuleAck = checkRearDoorAckStub;
        dependencies.requestChildLockLedStatusDisplay = requestLedDisplayStub;
        dependencies.requestEmergencyUnlockNotification = requestEmergencyNotifStub;
        dependencies.handleRearDoorApplyFailure = handleRearDoorFailureStub;
        dependencies.context = &context;
    }

    Uc2TestContext context {};
    Uc2Dependencies dependencies {};
};

/* F1: 비상 신호 수신 확인 */
TEST_F(Uc2Test, CrashSignalOneIsAccepted)
{
    EXPECT_TRUE(receiveEmergencyUnlockRequest(1));
}

TEST_F(Uc2Test, CrashSignalZeroIsRejected)
{
    EXPECT_FALSE(receiveEmergencyUnlockRequest(0));
}

/* F2: 현재 상태 조회 */
TEST_F(Uc2Test, NullControllerReturnsFailSafeOff)
{
    EXPECT_EQ(CHILD_LOCK_STATE_OFF, getUc2CurrentChildLockState(nullptr));
}

TEST_F(Uc2Test, ControllerReturnsStoredState)
{
    Uc2ChildLockController ctrl = { CHILD_LOCK_STATE_ON };
    EXPECT_EQ(CHILD_LOCK_STATE_ON, getUc2CurrentChildLockState(&ctrl));
}

/* F3: 상태 변경 필요 여부 판단 */
TEST_F(Uc2Test, OnStateRequiresEmergencyAction)
{
    EXPECT_TRUE(determineEmergencyAction(CHILD_LOCK_STATE_ON));
}

TEST_F(Uc2Test, OffStateSkipsEmergencyAction)
{
    EXPECT_FALSE(determineEmergencyAction(CHILD_LOCK_STATE_OFF));
}

/* F4: 강제 해제 상태 결정 */
TEST_F(Uc2Test, ForceChildLockOffReturnsOff)
{
    EXPECT_EQ(CHILD_LOCK_STATE_OFF, forceChildLockOff());
}

/* 정상 플로우: ON → 비상 신호 → OFF */
TEST_F(Uc2Test, OnStateWithCrashSignalReleasesLock)
{
    Uc2ChildLockController ctrl = { CHILD_LOCK_STATE_ON };

    Uc2Status status = processEmergencyUnlockRequest(&ctrl, &dependencies, 1);

    EXPECT_EQ(UC2_STATUS_OK, status);
    EXPECT_EQ(CHILD_LOCK_STATE_OFF, ctrl.currentState);
    EXPECT_EQ(CHILD_LOCK_STATE_OFF, context.appliedState);
}

/* A1: 이미 OFF 상태 → 추가 동작 없이 종료 */
TEST_F(Uc2Test, AlreadyOffStateSkipsRelease)
{
    Uc2ChildLockController ctrl = { CHILD_LOCK_STATE_OFF };

    Uc2Status status = processEmergencyUnlockRequest(&ctrl, &dependencies, 1);

    EXPECT_EQ(UC2_STATUS_ALREADY_UNLOCKED, status);
    EXPECT_EQ(0, context.applyCalls);
}

/* F9: UC-4 비상 해제 알림 요청 */
TEST_F(Uc2Test, SuccessRequestsEmergencyNotification)
{
    Uc2ChildLockController ctrl = { CHILD_LOCK_STATE_ON };

    Uc2Status status = processEmergencyUnlockRequest(&ctrl, &dependencies, 1);

    EXPECT_EQ(UC2_STATUS_OK, status);
    EXPECT_EQ(1, context.notificationCalls);
}

/* F8: UC-3 LED 표시 요청 */
TEST_F(Uc2Test, SuccessRequestsLedDisplay)
{
    Uc2ChildLockController ctrl = { CHILD_LOCK_STATE_ON };

    processEmergencyUnlockRequest(&ctrl, &dependencies, 1);

    EXPECT_EQ(1, context.ledDisplayCalls);
    EXPECT_EQ(CHILD_LOCK_STATE_OFF, context.displayedState);
}

/* F10: ACK 미수신 → 이전 상태 복원 */
TEST_F(Uc2Test, AckTimeoutRestoresPreviousState)
{
    Uc2ChildLockController ctrl = { CHILD_LOCK_STATE_ON };
    context.ackReceived = false;
    context.applySuccess = false;

    Uc2Status status = processEmergencyUnlockRequest(&ctrl, &dependencies, 1);

    EXPECT_EQ(UC2_STATUS_ACK_TIMEOUT, status);
    EXPECT_EQ(CHILD_LOCK_STATE_ON, ctrl.currentState);
    EXPECT_EQ(1, context.failureCalls);
}

/* 비상 신호 없으면 동작하지 않음 */
TEST_F(Uc2Test, NoSignalReturnsNoRequest)
{
    Uc2ChildLockController ctrl = { CHILD_LOCK_STATE_ON };

    Uc2Status status = processEmergencyUnlockRequest(&ctrl, &dependencies, 0);

    EXPECT_EQ(UC2_STATUS_NO_REQUEST, status);
    EXPECT_EQ(0, context.applyCalls);
}

/* NULL 인자 방어 */
TEST_F(Uc2Test, NullArgumentsReturnInvalidArg)
{
    EXPECT_EQ(UC2_STATUS_INVALID_ARGUMENT,
        processEmergencyUnlockRequest(nullptr, nullptr, 1));
}
