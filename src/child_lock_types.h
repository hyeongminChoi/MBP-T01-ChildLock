#ifndef CHILD_LOCK_TYPES_H
#define CHILD_LOCK_TYPES_H

#include <stdbool.h>

/*
 * Shared enum definitions for UC-1 and UC-3.
 * The values are intentionally explicit for stable logging.
 * This header is dependency-free and safe for C/C++ include.
 */

/* Child lock state used by UC-1 and UC-3. */
typedef enum ChildLockStateTag {
    CHILD_LOCK_STATE_ON = 0,
    CHILD_LOCK_STATE_OFF = 1
} ChildLockState;

/* UC-1 action derived from current child lock state. */
typedef enum ChildLockActionTag {
    CHILD_LOCK_ACTION_LOCK = 0,
    CHILD_LOCK_ACTION_UNLOCK = 1
} ChildLockAction;

/* UC-3 LED command mapped from child lock state. */
typedef enum LedCommandTag {
    LED_COMMAND_ON = 0,
    LED_COMMAND_OFF = 1
} LedCommand;

#endif
