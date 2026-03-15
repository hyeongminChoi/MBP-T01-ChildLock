#ifndef UC3_CHILD_LOCK_STATUS_DISPLAY_H
#define UC3_CHILD_LOCK_STATUS_DISPLAY_H

#include <stdbool.h>

#include "child_lock_types.h"

/*
 * UC-3 dependency contract.
 * External I/O is abstracted for unit-test isolation.
 * Each callback returns true on successful call execution.
 */
typedef struct Uc3DependenciesTag {
    bool (*sendLedCommand)(LedCommand ledCommand, void *context);
    bool (*checkLedAck)(bool *ackReceived, bool *displaySuccess, void *context);
    bool (*handleDisplayFailure)(ChildLockState displayState, bool ackReceived, void *context);
    void *context;
} Uc3Dependencies;

/*
 * UC-3 result status.
 * ACK timeout is separated from display failure for diagnostics.
 */
typedef enum Uc3StatusTag {
    UC3_STATUS_OK = 0,
    UC3_STATUS_INVALID_ARGUMENT = 1,
    UC3_STATUS_ACK_TIMEOUT = 2,
    UC3_STATUS_DISPLAY_FAILURE = 3
} Uc3Status;

/* F1: 상태 표시 요청 수신 */
bool receiveChildLockStatusDisplayRequest(ChildLockState displayState);
/* F2: LED 제어 명령 결정 */
LedCommand determineLedCommand(ChildLockState displayState);
/* F3: LED 제어 명령 송신 */
bool sendLedCommandToDoorPanel(const Uc3Dependencies *dependencies, LedCommand ledCommand);
/* F4: 표시 반영 결과 확인 */
bool checkDoorPanelLedAck(const Uc3Dependencies *dependencies, bool *ackReceived, bool *displaySuccess);
/* F5: 표시 실패 예외 처리 */
bool handleLedDisplayFailure(const Uc3Dependencies *dependencies, ChildLockState displayState, bool ackReceived);
/* UC-3 end-to-end orchestration */
Uc3Status processChildLockStatusDisplayRequest(
    const Uc3Dependencies *dependencies, ChildLockState displayState);

#endif
