#include "uc3_child_lock_status_display.h"

/*
 * Validate that incoming state is one of the defined enum values.
 * This blocks invalid casted integers from being processed silently.
 */
static bool isValidState(ChildLockState state)
{
    bool isValid = false;

    if ((state == CHILD_LOCK_STATE_ON) || (state == CHILD_LOCK_STATE_OFF)) {
        isValid = true;
    }

    return isValid;
}

bool receiveChildLockStatusDisplayRequest(ChildLockState displayState)
{
    /* Request is accepted only for valid OFF/ON values. */
    bool requestReceived = false;

    if (isValidState(displayState)) {
        requestReceived = true;
    }

    return requestReceived;
}

LedCommand determineLedCommand(ChildLockState displayState)
{
    /* OFF is default for fail-safe behavior. */
    LedCommand ledCommand = LED_COMMAND_OFF;

    if (displayState == CHILD_LOCK_STATE_ON) {
        ledCommand = LED_COMMAND_ON;
    }

    return ledCommand;
}

bool sendLedCommandToDoorPanel(const Uc3Dependencies *dependencies, LedCommand ledCommand)
{
    /* Callback dependency keeps hardware communication replaceable. */
    bool commandSent = false;

    if ((dependencies != (const Uc3Dependencies *)0) && (dependencies->sendLedCommand != (void *)0)) {
        commandSent = dependencies->sendLedCommand(ledCommand, dependencies->context);
    }

    return commandSent;
}

bool checkDoorPanelLedAck(const Uc3Dependencies *dependencies, bool *ackReceived, bool *displaySuccess)
{
    /* ACK and display flags are output parameters from the panel interface. */
    bool checkSuccess = false;

    if ((dependencies != (const Uc3Dependencies *)0) &&
        (dependencies->checkLedAck != (void *)0) &&
        (ackReceived != (bool *)0) &&
        (displaySuccess != (bool *)0)) {
        checkSuccess = dependencies->checkLedAck(ackReceived, displaySuccess, dependencies->context);
    }

    return checkSuccess;
}

bool handleLedDisplayFailure(const Uc3Dependencies *dependencies, ChildLockState displayState, bool ackReceived)
{
    /* Failure path delegates reporting/logging to injected handler. */
    bool errorHandled = false;

    if ((dependencies != (const Uc3Dependencies *)0) && (dependencies->handleDisplayFailure != (void *)0)) {
        errorHandled = dependencies->handleDisplayFailure(displayState, ackReceived, dependencies->context);
    }

    return errorHandled;
}

Uc3Status processChildLockStatusDisplayRequest(const Uc3Dependencies *dependencies, ChildLockState displayState)
{
    /*
     * UC-3 flow:
     * 1) Request reception check.
     * 2) Command decision and transmission.
     * 3) ACK/display result confirmation.
     * 4) Exception handling on failure.
     */
    Uc3Status status = UC3_STATUS_INVALID_ARGUMENT;

    if (dependencies != (const Uc3Dependencies *)0) {
        /* F1 */
        if (receiveChildLockStatusDisplayRequest(displayState)) {
            /* F2 + F3 */
            LedCommand ledCommand = determineLedCommand(displayState);
            if (sendLedCommandToDoorPanel(dependencies, ledCommand)) {
                /* F4 */
                bool ackReceived = false;
                bool displaySuccess = false;
                if (checkDoorPanelLedAck(dependencies, &ackReceived, &displaySuccess) &&
                    ackReceived && displaySuccess) {
                    status = UC3_STATUS_OK;
                } else {
                    /* F5 */
                    if (handleLedDisplayFailure(dependencies, displayState, ackReceived)) {
                        if (!ackReceived) {
                            status = UC3_STATUS_ACK_TIMEOUT;
                        } else {
                            status = UC3_STATUS_DISPLAY_FAILURE;
                        }
                    }
                }
            }
        }
    }

    return status;
}
