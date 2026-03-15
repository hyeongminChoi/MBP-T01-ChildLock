#ifndef UC2_EMERGENCY_RELEASE_H
#define UC2_EMERGENCY_RELEASE_H

#include <stdbool.h>

#include "child_lock_types.h"

/*
 * UC-2 persistent state container.
 * Stored state is the last confirmed state from rear door module.
 */
typedef struct Uc2ChildLockControllerTag {
    ChildLockState currentState;
} Uc2ChildLockController;

/*
 * UC-2 external dependencies.
 * Rear Door Module, UC-3, and UC-4 are abstracted as callbacks
 * for decoupled unit testing and hardware independence.
 */
typedef struct Uc2DependenciesTag {
    bool (*applyStateToRearDoorModule)(ChildLockState targetState, void *context);
    bool (*checkRearDoorModuleAck)(bool *ackReceived, bool *applySuccess, void *context);
    bool (*requestChildLockLedStatusDisplay)(ChildLockState finalState, void *context);
    bool (*requestEmergencyUnlockNotification)(ChildLockState finalState, void *context);
    bool (*handleRearDoorApplyFailure)(ChildLockState previousState, bool ackReceived, void *context);
    void *context;
} Uc2Dependencies;

/*
 * UC-2 orchestration result.
 * Each failure path is named separately for diagnostics.
 */
typedef enum Uc2StatusTag {
    UC2_STATUS_OK                        = 0,
    UC2_STATUS_NO_REQUEST                = 1,
    UC2_STATUS_INVALID_ARGUMENT          = 2,
    UC2_STATUS_ALREADY_UNLOCKED          = 3,
    UC2_STATUS_ACK_TIMEOUT               = 4,
    UC2_STATUS_APPLY_FAILURE             = 5,
    UC2_STATUS_LED_DISPLAY_REQUEST_FAILED = 6
} Uc2Status;

/* F1: 비상 해제 요청 수신 */
bool receiveEmergencyUnlockRequest(int crashSignal);
/* F2: 현재 차일드락 상태 조회 */
ChildLockState getUc2CurrentChildLockState(const Uc2ChildLockController *controller);
/* F3: 상태 변경 여부 결정 */
bool determineEmergencyAction(ChildLockState currentState);
/* F4: 차일드락 강제 해제 처리 */
ChildLockState forceChildLockOff(void);
/* F5: Rear Door Module 상태 반영 요청 */
bool applyUc2StateToRearDoorModule(const Uc2Dependencies *dependencies, ChildLockState targetState);
/* F6: Rear Door Module ACK/반영 확인 */
bool checkUc2RearDoorModuleAck(const Uc2Dependencies *dependencies, bool *ackReceived, bool *applySuccess);
/* F7: 차일드락 상태 저장 */
bool saveUc2ChildLockState(Uc2ChildLockController *controller, ChildLockState finalState);
/* F8: UC-3 LED 상태 표시 요청 */
bool requestUc2LedStatusDisplay(const Uc2Dependencies *dependencies, ChildLockState finalState);
/* F9: UC-4 비상 해제 알림 요청 */
bool requestEmergencyUnlockNotification(const Uc2Dependencies *dependencies, ChildLockState finalState);
/* F10: 반영 실패 예외 처리 */
bool handleUc2RearDoorApplyFailure(const Uc2Dependencies *dependencies, ChildLockState previousState, bool ackReceived);
/* UC-2 end-to-end orchestration */
Uc2Status processEmergencyUnlockRequest(
    Uc2ChildLockController *controller, const Uc2Dependencies *dependencies, int crashSignal);

#endif
