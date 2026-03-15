#ifndef UC4_SYSTEM_INTERVENTION_NOTIFICATION_H
#define UC4_SYSTEM_INTERVENTION_NOTIFICATION_H

#include <stdbool.h>

#include "child_lock_types.h"

/*
 * UC-4 intervention type.
 * Each value maps to a distinct notification message category.
 */
typedef enum InterventionTypeTag {
    INTERVENTION_TYPE_UNLOCK_DENIED        = 0,
    INTERVENTION_TYPE_EMERGENCY_AUTO_UNLOCK = 1,
    INTERVENTION_TYPE_SYSTEM_FAULT         = 2
} InterventionType;

/*
 * UC-4 notification type.
 * Derived from InterventionType via determineNotificationType().
 */
typedef enum NotificationTypeTag {
    NOTIFICATION_TYPE_WARNING = 0,
    NOTIFICATION_TYPE_INFO    = 1,
    NOTIFICATION_TYPE_FAULT   = 2
} NotificationType;

/*
 * UC-4 external dependencies.
 * Cluster/HMI communication is abstracted as callbacks
 * for decoupled unit testing and hardware independence.
 */
typedef struct Uc4DependenciesTag {
    bool (*sendWarning)(const char *message, void *context);
    bool (*sendInfo)(const char *message, void *context);
    bool (*sendFault)(const char *message, void *context);
    bool (*checkClusterHmiAck)(bool *ackReceived, bool *displaySuccess, void *context);
    bool (*handleClusterDisplayFailure)(bool ackReceived, void *context);
    void *context;
} Uc4Dependencies;

/*
 * UC-4 orchestration result.
 * Each failure path is named separately for diagnostics.
 */
typedef enum Uc4StatusTag {
    UC4_STATUS_OK                   = 0,
    UC4_STATUS_INVALID_ARGUMENT     = 1,
    UC4_STATUS_INVALID_TYPE         = 2,
    UC4_STATUS_ACK_TIMEOUT          = 3,
    UC4_STATUS_DISPLAY_FAILURE      = 4
} Uc4Status;

/* F1: 시스템 개입 알림 요청 수신 */
bool receiveInterventionNotificationRequest(InterventionType interventionType);
/* F2: 알림 유형 결정 */
NotificationType determineNotificationType(InterventionType interventionType);
/* F3: 경고 메시지 송신 (UNLOCK_DENIED) */
bool displayWarning(const Uc4Dependencies *dependencies, const char *message);
/* F4: 안내 메시지 송신 (EMERGENCY_AUTO_UNLOCK) */
bool displayInfo(const Uc4Dependencies *dependencies, const char *message);
/* F5: 오류 메시지 송신 (SYSTEM_FAULT) */
bool displayFault(const Uc4Dependencies *dependencies, const char *message);
/* F6: Cluster/HMI ACK/표시 결과 확인 */
bool checkClusterHmiAck(const Uc4Dependencies *dependencies, bool *ackReceived, bool *displaySuccess);
/* F7: 표시 실패 예외 처리 */
bool handleClusterDisplayFailure(const Uc4Dependencies *dependencies, bool ackReceived);
/* UC-4 end-to-end orchestration */
Uc4Status processInterventionNotification(
    const Uc4Dependencies *dependencies, InterventionType interventionType, const char *message);

#endif
