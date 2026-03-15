/**
 * @file uc4_system_intervention_notification.c
 * @brief UC-4 시스템 개입 상황 알림 제공 모듈
 * @details 차일드락 관련 시스템 개입 상황(해제 거부, 비상 자동 해제, 시스템 오류) 발생 시
 *          해당 정보를 Cluster/HMI에 전달하여 운전자에게 알린다.
 * @author harvy1070
 * @date 2026-03-15
 * @note AI 구현 도구: Claude (claude-sonnet-4-6) | 구현일: 2026-03-15
 * @note 기준 문서: SwRS_v2.md UC-4, SwDD UC-4, uc4-flowchart.puml
 */

#include "uc4_system_intervention_notification.h"

/**
 * @brief 시스템 개입 알림 요청을 수신한다.
 * @details InterventionType 값이 정의된 범위 내에 있는 경우에만 요청을 수락한다.
 * @param interventionType 시스템 개입 상황 유형
 * @return requestReceived 유효한 요청 수신 여부
 */
bool receiveInterventionNotificationRequest(InterventionType interventionType)
{
    /* 정의된 범위 외의 값은 요청으로 수락하지 않는다. */
    bool requestReceived = false;

    if ((interventionType == INTERVENTION_TYPE_UNLOCK_DENIED) ||
        (interventionType == INTERVENTION_TYPE_EMERGENCY_AUTO_UNLOCK) ||
        (interventionType == INTERVENTION_TYPE_SYSTEM_FAULT)) {
        requestReceived = true;
    }

    return requestReceived;
}

/**
 * @brief InterventionType으로부터 전송할 알림 유형을 결정한다.
 * @param interventionType 시스템 개입 상황 유형
 * @return notificationType 결정된 알림 유형 (기본값 WARNING)
 */
NotificationType determineNotificationType(InterventionType interventionType)
{
    /* 알 수 없는 값은 WARNING 으로 처리하여 운전자에게 최소한의 정보를 제공한다. */
    NotificationType notificationType = NOTIFICATION_TYPE_WARNING;

    if (interventionType == INTERVENTION_TYPE_EMERGENCY_AUTO_UNLOCK) {
        notificationType = NOTIFICATION_TYPE_INFO;
    } else if (interventionType == INTERVENTION_TYPE_SYSTEM_FAULT) {
        notificationType = NOTIFICATION_TYPE_FAULT;
    }

    return notificationType;
}

/**
 * @brief 상태 변경 거부 상황에서 Cluster/HMI에 경고 메시지를 송신한다.
 * @param dependencies 외부 의존성 구조체 포인터
 * @param message 표시할 경고 메시지 문자열
 * @return commandSent 메시지 송신 성공 여부
 */
bool displayWarning(const Uc4Dependencies *dependencies, const char *message)
{
    /* sendWarning 콜백 미등록 시 송신 불가. */
    bool commandSent = false;

    if ((dependencies != (const Uc4Dependencies *)0) &&
        (dependencies->sendWarning != (void *)0) &&
        (message != (const char *)0)) {
        commandSent = dependencies->sendWarning(message, dependencies->context);
    }

    return commandSent;
}

/**
 * @brief 비상 자동 해제 상황에서 Cluster/HMI에 안내 메시지를 송신한다.
 * @param dependencies 외부 의존성 구조체 포인터
 * @param message 표시할 안내 메시지 문자열
 * @return commandSent 메시지 송신 성공 여부
 */
bool displayInfo(const Uc4Dependencies *dependencies, const char *message)
{
    /* sendInfo 콜백 위임. */
    bool commandSent = false;

    if ((dependencies != (const Uc4Dependencies *)0) &&
        (dependencies->sendInfo != (void *)0) &&
        (message != (const char *)0)) {
        commandSent = dependencies->sendInfo(message, dependencies->context);
    }

    return commandSent;
}

/**
 * @brief 시스템 오류 상황에서 Cluster/HMI에 오류 메시지를 송신한다.
 * @param dependencies 외부 의존성 구조체 포인터
 * @param message 표시할 오류 메시지 문자열
 * @return commandSent 메시지 송신 성공 여부
 */
bool displayFault(const Uc4Dependencies *dependencies, const char *message)
{
    /* sendFault 콜백 위임. */
    bool commandSent = false;

    if ((dependencies != (const Uc4Dependencies *)0) &&
        (dependencies->sendFault != (void *)0) &&
        (message != (const char *)0)) {
        commandSent = dependencies->sendFault(message, dependencies->context);
    }

    return commandSent;
}

/**
 * @brief Cluster/HMI로부터 ACK 수신 여부와 메시지 표시 성공 여부를 확인한다.
 * @param dependencies 외부 의존성 구조체 포인터
 * @param ackReceived ACK 수신 여부 출력 파라미터
 * @param displaySuccess 메시지 표시 성공 여부 출력 파라미터
 * @return checkSuccess 확인 동작 수행 성공 여부
 */
bool checkClusterHmiAck(const Uc4Dependencies *dependencies, bool *ackReceived, bool *displaySuccess)
{
    /* 출력 파라미터 NULL 시 결과 전달 불가로 실패 처리. */
    bool checkSuccess = false;

    if ((dependencies != (const Uc4Dependencies *)0) &&
        (dependencies->checkClusterHmiAck != (void *)0) &&
        (ackReceived != (bool *)0) &&
        (displaySuccess != (bool *)0)) {
        checkSuccess = dependencies->checkClusterHmiAck(ackReceived, displaySuccess, dependencies->context);
    }

    return checkSuccess;
}

/**
 * @brief ACK 미수신 또는 표시 실패 시 예외를 처리한다.
 * @details 차일드락 제어 기능은 유지되며 진단 로그 기록은 콜백으로 위임한다.
 * @param dependencies 외부 의존성 구조체 포인터
 * @param ackReceived ACK 수신 여부
 * @return errorHandled 오류 처리 완료 여부
 */
bool handleClusterDisplayFailure(const Uc4Dependencies *dependencies, bool ackReceived)
{
    /* 표시 실패는 차일드락 제어에 영향을 주지 않는다. 진단 로그만 기록한다. */
    bool errorHandled = false;

    if ((dependencies != (const Uc4Dependencies *)0) &&
        (dependencies->handleClusterDisplayFailure != (void *)0)) {
        errorHandled = dependencies->handleClusterDisplayFailure(ackReceived, dependencies->context);
    }

    return errorHandled;
}

/**
 * @brief InterventionType에 따라 적절한 메시지 송신 함수를 호출한다.
 * @details UNLOCK_DENIED → displayWarning, EMERGENCY_AUTO_UNLOCK → displayInfo,
 *          SYSTEM_FAULT → displayFault 로 분기한다.
 * @param dependencies 외부 의존성 구조체 포인터
 * @param interventionType 시스템 개입 상황 유형
 * @param message 표시할 메시지 문자열
 * @return commandSent 메시지 송신 성공 여부
 */
static bool dispatchNotificationMessage(
    const Uc4Dependencies *dependencies,
    InterventionType interventionType,
    const char *message)
{
    bool commandSent = false;

    if (interventionType == INTERVENTION_TYPE_UNLOCK_DENIED) {
        commandSent = displayWarning(dependencies, message);
    } else if (interventionType == INTERVENTION_TYPE_EMERGENCY_AUTO_UNLOCK) {
        commandSent = displayInfo(dependencies, message);
    } else {
        /* INTERVENTION_TYPE_SYSTEM_FAULT */
        commandSent = displayFault(dependencies, message);
    }

    return commandSent;
}

/**
 * @brief UC-4 시스템 개입 상황 알림 전체 플로우를 수행하는 오케스트레이터 함수.
 * @details
 *   F1. InterventionType 유효성 확인
 *   F2. 알림 유형 결정
 *   F3~F5. InterventionType에 따라 메시지 종류 분기하여 송신
 *   F6. Cluster/HMI ACK 확인
 *   F7. 실패 시 예외 처리 (차일드락 제어 기능은 유지)
 * @param dependencies 외부 의존성 구조체 포인터
 * @param interventionType 시스템 개입 상황 유형
 * @param message 표시할 메시지 문자열
 * @return Uc4Status 처리 결과 상태 코드
 */
Uc4Status processInterventionNotification(
    const Uc4Dependencies *dependencies,
    InterventionType interventionType,
    const char *message)
{
    Uc4Status status = UC4_STATUS_INVALID_ARGUMENT;

    if ((dependencies != (const Uc4Dependencies *)0) &&
        (message != (const char *)0)) {
        /* F1: 유효한 InterventionType 확인 */
        if (receiveInterventionNotificationRequest(interventionType)) {
            /* F2: 알림 유형 결정 (내부 분기 참조용) */
            (void)determineNotificationType(interventionType);

            /* F3~F5: InterventionType에 따라 메시지 송신 */
            if (dispatchNotificationMessage(dependencies, interventionType, message)) {
                /* F6: Cluster/HMI ACK 확인 */
                bool ackReceived = false;
                bool displaySuccess = false;

                if (checkClusterHmiAck(dependencies, &ackReceived, &displaySuccess) &&
                    ackReceived && displaySuccess) {
                    status = UC4_STATUS_OK;
                } else {
                    /* F7: 표시 실패 예외 처리 — 차일드락 제어는 계속 유지 */
                    (void)handleClusterDisplayFailure(dependencies, ackReceived);

                    if (!ackReceived) {
                        status = UC4_STATUS_ACK_TIMEOUT;
                    } else {
                        status = UC4_STATUS_DISPLAY_FAILURE;
                    }
                }
            }
        } else {
            status = UC4_STATUS_INVALID_TYPE;
        }
    }

    return status;
}
