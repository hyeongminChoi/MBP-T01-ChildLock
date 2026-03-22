#include <gtest/gtest.h>

extern "C" {
#include "child_lock_types.h"
#include "uc4_system_intervention_notification.h"
}

struct Uc4TestContext {
    bool warningResult;
    bool infoResult;
    bool faultResult;
    bool ackReceived;
    bool displaySuccess;
    bool failureHandled;
    int warningCalls;
    int infoCalls;
    int faultCalls;
    int checkAckCalls;
    int failureCalls;
};

static bool sendWarningStub(const char *, void *context)
{
    Uc4TestContext *ctx = static_cast<Uc4TestContext *>(context);
    ctx->warningCalls += 1;
    return ctx->warningResult;
}

static bool sendInfoStub(const char *, void *context)
{
    Uc4TestContext *ctx = static_cast<Uc4TestContext *>(context);
    ctx->infoCalls += 1;
    return ctx->infoResult;
}

static bool sendFaultStub(const char *, void *context)
{
    Uc4TestContext *ctx = static_cast<Uc4TestContext *>(context);
    ctx->faultCalls += 1;
    return ctx->faultResult;
}

static bool checkClusterHmiAckStub(bool *ackReceived, bool *displaySuccess, void *context)
{
    Uc4TestContext *ctx = static_cast<Uc4TestContext *>(context);
    ctx->checkAckCalls += 1;
    *ackReceived = ctx->ackReceived;
    *displaySuccess = ctx->displaySuccess;
    return true;
}

static bool handleClusterDisplayFailureStub(bool, void *context)
{
    Uc4TestContext *ctx = static_cast<Uc4TestContext *>(context);
    ctx->failureCalls += 1;
    return ctx->failureHandled;
}

class Uc4Test : public ::testing::Test {
protected:
    void SetUp() override
    {
        context.warningResult = true;
        context.infoResult = true;
        context.faultResult = true;
        context.ackReceived = true;
        context.displaySuccess = true;
        context.failureHandled = true;
        context.warningCalls = 0;
        context.infoCalls = 0;
        context.faultCalls = 0;
        context.checkAckCalls = 0;
        context.failureCalls = 0;

        dependencies.sendWarning = sendWarningStub;
        dependencies.sendInfo = sendInfoStub;
        dependencies.sendFault = sendFaultStub;
        dependencies.checkClusterHmiAck = checkClusterHmiAckStub;
        dependencies.handleClusterDisplayFailure = handleClusterDisplayFailureStub;
        dependencies.context = &context;
    }

    Uc4TestContext context {};
    Uc4Dependencies dependencies {};
};

/* F1: 유효한 InterventionType 수신 확인 */
TEST_F(Uc4Test, UnlockDeniedTypeIsAccepted)
{
    EXPECT_TRUE(receiveInterventionNotificationRequest(INTERVENTION_TYPE_UNLOCK_DENIED));
}

TEST_F(Uc4Test, EmergencyAutoUnlockTypeIsAccepted)
{
    EXPECT_TRUE(receiveInterventionNotificationRequest(INTERVENTION_TYPE_EMERGENCY_AUTO_UNLOCK));
}

TEST_F(Uc4Test, SystemFaultTypeIsAccepted)
{
    EXPECT_TRUE(receiveInterventionNotificationRequest(INTERVENTION_TYPE_SYSTEM_FAULT));
}

TEST_F(Uc4Test, InvalidTypeIsRejected)
{
    EXPECT_FALSE(receiveInterventionNotificationRequest((InterventionType)99));
}

/* F2: 알림 유형 결정 */
TEST_F(Uc4Test, UnlockDeniedMapsToWarning)
{
    EXPECT_EQ(NOTIFICATION_TYPE_WARNING,
        determineNotificationType(INTERVENTION_TYPE_UNLOCK_DENIED));
}

TEST_F(Uc4Test, EmergencyAutoUnlockMapsToInfo)
{
    EXPECT_EQ(NOTIFICATION_TYPE_INFO,
        determineNotificationType(INTERVENTION_TYPE_EMERGENCY_AUTO_UNLOCK));
}

TEST_F(Uc4Test, SystemFaultMapsToFault)
{
    EXPECT_EQ(NOTIFICATION_TYPE_FAULT,
        determineNotificationType(INTERVENTION_TYPE_SYSTEM_FAULT));
}

/* F3: UNLOCK_DENIED → 경고 메시지 송신 */
TEST_F(Uc4Test, UnlockDeniedSendsWarningMessage)
{
    Uc4Status status = processInterventionNotification(
        &dependencies, INTERVENTION_TYPE_UNLOCK_DENIED, "해제 거부");

    EXPECT_EQ(UC4_STATUS_OK, status);
    EXPECT_EQ(1, context.warningCalls);
    EXPECT_EQ(0, context.infoCalls);
    EXPECT_EQ(0, context.faultCalls);
}

/* F4: EMERGENCY_AUTO_UNLOCK → 안내 메시지 송신 */
TEST_F(Uc4Test, EmergencyAutoUnlockSendsInfoMessage)
{
    Uc4Status status = processInterventionNotification(
        &dependencies, INTERVENTION_TYPE_EMERGENCY_AUTO_UNLOCK, "자동 해제");

    EXPECT_EQ(UC4_STATUS_OK, status);
    EXPECT_EQ(0, context.warningCalls);
    EXPECT_EQ(1, context.infoCalls);
    EXPECT_EQ(0, context.faultCalls);
}

/* F5: SYSTEM_FAULT → 오류 메시지 송신 */
TEST_F(Uc4Test, SystemFaultSendsFaultMessage)
{
    Uc4Status status = processInterventionNotification(
        &dependencies, INTERVENTION_TYPE_SYSTEM_FAULT, "시스템 오류");

    EXPECT_EQ(UC4_STATUS_OK, status);
    EXPECT_EQ(0, context.warningCalls);
    EXPECT_EQ(0, context.infoCalls);
    EXPECT_EQ(1, context.faultCalls);
}

/* F6: Cluster/HMI ACK 확인 */
TEST_F(Uc4Test, AckAndDisplaySuccessCompletesNormally)
{
    context.ackReceived = true;
    context.displaySuccess = true;

    Uc4Status status = processInterventionNotification(
        &dependencies, INTERVENTION_TYPE_UNLOCK_DENIED, "경고");

    EXPECT_EQ(UC4_STATUS_OK, status);
    EXPECT_EQ(1, context.checkAckCalls);
    EXPECT_EQ(0, context.failureCalls);
}

/* F7: ACK 미수신 → 예외 처리 (차일드락 제어는 유지) */
TEST_F(Uc4Test, AckTimeoutTriggersExceptionHandling)
{
    context.ackReceived = false;
    context.displaySuccess = false;

    Uc4Status status = processInterventionNotification(
        &dependencies, INTERVENTION_TYPE_UNLOCK_DENIED, "경고");

    EXPECT_EQ(UC4_STATUS_ACK_TIMEOUT, status);
    EXPECT_EQ(1, context.failureCalls);
}

/* F7: 표시 실패 → 예외 처리 */
TEST_F(Uc4Test, DisplayFailureTriggersExceptionHandling)
{
    context.ackReceived = true;
    context.displaySuccess = false;

    Uc4Status status = processInterventionNotification(
        &dependencies, INTERVENTION_TYPE_UNLOCK_DENIED, "경고");

    EXPECT_EQ(UC4_STATUS_DISPLAY_FAILURE, status);
    EXPECT_EQ(1, context.failureCalls);
}

/* NULL 인자 방어 */
TEST_F(Uc4Test, NullDependenciesReturnInvalidArg)
{
    EXPECT_EQ(UC4_STATUS_INVALID_ARGUMENT,
        processInterventionNotification(nullptr, INTERVENTION_TYPE_UNLOCK_DENIED, "경고"));
}

TEST_F(Uc4Test, NullMessageReturnInvalidArg)
{
    EXPECT_EQ(UC4_STATUS_INVALID_ARGUMENT,
        processInterventionNotification(&dependencies, INTERVENTION_TYPE_UNLOCK_DENIED, nullptr));
}

/* 유효하지 않은 타입 */
TEST_F(Uc4Test, InvalidTypeReturnsInvalidType)
{
    EXPECT_EQ(UC4_STATUS_INVALID_TYPE,
        processInterventionNotification(&dependencies, (InterventionType)99, "경고"));
}
