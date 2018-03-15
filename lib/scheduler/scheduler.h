/*
 *  Module: Scheduler
 *
 *  A task scheduling system for the Arduino.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2017 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "utils.h"

START_C_SECTION

/*
 *  Scheduler Constants & Macro Functions
 */

/*
 * Constant: SCHEDULER_MINIMUM_PERIOD
 *  The minimum period for periodically called functions.  This
 *  is used to prevent thashing
 */
#define SCHEDULER_MINIMUM_PERIOD 500

/*
 * Constant Family: TASK_EXIT_*
 *  A set of constants which specify scheduler recognized exit
 *  codes and a range of allowed custom exit codes.
 */
/* Exit without issues */
#define TASK_EXIT_OK                0x00
/* Exit with issues, triggeres event */
#define TASK_EXIT_FAILED            0x01
/* Exit without rescheduling.
 * Only effects periodic & event-based tasks
 */
#define TASK_EXIT_NO_RESCHEDULE     0x02
/* Exit, and reschedule to exicute again. */
#define TASK_EXIT_RESCHEDULE_NOW    0x03
/* Lower priority (priority+1) for future calls.
 * Only effects periodic & event-based tasks
 */
#define TASK_EXIT_LOWER_PRIORITY    0x04
/* Raises priority (priority-1) for future calls.
 * Only effects periodic & event-based tasks
 */
#define TASK_EXIT_RAISE_PRIORITY    0x05
/* Reserved: 0x06 to 0x0F */

/* Beginning of custom exit code range.
 *  This exit codes can be quiried using scheduler_exit_code_of().
 *  Only stores exit codes of periodic and event-based
 *  tasks.
 */
#define TASK_EXIT_START_CUSTOM      0x10

/* End of custom exit code range. */
#define TASK_EXIT_END_CUSTOM        0xFE
#define TASK_EXIT_NO_CODE           0xFF

/*
 * Macro Function: TASK_EXIT_CUSTOM_CODE
 *  Used to define a custom exit code for a task.
 */
#define TASK_EXIT_CUSTOM_CODE(code) ((code) + TASK_EXIT_START_CUSTOM)

/*
 * Constant: TASK_PRIORITY_LOWEST
 *  A constant representing the lowest priority
 *  possible.
 */
#define TASK_PRIORITY_LOWEST        0xFF

/*
 * Constant: TASK_PRIORITY_HIGHEST
 *  A constant representing the highest priority
 *  possible.
 */
#define TASK_PRIORITY_HIGHEST       0x00

#define SCHEDULER_ID_MASK 0x7F
#define SCHEDULER_INVALID_ID -1
#define SCHEDULER_ID_IS_VALID(task_id) \
    (((task_id) & SCHEDULER_ID_MASK) == (task_id))


/*
 * Constant: SCHEDULER_EVENT_TASK_FAILED
 *  A reserved task event flag.  This event
 *  is triggered if a task fails.
 */
#define SCHEDULER_EVENT_TASK_FAILED 0x01

/*
 *  Scheduler Type Definitions.
 */

/*
 * Typedef: task_id_t
 *  The ID given to a scheduled task.  Can be used to modify tasks
 *  which are scheduled.  Any negative task ID is treated as an error
 *  code.
 */
typedef int8_t task_id_t;

/*
 * Typedef: event_mask_t
 *  The event mask is used when scheduling "on-event" tasks.  Allows
 *  tasks to be triggered by more than one task.
 */
typedef uint16_t event_mask_t;

/*
 * Typedef task_t
 *  Function pointer type of a valid task.
 *
 * Parameters:
 *  arg - A pointer to data which was specified when the task was
 *          scheduled.
 *  Returns:
 *      Task exit code.  See standard exit codes and practice for
 *      custom exit codes.
 */
typedef uint8_t (*task_t) (void * arg);

/*
 * Typedef event_task_t
 *  Function pointer type of a valid event-based task.
 */
typedef uint8_t (*event_task_t) (event_mask_t event_mask, void * arg);

/*
 *  Task Scheduling Functions
 */

/*
 * Function scheduler_periodic_callback
 *  Schedules a task to be called periodically based on priority.
 *  The amount of time between calls can be set using the arguement
 *  `period_micros` which specifies the call period in micro seconds.
 *  This value must be at least SCHEDULER_MINIMUM_PERIOD.  If the
 *  period given is less than SCHEDULER_MINIMUM_PERIOD, it will be
 *  rounded up.
 *
 *  The exit code of the most recently completed execution of this
 *  task is stored until the task is removed from the scheduler.
 *
 * Parameters:
 *  priority - The priority the task get amoung other simultaneously
 *              scheduled tasks.  0 is highest, 255 is lowest.
 *  period_micros - The task call period in micro seconds.
 *  task_pointer - A function pointer to task.
 *  data - An optional data pointer which will be passed to the task
 *          when called.  Can be set to NULL.
 *
 * Returns:
 *  If task is scheduled without issues, then the task id of the
 *  newly scheduled task is returned.  Otherwise, -1 is returned
 *  and the task was not scheduled.
 */
task_id_t scheduler_periodic_callback(
    uint8_t priority,
    uint32_t period_micros,
    task_t task_pointer,
    void * data);

/*
 * Function scheduler_delayed_callback
 *  Schedules a task to be called once after a specified delay.
 *  If the specified delay time is 0, then the call is identical
 *  to scheduler_immediate_callback().
 *
 * Parameters:
 *  priority - The priority the task get amoung other simultaneously
 *              scheduled tasks.  0 is highest, 255 is lowest.
 *  delay_micros - Delay time of calling task in microseconds.
 *  task_pointer - A function pointer to task.
 *  data - An optional data pointer which will be passed to the task
 *          when called.  Can be set to NULL.
 *
 * Returns:
 *  If task is scheduled without issues, then the task id of the
 *  newly scheduled task is returned.  Otherwise, -1 is returned
 *  and the task was not scheduled.
 */
task_id_t scheduler_delayed_callback(
    uint8_t priority,
    uint32_t delay_micros,
    task_t task_pointer,
    void * data);


/*
 * Function scheduler_immediate_callback
 *  Schedules a task to be called once as soon as possible, but
 *  not until the current task completes (if called from a task).
 *
 * Parameters:
 *  priority - The priority the task get amoung other simultaneously
 *              scheduled tasks.  0 is highest, 255 is lowest.
 *  task_pointer - A function pointer to task.
 *  data - An optional data pointer which will be passed to the task
 *          when called.  Can be set to NULL.
 *
 * Returns:
 *  If task is scheduled without issues, then the task id of the
 *  newly scheduled task is returned.  Otherwise, -1 is returned
 *  and the task was not scheduled.
 */
task_id_t scheduler_immediate_callback(
    uint8_t priority,
    task_t task_pointer,
    void * data);

/*
 * Function scheduler_on_event_callback
 *  Schedules a task to be called when the specified event(s) occur.
 *  The event mask can specify several events by bitwise ORing the
 *  event masks together.
 *
 *  The scheduled task must be of type event_task_t, which
 *  accepts a bit masks of the current events.
 *
 *  If the task is to be triggered by multiple events, and one or more
 *  triggering events occur simultaneously, the event task is only
 *  called once.
 *
 * Parameters:
 *  priority - The priority the task get amoung other simultaneously
 *              scheduled tasks.  0 is highest, 255 is lowest.
 *  event_mask - A bit masks of events to trigger the task.
 *  task_pointer - A function pointer to event task.
 *  data - An optional data pointer which will be passed to the task
 *          when called.  Can be set to NULL.
 *
 * Returns:
 *  If task is scheduled without issues, then the task id of the
 *  newly scheduled task is returned.  Otherwise, -1 is returned
 *  and the task was not scheduled.
 */
task_id_t scheduler_on_event_callback(
    uint8_t priority,
    event_mask_t event_mask,
    event_task_t task_pointer,
    void * data);


/*
 * Function scheduler_on_event_callback_without_mask
 *  Schedules a task to be called when the specified event(s) occur.
 *  The event mask can specify several events by bitwise ORing the
 *  event masks together.
 *
 *  If the task is to be triggered by multiple events, and one or more
 *  triggering events occur simultaneously, the event task is only
 *  called once.
 *
 * Parameters:
 *  priority - The priority the task get amoung other simultaneously
 *              scheduled tasks.  0 is highest, 255 is lowest.
 *  event_mask - A bit masks of events to trigger the task.
 *  task_pointer - A function pointer to task.
 *  data - An optional data pointer which will be passed to the task
 *          when called.  Can be set to NULL.
 *
 * Returns:
 *  If task is scheduled without issues, then the task id of the
 *  newly scheduled task is returned.  Otherwise, -1 is returned
 *  and the task was not scheduled.
 */
task_id_t scheduler_on_event_callback_without_mask(
    uint8_t priority,
    event_mask_t event_mask,
    task_t task_pointer,
    void * data);

/*
 * Function: scheduler_remove
 *  Removes the specified task (by ID) to be removed from the
 *  scheduler.  If the current task is the one being requested
 *  to be removed, the current task will need to finish
 *  executing before it is removed.
 *
 *  If the given task id is invalid, or it does not specify
 *  a scheduled task, no effect will occur.
 *
 *  WARNING: Tasks IDs may be reused after task is removed from
 *  the scheduler.  Do not call this unless you know the task ID
 *  you are specifying is your task.
 *
 * Parameters:
 *  id - The task id of the task to be removed.
 */
void scheduler_remove(task_id_t id);

/*
 * Function: scheduler_trigger_event
 *  Sets the event trigger mask in the scheduler.  Tasks which
 *  are to be called upon the event will be scheduled to execute.
 *  Event mask is reset after event are triggered.
 *
 * Parameters:
 *  event_mask - A bit mask of events to be triggered.
 */
void scheduler_trigger_event(event_mask_t event_mask);

/*
 *  Task Utilities
 */

/*
 * Function: scheduler_current_task_id
 *  Returns the task id of the currently executing task.  If
 *  no task is currently executing, then -1 is returned.
 *
 * Returns:
 *  The task id of the current task, or -1 if no task is executing.
 */
task_id_t scheduler_current_task_id(void);

/*
 * Function: scheduler_last_task_id
 *  Returns the task id of the most recently completed task.
 *  If no tasks has executed yet, or the last task was removed
 *  from the scheduler, then -1 is returned.
 *
 * Returns:
 *  The task id of the most recently complete task if available,
 *  or -1 if id is unavailable.
 */
task_id_t scheduler_last_task_id(void);

/*
 * Function: scheduler_exit_code_of
 *  Get the stored exit code of the most recent execution of
 *  a periodically or event-based tasks.  The exit codes of
 *  non-reocurring or removed tasks are not stored.
 *
 * Parameters
 *  The task id of the task exit code to be retreived.
 *
 * Returns:
 *  Returns the exit code if it is available, otherwise returns
 *  TASK_EXIT_NO_CODE.
 */
uint8_t scheduler_exit_code_of(task_id_t task_id);

/*
 *  Scheduler Module Initialize and Loop
 */

/*
 * Function: scheduler_init
 *  Initializes the scheduler module.  Should be called
 *  before a call to scheduler_loop().
 */
void scheduler_init(void);

/*
 * Function: scheduler_loop
 *  The main task look of the scheduler.
 */
void scheduler_loop(void);

END_C_SECTION

#endif /* _SCHEDULER_H_ */
