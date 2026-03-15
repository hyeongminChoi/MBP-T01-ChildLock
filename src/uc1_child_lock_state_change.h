#ifndef UC1_CHILD_LOCK_STATE_CHANGE_H
#define UC1_CHILD_LOCK_STATE_CHANGE_H

#include <stdbool.h>

#include "child_lock_types.h"

/*
 * UC-1 persistent state container.
 * Stored state is the last confirmed state from rear door module.
 */
typedef struct ChildLockControllerTag {
    ChildLockState currentState;
} ChildLockController;

/*
 * UC-1 external dependencies.
 * UC-4/UC-5/UC-3 are represented as callbacks for decoupled testing.
 */
typedef struct Uc1DependenciesTag {
    bool (*validateReleaseCondition)(void *context);
    bool (*requestUnlockRejectedNotification)(ChildLockState currentState, void *context);
    bool (*applyStateToRearDoorModule)(ChildLockState targetState, void *context);
    bool (*checkRearDoorModuleAck)(bool *ackReceived, bool *applySuccess, void *context);
    bool (*requestChildLockLedStatusDisplay)(ChildLockState finalState, void *context);
    void *context;
} Uc1Dependencies;

/*
 * UC-1 orchestration result.
 * ACK timeout and apply failure are separated for diagnostics.
 */
typedef enum Uc1StatusTag {
    UC1_STATUS_OK = 0,
    UC1_STATUS_NO_REQUEST = 1,
    UC1_STATUS_INVALID_ARGUMENT = 2,
    UC1_STATUS_ACK_TIMEOUT = 3,
    UC1_STATUS_APPLY_FAILURE = 4,
    UC1_STATUS_LED_DISPLAY_REQUEST_FAILED = 5
} Uc1Status;

/* F1: 차일드락 상태 변경 요청 수신 */
bool receiveChildLockToggleRequest(bool toggleRequest);
/* F2: 현재 차일드락 상태 조회 */
ChildLockState getCurrentChildLockState(const ChildLockController *controller);
/* F3: 상태 변경 방향 결정 */
ChildLockAction determineChildLockAction(ChildLockState currentState);
/* F4: OFF -> ON 설정 */
ChildLockState setChildLockOn(void);
/* F5: UC-5 해제 조건 검증 요청 */
bool validateChildLockReleaseCondition(const Uc1Dependencies *dependencies);
/* F6: 해제 가능 시 OFF 설정 */
ChildLockState setChildLockOff(void);
/* F7: 해제 불가 시 상태 유지 */
ChildLockState keepCurrentChildLockState(ChildLockState currentState);
/* F8: UC-4 시스템 개입 알림 요청 */
bool requestUnlockRejectedNotification(const Uc1Dependencies *dependencies, ChildLockState currentState);
/* F9: Rear Door Module 상태 반영 요청 */
bool applyChildLockStateToRearDoorModule(const Uc1Dependencies *dependencies, ChildLockState targetState);
/* F10: Rear Door Module ACK/반영 확인 */
bool checkRearDoorModuleAck(const Uc1Dependencies *dependencies, bool *ackReceived, bool *applySuccess);
/* F11: 차일드락 상태 저장 */
bool saveChildLockState(ChildLockController *controller, ChildLockState finalState);
/* F12: UC-3 상태 표시 요청 */
bool requestChildLockLedStatusDisplay(const Uc1Dependencies *dependencies, ChildLockState finalState);
/* F13: 반영 실패 예외 처리 */
ChildLockState handleRearDoorApplyFailure(
    ChildLockState previousState, bool ackReceived, bool *errorHandled);
/* UC-1 end-to-end orchestration */
Uc1Status processChildLockToggleRequest(
    ChildLockController *controller, const Uc1Dependencies *dependencies, bool toggleRequest);

#endif
