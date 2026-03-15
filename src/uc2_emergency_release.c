/**
 * @file uc2_emergency_release.c
 * @brief UC-2 비상 상황 자동 해제 모듈
 * @details 차량 충돌 또는 에어백 전개 신호 수신 시 차일드락을 자동 해제한다.
 *          Primary Actor 없음. Crash/Airbag System이 트리거 역할을 수행한다.
 * @author harvy1070
 * @date 2026-03-15
 * @note AI 구현 도구: Claude (claude-sonnet-4-6) | 구현일: 2026-03-15
 * @note 기준 문서: SwRS_v2.md UC-2, SwDD UC-2, uc2-flowchart.puml
 */

#include "uc2_emergency_release.h"

/**
 * @brief 비상 상황 자동 해제 요청을 수신한다.
 * @param crashSignal 비상 이벤트 신호 (1 = 발생, 0 = 미발생)
 * @return requestReceived 요청 수신 성공 여부
 */
bool receiveEmergencyUnlockRequest(int crashSignal)
{
    /* crashSignal 1 은 비상 이벤트 발생을 의미한다. */
    bool requestReceived = false;

    if (crashSignal == 1) {
        requestReceived = true;
    }

    return requestReceived;
}

/**
 * @brief 현재 차일드락 상태를 조회한다.
 * @param controller 차일드락 상태 컨테이너 포인터
 * @return currentState 현재 차일드락 상태 (controller NULL 시 fail-safe로 OFF 반환)
 */
ChildLockState getUc2CurrentChildLockState(const Uc2ChildLockController *controller)
{
    /* controller 가 NULL 이면 OFF 를 반환하여 불필요한 해제 동작을 방지한다. */
    ChildLockState currentState = CHILD_LOCK_STATE_OFF;

    if (controller != (const Uc2ChildLockController *)0) {
        currentState = controller->currentState;
    }

    return currentState;
}

/**
 * @brief 현재 상태를 기준으로 비상 해제 동작이 필요한지 판단한다.
 * @param currentState 현재 차일드락 상태
 * @return actionNeeded ON 상태일 때만 true 반환
 */
bool determineEmergencyAction(ChildLockState currentState)
{
    /* 이미 OFF 이면 추가 동작이 불필요하다. */
    bool actionNeeded = false;

    if (currentState == CHILD_LOCK_STATE_ON) {
        actionNeeded = true;
    }

    return actionNeeded;
}

/**
 * @brief 차일드락을 강제 해제 상태(OFF)로 결정한다.
 * @return targetState CHILD_LOCK_STATE_OFF
 */
ChildLockState forceChildLockOff(void)
{
    /* 비상 상황에서는 항상 OFF 가 목표 상태이다. */
    ChildLockState targetState = CHILD_LOCK_STATE_OFF;
    return targetState;
}

/**
 * @brief 결정된 차일드락 상태를 Rear Door Module에 반영 요청한다.
 * @param dependencies 외부 의존성 구조체 포인터
 * @param targetState 적용할 차일드락 상태
 * @return applyRequestSent 요청 송신 성공 여부
 */
bool applyUc2StateToRearDoorModule(const Uc2Dependencies *dependencies, ChildLockState targetState)
{
    /* 콜백이 없으면 요청 자체가 불가능하다. */
    bool applyRequestSent = false;

    if ((dependencies != (const Uc2Dependencies *)0) &&
        (dependencies->applyStateToRearDoorModule != (void *)0)) {
        applyRequestSent = dependencies->applyStateToRearDoorModule(targetState, dependencies->context);
    }

    return applyRequestSent;
}

/**
 * @brief Rear Door Module로부터 ACK 수신 여부와 상태 반영 성공 여부를 확인한다.
 * @param dependencies 외부 의존성 구조체 포인터
 * @param ackReceived ACK 수신 여부 출력 파라미터
 * @param applySuccess 상태 반영 성공 여부 출력 파라미터
 * @return checkSuccess 확인 동작 수행 성공 여부
 */
bool checkUc2RearDoorModuleAck(const Uc2Dependencies *dependencies, bool *ackReceived, bool *applySuccess)
{
    /* 출력 파라미터가 NULL 이면 결과를 전달할 수 없으므로 실패로 처리한다. */
    bool checkSuccess = false;

    if ((dependencies != (const Uc2Dependencies *)0) &&
        (dependencies->checkRearDoorModuleAck != (void *)0) &&
        (ackReceived != (bool *)0) &&
        (applySuccess != (bool *)0)) {
        checkSuccess = dependencies->checkRearDoorModuleAck(ackReceived, applySuccess, dependencies->context);
    }

    return checkSuccess;
}

/**
 * @brief 최종 확정된 차일드락 상태를 내부 상태값에 저장한다.
 * @param controller 차일드락 상태 컨테이너 포인터
 * @param finalState 저장할 최종 확정 상태
 * @return saveSuccess 저장 성공 여부
 */
bool saveUc2ChildLockState(Uc2ChildLockController *controller, ChildLockState finalState)
{
    /* controller 가 NULL 이면 저장 자체가 불가능하다. */
    bool saveSuccess = false;

    if (controller != (Uc2ChildLockController *)0) {
        controller->currentState = finalState;
        saveSuccess = true;
    }

    return saveSuccess;
}

/**
 * @brief UC-3을 호출하여 도어 패널 LED에 차일드락 상태를 표시 요청한다.
 * @param dependencies 외부 의존성 구조체 포인터
 * @param finalState 표시할 차일드락 상태
 * @return displayRequestSent 표시 요청 송신 성공 여부
 */
bool requestUc2LedStatusDisplay(const Uc2Dependencies *dependencies, ChildLockState finalState)
{
    /* UC-3 콜백 위임. 콜백 미등록 시 표시 요청 불가. */
    bool displayRequestSent = false;

    if ((dependencies != (const Uc2Dependencies *)0) &&
        (dependencies->requestChildLockLedStatusDisplay != (void *)0)) {
        displayRequestSent =
            dependencies->requestChildLockLedStatusDisplay(finalState, dependencies->context);
    }

    return displayRequestSent;
}

/**
 * @brief UC-4를 호출하여 비상 상황 자동 해제 알림을 클러스터/HMI에 요청한다.
 * @param dependencies 외부 의존성 구조체 포인터
 * @param finalState 현재 차일드락 상태
 * @return notificationRequested 알림 요청 성공 여부
 */
bool requestEmergencyUnlockNotification(const Uc2Dependencies *dependencies, ChildLockState finalState)
{
    /* UC-4 콜백 위임. 운전자에게 자동 해제 사실을 안내한다. */
    bool notificationRequested = false;

    if ((dependencies != (const Uc2Dependencies *)0) &&
        (dependencies->requestEmergencyUnlockNotification != (void *)0)) {
        notificationRequested =
            dependencies->requestEmergencyUnlockNotification(finalState, dependencies->context);
    }

    return notificationRequested;
}

/**
 * @brief Rear Door Module 반영 실패 또는 ACK 미수신 시 예외를 처리한다.
 * @param dependencies 외부 의존성 구조체 포인터
 * @param previousState 변경 전 차일드락 상태
 * @param ackReceived ACK 수신 여부
 * @return errorHandled 오류 처리 완료 여부
 */
bool handleUc2RearDoorApplyFailure(
    const Uc2Dependencies *dependencies, ChildLockState previousState, bool ackReceived)
{
    /* 실패 처리는 콜백으로 위임한다. 콜백 미등록 시에도 false 반환으로 상위에 알린다. */
    bool errorHandled = false;

    if ((dependencies != (const Uc2Dependencies *)0) &&
        (dependencies->handleRearDoorApplyFailure != (void *)0)) {
        errorHandled =
            dependencies->handleRearDoorApplyFailure(previousState, ackReceived, dependencies->context);
    }

    return errorHandled;
}

/**
 * @brief 비상 해제 후 Rear Door Module에 상태를 반영하고 최종 결과를 커밋한다.
 * @details applySuccess 확인 후 상태 저장 → LED 표시 → 알림 요청 순서로 진행한다.
 *          실패 시 이전 상태를 복원하고 실패 원인에 따른 상태 코드를 반환한다.
 * @param controller 차일드락 상태 컨테이너 포인터
 * @param dependencies 외부 의존성 구조체 포인터
 * @param previousState Rear Door Module 반영 전 상태 (복원 기준)
 * @param targetState 반영할 목표 상태
 * @return Uc2Status 처리 결과 상태 코드
 */
static Uc2Status commitEmergencyState(
    Uc2ChildLockController *controller,
    const Uc2Dependencies *dependencies,
    ChildLockState previousState,
    ChildLockState targetState)
{
    Uc2Status status = UC2_STATUS_APPLY_FAILURE;
    bool ackReceived = false;
    bool applySuccess = false;

    if (applyUc2StateToRearDoorModule(dependencies, targetState) &&
        checkUc2RearDoorModuleAck(dependencies, &ackReceived, &applySuccess) &&
        ackReceived && applySuccess) {
        /* 반영 성공: 상태 저장 → LED 표시 → 비상 해제 알림 */
        (void)saveUc2ChildLockState(controller, targetState);
        (void)requestUc2LedStatusDisplay(dependencies, targetState);

        if (requestEmergencyUnlockNotification(dependencies, targetState)) {
            status = UC2_STATUS_OK;
        } else {
            status = UC2_STATUS_LED_DISPLAY_REQUEST_FAILED;
        }
    } else {
        /* 반영 실패: 이전 상태 복원 후 실패 처리 */
        (void)saveUc2ChildLockState(controller, previousState);
        (void)handleUc2RearDoorApplyFailure(dependencies, previousState, ackReceived);

        if (!ackReceived) {
            status = UC2_STATUS_ACK_TIMEOUT;
        }
    }

    return status;
}

/**
 * @brief UC-2 비상 상황 자동 해제 전체 플로우를 수행하는 오케스트레이터 함수.
 * @details
 *   F1. crashSignal 수신 확인
 *   F2. 현재 차일드락 상태 조회
 *   F3. 상태 변경 필요 여부 판단
 *   F4. 이미 OFF 이면 추가 동작 없이 종료 (A1)
 *   F5~F10. ON 상태일 때 강제 해제 → 반영 → 저장 → 표시 → 알림 수행
 * @param controller 차일드락 상태 컨테이너 포인터
 * @param dependencies 외부 의존성 구조체 포인터
 * @param crashSignal 비상 이벤트 신호 (1 = 발생)
 * @return Uc2Status 처리 결과 상태 코드
 */
Uc2Status processEmergencyUnlockRequest(
    Uc2ChildLockController *controller, const Uc2Dependencies *dependencies, int crashSignal)
{
    Uc2Status status = UC2_STATUS_INVALID_ARGUMENT;

    if ((controller != (Uc2ChildLockController *)0) &&
        (dependencies != (const Uc2Dependencies *)0)) {
        /* F1: 비상 신호 수신 확인 */
        if (receiveEmergencyUnlockRequest(crashSignal)) {
            /* F2: 현재 상태 조회 */
            ChildLockState currentState = getUc2CurrentChildLockState(controller);

            /* F3: 상태 변경 필요 여부 판단 */
            if (determineEmergencyAction(currentState)) {
                /* F4: ON 상태 → 강제 해제 목표 상태 결정 후 커밋 */
                ChildLockState targetState = forceChildLockOff();
                status = commitEmergencyState(controller, dependencies, currentState, targetState);
            } else {
                /* A1: 이미 OFF 상태 → 추가 동작 불필요 */
                status = UC2_STATUS_ALREADY_UNLOCKED;
            }
        } else {
            status = UC2_STATUS_NO_REQUEST;
        }
    }

    return status;
}
