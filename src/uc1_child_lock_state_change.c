#include "uc1_child_lock_state_change.h"

/*
 * F1.
 * Toggle request is currently represented by a boolean flag.
 * False means no operation requested.
 */
bool receiveChildLockToggleRequest(bool toggleRequest)
{
    bool requestReceived = false;

    if (toggleRequest) {
        requestReceived = true;
    }

    return requestReceived;
}

ChildLockState getCurrentChildLockState(const ChildLockController *controller)
{
    /* F2: default OFF is fail-safe if controller is missing. */
    ChildLockState currentState = CHILD_LOCK_STATE_OFF;

    if (controller != (const ChildLockController *)0) {
        currentState = controller->currentState;
    }

    return currentState;
}

ChildLockAction determineChildLockAction(ChildLockState currentState)
{
    /* F3: ON means unlock action, OFF means lock action. */
    ChildLockAction targetAction = CHILD_LOCK_ACTION_LOCK;

    if (currentState == CHILD_LOCK_STATE_ON) {
        targetAction = CHILD_LOCK_ACTION_UNLOCK;
    }

    return targetAction;
}

ChildLockState setChildLockOn(void)
{
    /* F4 */
    ChildLockState targetState = CHILD_LOCK_STATE_ON;
    return targetState;
}

bool validateChildLockReleaseCondition(const Uc1Dependencies *dependencies)
{
    /* F5: delegated to UC-5 callback. */
    bool releaseAllowed = false;

    if ((dependencies != (const Uc1Dependencies *)0) &&
        (dependencies->validateReleaseCondition != (void *)0)) {
        releaseAllowed = dependencies->validateReleaseCondition(dependencies->context);
    }

    return releaseAllowed;
}

ChildLockState setChildLockOff(void)
{
    /* F6 */
    ChildLockState targetState = CHILD_LOCK_STATE_OFF;
    return targetState;
}

ChildLockState keepCurrentChildLockState(ChildLockState currentState)
{
    /* F7 */
    ChildLockState targetState = currentState;
    return targetState;
}

bool requestUnlockRejectedNotification(const Uc1Dependencies *dependencies, ChildLockState currentState)
{
    /* F8: delegated to UC-4 callback. */
    bool notificationRequested = false;

    if ((dependencies != (const Uc1Dependencies *)0) &&
        (dependencies->requestUnlockRejectedNotification != (void *)0)) {
        notificationRequested =
            dependencies->requestUnlockRejectedNotification(currentState, dependencies->context);
    }

    return notificationRequested;
}

bool applyChildLockStateToRearDoorModule(const Uc1Dependencies *dependencies, ChildLockState targetState)
{
    /* F9 */
    bool applyRequestSent = false;

    if ((dependencies != (const Uc1Dependencies *)0) &&
        (dependencies->applyStateToRearDoorModule != (void *)0)) {
        applyRequestSent = dependencies->applyStateToRearDoorModule(targetState, dependencies->context);
    }

    return applyRequestSent;
}

bool checkRearDoorModuleAck(const Uc1Dependencies *dependencies, bool *ackReceived, bool *applySuccess)
{
    /* F10 */
    bool checkSuccess = false;

    if ((dependencies != (const Uc1Dependencies *)0) &&
        (dependencies->checkRearDoorModuleAck != (void *)0) &&
        (ackReceived != (bool *)0) &&
        (applySuccess != (bool *)0)) {
        checkSuccess = dependencies->checkRearDoorModuleAck(ackReceived, applySuccess, dependencies->context);
    }

    return checkSuccess;
}

bool saveChildLockState(ChildLockController *controller, ChildLockState finalState)
{
    /* F11 */
    bool saveSuccess = false;

    if (controller != (ChildLockController *)0) {
        controller->currentState = finalState;
        saveSuccess = true;
    }

    return saveSuccess;
}

bool requestChildLockLedStatusDisplay(const Uc1Dependencies *dependencies, ChildLockState finalState)
{
    /* F12: UC-3 callback hook. */
    bool displayRequestSent = false;

    if ((dependencies != (const Uc1Dependencies *)0) &&
        (dependencies->requestChildLockLedStatusDisplay != (void *)0)) {
        displayRequestSent =
            dependencies->requestChildLockLedStatusDisplay(finalState, dependencies->context);
    }

    return displayRequestSent;
}

ChildLockState handleRearDoorApplyFailure(
    ChildLockState previousState, bool ackReceived, bool *errorHandled)
{
    /*
     * F13.
     * In this phase, the fallback behavior is state retention.
     * Detailed diagnostic routing can be added by UC-4/UC-5 extensions.
     */
    ChildLockState restoredState = previousState;
    bool localHandled = true;

    if (ackReceived) {
        localHandled = true;
    }

    if (errorHandled != (bool *)0) {
        *errorHandled = localHandled;
    }

    return restoredState;
}

Uc1Status processChildLockToggleRequest(
    ChildLockController *controller, const Uc1Dependencies *dependencies, bool toggleRequest)
{
    /*
     * UC-1 flow orchestration.
     * OFF->ON directly.
     * ON->OFF requires validator callback.
     * Rear door confirmation is mandatory before state commit.
     */
    Uc1Status status = UC1_STATUS_INVALID_ARGUMENT;
    bool requestReceived = false;
    ChildLockState previousState = CHILD_LOCK_STATE_OFF;
    ChildLockState targetState = CHILD_LOCK_STATE_OFF;
    ChildLockAction targetAction = CHILD_LOCK_ACTION_LOCK;
    bool releaseAllowed = false;
    bool applyRequestSent = false;
    bool ackReceived = false;
    bool applySuccess = false;
    bool checkDone = false;
    bool errorHandled = false;
    bool displayRequestSent = false;

    if ((controller != (ChildLockController *)0) && (dependencies != (const Uc1Dependencies *)0)) {
        /* F1 */
        requestReceived = receiveChildLockToggleRequest(toggleRequest);
        if (requestReceived) {
            /* F2 + F3 */
            previousState = getCurrentChildLockState(controller);
            targetAction = determineChildLockAction(previousState);
            if (targetAction == CHILD_LOCK_ACTION_LOCK) {
                /* F4 */
                targetState = setChildLockOn();
            } else {
                /* F5 */
                releaseAllowed = validateChildLockReleaseCondition(dependencies);
                if (releaseAllowed) {
                    /* F6 */
                    targetState = setChildLockOff();
                } else {
                    /* F7 + F8 */
                    targetState = keepCurrentChildLockState(previousState);
                    (void)requestUnlockRejectedNotification(dependencies, previousState);
                }
            }

            /* F9 + F10 */
            applyRequestSent = applyChildLockStateToRearDoorModule(dependencies, targetState);
            if (applyRequestSent) {
                checkDone = checkRearDoorModuleAck(dependencies, &ackReceived, &applySuccess);
            }

            if (applyRequestSent && checkDone && ackReceived && applySuccess) {
                /* F11 + F12 */
                (void)saveChildLockState(controller, targetState);
                displayRequestSent = requestChildLockLedStatusDisplay(dependencies, targetState);
                if (displayRequestSent) {
                    status = UC1_STATUS_OK;
                } else {
                    status = UC1_STATUS_LED_DISPLAY_REQUEST_FAILED;
                }
            } else {
                /* F13 */
                (void)handleRearDoorApplyFailure(previousState, ackReceived, &errorHandled);
                (void)saveChildLockState(controller, previousState);
                if (!ackReceived) {
                    status = UC1_STATUS_ACK_TIMEOUT;
                } else {
                    status = UC1_STATUS_APPLY_FAILURE;
                }
            }
        } else {
            status = UC1_STATUS_NO_REQUEST;
        }
    }

    return status;
}
