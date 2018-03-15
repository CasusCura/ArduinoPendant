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

#include <string.h>
#include <Arduino.h>
#include "scheduler.h"

/*
 *  Scheduler Internal Constants & Macro Functions
 */

#define SCHEDULER_MAX_TASKS 16

/*
 * Constant: TASK_EXIT_RESERVED_MASK
 *  A bit mask flag for exit codes which are reserved.
 */
#define TASK_EXIT_RESERVED_MASK 0x0F

/*
 * Macro Function: TASK_EXIT_IS_RESERVED
 *  Determines wheather an exit code is a reserved
 *  exit code.  Can be used in if statements.
 */
#define TASK_EXIT_IS_RESERVED(exit_code) \
    (((exit_code) & TASK_EXIT_RESERVED_MASK) \
    && !((exit_code) & ~TASK_EXIT_RESERVED_MASK))

/*
 * Constants Family: TASK_FLAG_*
 *  A set of scheduled task flags which specify how the scheduler
 *  is to handle the scheduled task.
 */
/*
 * 0 - Call task as a task_t type
 * 1 - Call task as an event_task_t type
 */
#define TASK_FLAG_EVENT_CALL        0x01
/*
 * 0 - Other trigger type
 * 1 - Triggered by time lapse
 */
#define TASK_FLAG_TIME_TRIGGER      0x02
/*
 * 0 - Other trigger type
 * 1 - Triggered by events
 */
#define TASK_FLAG_EVENT_TRIGGER     0x04
/*
 * 0 - Non-Periodic Trigger
 * 1 - Periodic Trigger
 */
#define TASK_FLAG_PERIODIC          0x08
/*
 * 0 - Task is not queued
 * 1 - Task is queued
 */
#define TASK_FLAG_IN_QUEUE          0x40
/*
 * 0 - No meaning
 * 1 - Remove after next execution
 */
#define TASK_FLAG_REMOVE            0x80

/*
 *  Scheduler Internal Type Definitions
 */

typedef uint8_t task_flags_t;


/*
 * Structure: scheduled_task_t
 *
 *  Represents a scheduled task.
 *
 *  task_id - Task ID is a unique amounst the other scheduled tasks.
 *  priority - A relative priority of the given task.
 *  flags - Bitmask of several flags for task meta data.
 *  task_function - The function to call upon execution.  Requires the
 *                  TASK_FLAG_EVENT_CALL flag to be false.
 *  event_task_function - The function to call upon execution, passing
 *                        the event flag.  Requires TASK_FLAG_EVENT_CALL
 *                        flag to be true.
 *  period - The period which the task is to be called.  Requires
 *           the TASK_FLAG_EVENT_PERIODIC flag to be set.
 *  next_execution - The system time of the next execution.  Requires
 *                   TASK_FLAG_TIME_TRIGGER flag to be set.
 *  event_mask - Bitmasks of events which cause a function to be
 *               triggered.  Requires TASK_FLAG_EVENT_TRIGGER flag to
 *               be set.
 *  trigger_event_mask - A bit masked of the currently triggered event bits
 *                       of the task.  Cleared after execution.
 *  data - Void pointer to task argument data.
 *  exit_code - The exit code of the task on its most recently
 *              completed execution.
 */
typedef struct {
    task_id_t task_id;
    uint8_t priority;
    /* Stores Internal Properties about A Task  */
    task_flags_t flags;
    union /* Function Type */
    {
        task_t task_function;
        event_task_t event_task_function;
    };
    union /* Trigger Info */
    {
        struct
        {
            time_t period;
            time_t next_execution;
        };
        struct
        {
            event_mask_t event_mask;
            event_mask_t trigger_event_mask;
        };
    };
    void * data;
    uint8_t exit_code;
} scheduled_task_t;

/*
 * Typedef: scheduled_task_ref_t
 *  Reference to a scheduled task (scheduled_task_t)
 */
typedef scheduled_task_t * scheduled_task_ref_t;

/*
 * Structure: task_schedule_t
 *  Represents a task schedule.
 */
typedef struct {
    uint8_t size;
    task_id_t last_id;
    scheduled_task_t scheduled_tasks[SCHEDULER_MAX_TASKS];
} task_schedule_t;

/*
 * Typedef: task_schedule_ref_t
 *  Reference to a task schedule (task_schedule_t)
 */
typedef task_schedule_t * task_schedule_ref_t;


/*
 * Structure: task_queue_t
 *  Represents a task queue.
 */
typedef struct {
    uint8_t size;
    scheduled_task_ref_t queued_tasks[SCHEDULER_MAX_TASKS];
} task_queue_t;

/*
 * Typedef: task_queue_ref_t
 *  Reference to a task queue (task_queue_t)
 */
typedef task_queue_t * task_queue_ref_t;

typedef struct {
    task_schedule_t task_schedule;
    task_queue_t task_queue;
    scheduled_task_ref_t current_task;
    scheduled_task_ref_t last_task;
    event_mask_t current_events;
    time_t last_scheduling;
    time_t next_scheduling;
} scheduler_t;

/*
 * Typedef: scheduler_ref_t
 *  Reference to a scheduler (scheduler_t)
 */
typedef scheduler_t * scheduler_ref_t;

/*
 *  Scheduler Internal Helper Functions
 */

static inline time_t system_time(void)
{
    return micros();
}

static inline void disable_interrupts(void)
{
    noInterrupts();
}

static inline void enabled_interrupts(void)
{
    interrupts();
}

static bool micro_time_is_in_range(
    time_t check_time,
    time_t lower_bound,
    time_t upper_bound)
{
    /* Check if non-overflow range. */
    if (lower_bound < upper_bound)
    {
        return check_time >= lower_bound && check_time <= upper_bound;
    }
    else /* Overflow from upper_bound to lower_bound */
    {
        return check_time >= lower_bound || check_time <= upper_bound;
    }
}

/*
 *  Scheduler Internal Functions Prototypes.
 */

static void scheduler_queue_init(task_queue_ref_t task_queue);
static bool scheduler_queue_enqueue(
    task_queue_ref_t task_queue,
    scheduled_task_ref_t new_task);
static bool scheduler_queue_remove(
    task_queue_ref_t task_queue,
    task_id_t task_id);
static scheduled_task_ref_t scheduler_queue_next(task_queue_ref_t task_queue);

static void scheduler_schedule_init(task_schedule_ref_t task_schedule);
static scheduled_task_ref_t scheduler_schedule_get_task(
    task_schedule_ref_t task_schedule,
    task_id_t task_id);
static scheduled_task_ref_t scheduler_schedule_new_task(
    task_schedule_ref_t task_schedule);
static bool scheduler_schedule_remove_task(
    task_schedule_ref_t task_schedule,
    task_id_t task_id);
static bool scheduler_schedule_has_task(
    task_schedule_ref_t task_schedule,
    task_id_t task_id);

static void scheduler_internal_init(scheduler_ref_t scheduler);
static void scheduler_call_next(scheduler_ref_t scheduler);
static void scheduler_perform_scheduling(scheduler_ref_t scheduler);
static void scheduler_internal_loop(scheduler_ref_t scheduler);

static task_id_t scheduler_register_periodic_callback(
    scheduler_ref_t scheduler,
    uint8_t priority,
    uint32_t period_micros,
    task_t task_pointer,
    void * data);
static task_id_t scheduler_register_delayed_callback(
    scheduler_ref_t scheduler,
    uint8_t priority,
    uint32_t delay_micros,
    task_t task_pointer,
    void * data);
static task_id_t scheduler_register_immediate_callback(
    scheduler_ref_t scheduler,
    uint8_t priority,
    task_t task_pointer,
    void * data);
static task_id_t scheduler_register_on_event_callback(
    scheduler_ref_t scheduler,
    uint8_t priority,
    event_mask_t event_mask,
    event_task_t task_pointer,
    void * data);
static task_id_t scheduler_register_on_event_callback_without_mask(
    scheduler_ref_t scheduler,
    uint8_t priority,
    event_mask_t event_mask,
    task_t task_pointer,
    void * data);

static void scheduler_unregister_task(
    scheduler_ref_t scheduler,
    scheduled_task_ref_t task);
static void scheduler_unregister_task_by_id(
    scheduler_ref_t scheduler,
    task_id_t task_id);

static void scheduler_internal_trigger_event(
    scheduler_ref_t scheduler,
    event_mask_t event_mask);


/*
 *  Scheduler Queue Internal Functions
 */

/*
 * Function: scheduler_queue_init
 *  Initializes the state of the task_queue to an empty state.
 */
static void scheduler_queue_init(task_queue_ref_t task_queue)
{
    if (task_queue)
    {
        memset(task_queue, 0, sizeof(task_queue_t));
    }
}

/*
 * Function: scheduler_queue_enqueue
 *  Adds a new task to the task_queue.
 *
 * Return:
 *  If successfully added then returns true otherwise false.
 */
static bool scheduler_queue_enqueue(
    task_queue_ref_t task_queue,
    scheduled_task_ref_t new_task)
{
    uint8_t i;
    uint8_t insert_idx;

    /* Check if Queue is Full */
    if (!task_queue || task_queue->size == SCHEDULER_MAX_TASKS || !new_task)
    {
        return false;
    }

    /*
     * Check if already scheduled. Tasks cannot be queued twice.
     * If it is, consider it success.
     */
    if (new_task->flags & TASK_FLAG_IN_QUEUE)
    {
        return true;
    }

    /*
     * Find first location in the queue with a lower priority than
     * the new task.
     */
    insert_idx = task_queue->size;
    for (i = 0; i < task_queue->size; i++)
    {
        if (task_queue->queued_tasks[i]->priority > new_task->priority)
        {
            insert_idx = i;
            break;
        }
    }

    /* Push back any queued tasks after the insertion point. */
    for (i = task_queue->size; i > insert_idx; i--)
    {
        task_queue->queued_tasks[i] = task_queue->queued_tasks[i-1];
    }

    /* Insert new task. */
    task_queue->queued_tasks[insert_idx] = new_task;
    task_queue->size++;

    /* Mark task as scheduled. */
    new_task->flags |= TASK_FLAG_IN_QUEUE;

    return true;
}

/*
 * Function: scheduler_queue_remove
 *  Removes a task from the task_queue.
 *
 * Return:
 *  If successfully removed then returns true otherwise false.
 */
static bool scheduler_queue_remove(
    task_queue_ref_t task_queue,
    task_id_t task_id)
{
    uint8_t i;
    uint8_t remove_idx;
    scheduled_task_ref_t task;

    /* Check if queue has tasks and that task ID is valid. */
    if (task_queue->size == 0 || task_id < 0)
    {
        return false;
    }

    /* Find the task in the queue. */
    remove_idx = task_queue->size;
    for (i = 0; i < task_queue->size; i++)
    {
        if (task_queue->queued_tasks[i]->task_id == task_id)
        {
            remove_idx = i;
            break;
        }
    }

    /* Check that it was actually found. */
    if (remove_idx == task_queue->size)
    {
        return false;
    }

    /* Save task reference. */
    task = task_queue->queued_tasks[remove_idx];

    /* Shift all tasks through the queue. */
    task_queue->size--;
    for (i = remove_idx; i < task_queue->size; i++)
    {
        task_queue->queued_tasks[i] = task_queue->queued_tasks[i+1];
    }
    task_queue->queued_tasks[task_queue->size] = NULL;

    /* Remove the task queued flag. */
    task->flags &= (~TASK_FLAG_IN_QUEUE);

    return true;
}

/*
 * Function: scheduler_queue_next
 *  Gets the task at the front of the queue, removes it, and returns.
 *
 * Return:
 *  If queue is not empty, returns the scheduled task reference.
 *  Otherwise returns NULL.
 */
static scheduled_task_ref_t scheduler_queue_next(task_queue_ref_t task_queue)
{
    scheduled_task_ref_t next;

    if (task_queue->size == 0)
    {
        return NULL;
    }

    next = task_queue->queued_tasks[0];
    scheduler_queue_remove(task_queue, next->task_id);
    return next;
}

/*
 *  Task Schedule Internal Function
 */

/*
 * Function: scheduler_schedule_init
 *  Initializes the task schedule list to an empty list state.
 */
static void scheduler_schedule_init(task_schedule_ref_t task_schedule)
{
    uint8_t i;
    if (task_schedule)
    {
        memset(task_schedule, 0, sizeof(task_schedule_t));
        task_schedule->last_id = SCHEDULER_ID_MASK;

        /*
         * Because 0 is a valid task ID, we need to set all the
         * task ids to an invalid value.
         */
        for (i = 0; i < SCHEDULER_MAX_TASKS; i++)
        {
            task_schedule->scheduled_tasks[i].task_id = SCHEDULER_INVALID_ID;
        }
    }
}

/*
 * Function: scheduler_schedule_get_task
 *  Finds the task schedule based on the task ID
 *
 * Returns:
 *  If a task schedule reference if the task ID exists.  NULL
 *  otherwise.
 */
static scheduled_task_ref_t scheduler_schedule_get_task(
    task_schedule_ref_t task_schedule,
    task_id_t task_id)
{
    uint8_t idx;

    /* Check if task id is in valid range. */
    if (!SCHEDULER_ID_IS_VALID(task_id))
    {
        return NULL;
    }

    idx = task_id % SCHEDULER_MAX_TASKS;
    if (task_schedule->scheduled_tasks[idx].task_id == task_id)
    {
        return &task_schedule->scheduled_tasks[idx];
    }
    return NULL;
}

/*
 * Function: scheduler_schedule_new_task
 *  Finds an open spot in the task schedule and returns a reference
 *  to the slot.
 *
 * Returns:
 *  If a slot is open, then it returns the reference to the slot,
 *  otherwise returns NULL, and no slot is available.
 */
static scheduled_task_ref_t scheduler_schedule_new_task(
    task_schedule_ref_t task_schedule)
{
    uint8_t i;
    task_id_t new_id;
    scheduled_task_ref_t slot;

    /* Check if space is available. */
    if (task_schedule->size == SCHEDULER_MAX_TASKS)
    {
        return NULL;
    }

    /* Find open slot. */
    slot = NULL;
    for (i = 1; i <= SCHEDULER_MAX_TASKS; i++)
    {
        new_id = ((task_schedule->last_id + i) & SCHEDULER_ID_MASK);

        slot = &task_schedule->scheduled_tasks[new_id % SCHEDULER_ID_MASK];

        /* Check if slot is open. */
        if (slot->task_id == SCHEDULER_INVALID_ID)
        {
            break;
        }
    }

    slot->task_id = new_id;
    slot->exit_code = TASK_EXIT_NO_CODE;
    task_schedule->size++;
    task_schedule->last_id = new_id;

    return slot;
}

/*
 * Function: scheduler_schedule_remove_task
 *  Removes the specified task from the task schedule.
 *
 * Returns:
 *  If the task was removed successfully then function returns `true`,
 *  otherwise returns `false`.
 */
static bool scheduler_schedule_remove_task(
    task_schedule_ref_t task_schedule,
    task_id_t task_id)
{
    scheduled_task_ref_t slot;

    /* Check if any slot is used. */
    if (task_schedule->size == 0)
    {
        return false;
    }

    /* Get the slot of the specified task ID. */
    slot = scheduler_schedule_get_task(task_schedule, task_id);

    /* Check that task exists. */
    if (!slot)
    {
        return false;
    }

    /* Clear slot and assign it an invalid ID. */
    memset(slot, 0, sizeof(scheduled_task_t));
    slot->task_id = SCHEDULER_INVALID_ID;
    task_schedule->size--;

    return true;
}

/*
 * Function: scheduler_schedule_has_task
 *  Checks if the given task ID is in the schedule.
 *
 * Returns:
 *  `true` if the task ID is in the schedule, `false` otherwise.
 */
static bool scheduler_schedule_has_task(
    task_schedule_ref_t task_schedule,
    task_id_t task_id)
{
    uint8_t idx;
    idx = task_id % SCHEDULER_MAX_TASKS;
    return task_schedule->scheduled_tasks[idx].task_id == task_id;
}

/*
 * Scheduler Internal Functions.
 */

/*
 * Function: scheduler_internal_init
 *  Initializes the state of the scheduler to the state which
 *  tasks can be scheduled.
 */
static void scheduler_internal_init(scheduler_ref_t scheduler)
{
    if (!scheduler)
    {
        return;
    }

    memset(scheduler, 0, sizeof(scheduler_t));
    scheduler_schedule_init(&scheduler->task_schedule);
    scheduler_queue_init(&scheduler->task_queue);

    scheduler->last_scheduling = system_time();
    scheduler->next_scheduling = scheduler->last_scheduling - 1;
}

/*
 * Function: scheduler_call_next
 *  Calls the next queued function.  This will handle all the
 *  the details behind its call.
 *
 *  The called task will be removed if it is specified to and it
 *  has not requested a rescheduling.
 *
 *  A task's exit code is saved if the task schedule still exists
 *  after assessing the task schedule's fate.
 */
static void scheduler_call_next(scheduler_ref_t scheduler)
{
    scheduled_task_ref_t task;
    uint8_t exit_code;

    if (scheduler->current_task)
    {
        /* Cannot call task if there is already a task running. */
        return;
    }

    task = scheduler_queue_next(&scheduler->task_queue);

    if (!task)
    {
        /* No Task is current queued */
        return;
    }


    scheduler->current_task = task;

    /*
     * Call function based on type of task call.
     *  Event type or regular type.
     */
    if (task->flags & TASK_FLAG_EVENT_CALL)
    {
        enabled_interrupts();
        exit_code = task->event_task_function(task->trigger_event_mask, task->data);
        disable_interrupts();
    }
    else /* Regular Call */
    {
        enabled_interrupts();
        exit_code = task->task_function(task->data);
        disable_interrupts();
    }

    scheduler->current_task = NULL;

    /*
     * Determine what to do with leftover task schedule.
     *  Order of priority:
     *      1) Exit code
     *      2) Schedule remove flag
     *      3) Schedule, other flags
     *
     *  Note: Once removed from the queue, the `task` pointer becomes
     *        invalid.
     */

    /*
     * Task removal or non-removal cases.
     *  In the event of re-scheduling failing, there is not
     *  much to do in this context.
     */
    if (exit_code == TASK_EXIT_RESCHEDULE_NOW)
    {
        scheduler_queue_enqueue(&scheduler->task_queue, task);
    }
    else if (task->flags & TASK_FLAG_REMOVE
             || exit_code == TASK_EXIT_NO_RESCHEDULE)
    {
        scheduler_schedule_remove_task(
            &scheduler->task_schedule, task->task_id);
        task = NULL;
    }

    /*
     * Post-task task schedule modifications.
     */
    if (TASK_EXIT_IS_RESERVED(exit_code) && task)
    {
        switch (exit_code)
        {
            case TASK_EXIT_LOWER_PRIORITY:
                if (task->priority < TASK_PRIORITY_LOWEST)
                {
                    task->period++;
                }
                break;
            case TASK_EXIT_RAISE_PRIORITY:
                if (task->priority > TASK_PRIORITY_HIGHEST)
                {
                    task->priority--;
                }
                break;
        }
    }

    /*
     * If event based, remove the event flags which have been
     * passed.  Some events might have been triggered during
     * the task execution.  Must clear all old flags.  No
     * rescheduling has occured, so we can use the scheduler's
     * current_events mask.
     */
    if (task && task->flags & TASK_FLAG_EVENT_TRIGGER)
    {
        task->trigger_event_mask =
            (scheduler->current_events & task->event_mask);
    }

    if (exit_code == TASK_EXIT_FAILED)
    {
        scheduler_internal_trigger_event(
            scheduler,
            SCHEDULER_EVENT_TASK_FAILED);
    }


    /*
     *  If the task still exists, then it can store the return value.
     */
    if (task)
    {
        task->exit_code = exit_code;
    }

    /*
     * If the task schedule still exists, then we need to store its
     * pointer, otherwise, we can just set it to NULL.  In this case
     * task is NULL if we want to store NULL.
     */
    scheduler->last_task = task;
}

/*
 * Function: scheduler_perform_scheduling
 *  Queues all the tasks which need to be queued since the last
 *  scheduling.
 *
 *  After this, the scheduler will try to determine when
 *  the next scheduling should be conducted based on the
 *  time-based task schedules.
 *
 *  The current event flag is cleared.
 */
static void scheduler_perform_scheduling(scheduler_ref_t scheduler)
{
    time_t now, soonest_next;
    uint8_t idx;
    scheduled_task_ref_t task;

    if (scheduler->current_task)
    {
        /*
         * Cannot perform scheduling while there is a running task.
         */
        return;
    }

    now = system_time();
    soonest_next = now - 1;

    /* Loop over all task schedules in the scheduler. */
    for (idx = 0; idx < scheduler->task_schedule.size; idx++)
    {
        task = &scheduler->task_schedule.scheduled_tasks[idx];

        if (!SCHEDULER_ID_IS_VALID(task->task_id))
        {
            continue;
        }

        if (task->flags & TASK_FLAG_EVENT_TRIGGER)
        {
            /*
             * It is possible that an event-based task is trigger twice
             * while it is in queue.  This does not entitle to be queued
             * twice.  However, the trigger_event_mask should be updated.
             */
            task->trigger_event_mask |=
                (task->event_mask & scheduler->current_events);

            if (task->trigger_event_mask
                && !(task->flags & TASK_FLAG_IN_QUEUE))
            {
                scheduler_queue_enqueue(&scheduler->task_queue, task);
            }
        }
        else if (task->flags & TASK_FLAG_TIME_TRIGGER)
        {
            if (micro_time_is_in_range(
                    task->next_execution,
                    scheduler->last_scheduling,
                    now))
            {
                /*
                 * Possible that the task is still waiting from
                 * from the last enqueue.  Nothing to do, but the
                 * task loses that execution cycle.
                 */
                if (!(task->flags & TASK_FLAG_IN_QUEUE))
                {
                    scheduler_queue_enqueue(&scheduler->task_queue, task);
                }

                if (task->flags & TASK_FLAG_PERIODIC)
                {
                    task->next_execution += task->period;
                }
            }

            /* Determine when the scheduler should reschedule again. */
            if (micro_time_is_in_range(task->next_execution, now, soonest_next))
            {
                soonest_next = task->next_execution;
            }
        }
        else /* Invalid task trigger. */
        {
            scheduler_unregister_task(scheduler, task);
        }
    }

    scheduler->last_scheduling = now;
    scheduler->next_scheduling = soonest_next;
    scheduler->current_events = 0;
}

/*
 * Function: scheduler_internal_loop
 *  Checks if there are tasks to be executed and executes them.
 *  In between calls, the scheduler will check if needs to schedule
 *  most tasks.  Once there are no more tasks that currently need
 *  to be scheduled, and no more tasks that need to be executed,
 *  this function returns.
 */
static void scheduler_internal_loop(scheduler_ref_t scheduler)
{
    time_t now;

    if (!scheduler)
    {
        return;
    }

    while(scheduler->task_queue.size > 0)
    {
        now = system_time();
        if (scheduler->current_events
            || micro_time_is_in_range(
                scheduler->next_scheduling,
                scheduler->last_scheduling,
                now))
        {
            scheduler_perform_scheduling(scheduler);
        }

        scheduler_call_next(scheduler);
    }

    now = system_time();
    if (scheduler->current_events
        || micro_time_is_in_range(
            scheduler->next_scheduling,
            scheduler->last_scheduling,
            now))
    {
        scheduler_perform_scheduling(scheduler);
    }
}

/*
 *  Scheduler Internal Scheduling Functions
 */

/*
 * Function: scheduler_register_periodic_callback
 *  Used internally to the module to schedule a new periodic
 *  task.  Can assume the input arguments have been validated.
 */
static task_id_t scheduler_register_periodic_callback(
    scheduler_ref_t scheduler,
    uint8_t priority,
    uint32_t period_micros,
    task_t task_pointer,
    void * data)
{
    task_id_t new_id;
    scheduled_task_ref_t new_task;

    new_task = scheduler_schedule_new_task(&scheduler->task_schedule);

    if (new_task != NULL)
    {
        new_id = new_task->task_id;
        new_task->task_function = task_pointer;
        new_task->data = data;

        new_task->priority = priority;
        new_task->exit_code = TASK_EXIT_NO_CODE;
        new_task->next_execution = system_time() + period_micros;
        new_task->period = period_micros;
        new_task->flags = (TASK_FLAG_TIME_TRIGGER | TASK_FLAG_PERIODIC);

        if (micro_time_is_in_range(
                new_task->next_execution,
                scheduler->last_scheduling, scheduler->next_scheduling))
        {
            scheduler->next_scheduling = new_task->next_execution;
        }
    }
    else
    {
        new_id = SCHEDULER_INVALID_ID;
    }

    return new_id;
}

/*
 * Function: scheduler_register_delayed_callback
 *  Used internally to the module to schedule a new delayed
 *  task.  Can assume the input arguments have been validated.
 */
static task_id_t scheduler_register_delayed_callback(
    scheduler_ref_t scheduler,
    uint8_t priority,
    uint32_t delay_micros,
    task_t task_pointer,
    void * data)
{
    task_id_t new_id;
    scheduled_task_ref_t new_task;

    new_task = scheduler_schedule_new_task(&scheduler->task_schedule);

    if (new_task != NULL)
    {
        new_id = new_task->task_id;
        new_task->task_function = task_pointer;
        new_task->data = data;

        new_task->priority = priority;
        new_task->exit_code = TASK_EXIT_NO_CODE;
        new_task->next_execution = system_time() + delay_micros;
        new_task->flags = (TASK_FLAG_TIME_TRIGGER | TASK_FLAG_REMOVE);

        if (micro_time_is_in_range(
                new_task->next_execution,
                scheduler->last_scheduling, scheduler->next_scheduling))
        {
            scheduler->next_scheduling = new_task->next_execution;
        }
    }
    else
    {
        new_id = SCHEDULER_INVALID_ID;
    }

    return new_id;
}


/*
 * Function: scheduler_register_immediate_callback
 *  Used internally to the module to schedule a new immediate
 *  task.  Can assume the input arguments have been validated.
 */
static task_id_t scheduler_register_immediate_callback(
    scheduler_ref_t scheduler,
    uint8_t priority,
    task_t task_pointer,
    void * data)
{
    task_id_t new_id;
    scheduled_task_ref_t new_task;

    new_task = scheduler_schedule_new_task(&scheduler->task_schedule);

    if (new_task != NULL)
    {
        new_id = new_task->task_id;

        new_task->task_function = task_pointer;
        new_task->data = data;

        new_task->priority = priority;
        new_task->exit_code = TASK_EXIT_NO_CODE;
        new_task->next_execution = system_time();
        new_task->flags = (TASK_FLAG_TIME_TRIGGER | TASK_FLAG_REMOVE);

        scheduler->next_scheduling = new_task->next_execution;
    }
    else
    {
        new_id = SCHEDULER_INVALID_ID;
    }

    return new_id;
}

/*
 * Function: scheduler_register_on_event_callback
 *  Used internally to the module to schedule a new on-event
 *  task.  Can assume the input arguments have been validated.
 */
static task_id_t scheduler_register_on_event_callback(
    scheduler_ref_t scheduler,
    uint8_t priority,
    event_mask_t event_mask,
    event_task_t task_pointer,
    void * data)
{
    task_id_t new_id;
    scheduled_task_ref_t new_task;

    new_task = scheduler_schedule_new_task(&scheduler->task_schedule);

    if (new_task != NULL)
    {
        new_id = new_task->task_id;
        new_task->event_task_function = task_pointer;
        new_task->data = data;

        new_task->priority = priority;
        new_task->exit_code = TASK_EXIT_NO_CODE;
        new_task->event_mask = event_mask;
        new_task->flags = (TASK_FLAG_EVENT_TRIGGER | TASK_FLAG_EVENT_CALL);
    }
    else
    {
        new_id = SCHEDULER_INVALID_ID;
    }

    return new_id;
}

/*
 * Function: scheduler_register_on_event_callback_without_mask
 *  Used internally to the module to schedule a new on-event
 *  task without event mask function prototypes.  Can assume
 *  the input arguments have been validated.
 */
static task_id_t scheduler_register_on_event_callback_without_mask(
    scheduler_ref_t scheduler,
    uint8_t priority,
    event_mask_t event_mask,
    task_t task_pointer,
    void * data)
{
    task_id_t new_id;
    scheduled_task_ref_t new_task;

    new_task = scheduler_schedule_new_task(&scheduler->task_schedule);

    if (new_task != NULL)
    {
        new_id = new_task->task_id;
        new_task->task_function = task_pointer;
        new_task->data = data;

        new_task->priority = priority;
        new_task->exit_code = TASK_EXIT_NO_CODE;
        new_task->event_mask = event_mask;
        new_task->flags = TASK_FLAG_EVENT_TRIGGER;
    }
    else
    {
        new_id = SCHEDULER_INVALID_ID;
    }

    return new_id;
}

/*
 * Function: scheduler_unregister_task
 *  Used internally to the module to remove a task. The input
 *  arguments have been validated.
 */
static void scheduler_unregister_task(
    scheduler_ref_t scheduler,
    scheduled_task_ref_t task)
{
    /*
     * Check if the task to be removed is the current task.
     *  If the current task is the one to be removed, then let it
     *  finish running and have the scheduler remove it after.
     */
    if (scheduler->current_task
        && scheduler->current_task->task_id == task->task_id)
    {
        scheduler->current_task->flags |= TASK_FLAG_REMOVE;
    }
    else
    {
        /* Remove from queue first as it references the scheduler's list. */
        if (task->flags & TASK_FLAG_IN_QUEUE)
        {
            scheduler_queue_remove(&scheduler->task_queue, task->task_id);
        }

        /*
         * If the scheduler's last task is removed, then we
         * need remove it as last.
         */
        if (scheduler->last_task->task_id == task->task_id)
        {
            scheduler->last_task = NULL;
        }

        scheduler_schedule_remove_task(
            &scheduler->task_schedule,
            task->task_id);
    }
}

/*
 * Function: scheduler_unregister_task_by_id
 *  Used internally to the module to remove a task by a given
 *  task id.  If the task ID is invalid, nothing happens. The input
 *  arguments have been validated.
 */
static void scheduler_unregister_task_by_id(
    scheduler_ref_t scheduler,
    task_id_t task_id)
{
    scheduled_task_ref_t task;

    task = scheduler_schedule_get_task(&scheduler->task_schedule, task_id);

    if (task)
    {
        scheduler_unregister_task(scheduler, task);
    }
}

/*
 * Function: scheduler_internal_trigger_event
 *  Used internally to register an event with the scheduler.
 */
static void scheduler_internal_trigger_event(
    scheduler_ref_t scheduler,
    event_mask_t event_mask)
{
    if (!event_mask) return;
    scheduler->current_events |= event_mask;
}

/*
 *  Scheduler Internal Task Utilities
 */

/*
 * Function: scheduler_util_current_task_id
 *  Used internally to get the task id of the current task.  Returns
 *  an SCHEDULER_INVALID_ID if no task is currently executing.
 */
static task_id_t scheduler_util_current_task_id(scheduler_ref_t scheduler)
{
    if (!scheduler || !scheduler->current_task)
    {
        return SCHEDULER_INVALID_ID;
    }
    return scheduler->current_task->task_id;
}

/*
 * Function: scheduler_util_last_task_id
 *  Used internally to get the task id of the last task to complete
 *  execution.  Returns SCHEDULER_INVALID_ID if the previous task
 *  is unknown.
 */
static task_id_t scheduler_util_last_task_id(scheduler_ref_t scheduler)
{
    if (!scheduler || !scheduler->last_task)
    {
        return SCHEDULER_INVALID_ID;
    }
    return scheduler->last_task->task_id;
}

/*
 * Function: scheduler_util_exit_code_of
 *  Gets the exit code of the last exicution of the task.  If the
 *  task does not exist, or there is no exit code associated to it,
 *  then TASK_EXIT_NO_CODE is returned.
 */
static uint8_t scheduler_util_exit_code_of(
    scheduler_ref_t scheduler,
    task_id_t task_id)
{
    scheduled_task_ref_t task;
    if (!scheduler
        || !SCHEDULER_ID_IS_VALID(task_id)
        || !scheduler_schedule_has_task(&scheduler->task_schedule, task_id))
    {
        return TASK_EXIT_NO_CODE;
    }
    task = scheduler_schedule_get_task(&scheduler->task_schedule, task_id);
    return task->exit_code;
}

/*
 *  Scheduler Internal Variables
 */

static scheduler_t scheduler;

/*
 *  Scheduler Public Scheduling Functions
 */

task_id_t scheduler_periodic_callback(
    uint8_t priority,
    uint32_t period_micros,
    task_t task_pointer,
    void * data)
{
    task_id_t new_id;

    /*
     *  Input Validation
     */
    if (!task_pointer)
    {
        return SCHEDULER_INVALID_ID;
    }

    if (period_micros < SCHEDULER_MINIMUM_PERIOD)
    {
        period_micros = SCHEDULER_MINIMUM_PERIOD;
    }

    disable_interrupts();

    new_id = scheduler_register_periodic_callback(
        &scheduler,
        priority,
        period_micros,
        task_pointer,
        data);

    enabled_interrupts();
    return new_id;
}

task_id_t scheduler_delayed_callback(
    uint8_t priority,
    uint32_t delay_micros,
    task_t task_pointer,
    void * data)
{
    task_id_t new_id;

    if (!task_pointer)
    {
        return SCHEDULER_INVALID_ID;
    }

    if (delay_micros == 0)
    {
        return scheduler_immediate_callback(priority, task_pointer, data);
    }

    disable_interrupts();

    new_id = scheduler_register_delayed_callback(
        &scheduler,
        priority,
        delay_micros,
        task_pointer,
        data);

    enabled_interrupts();
    return new_id;
}

task_id_t scheduler_immediate_callback(
    uint8_t priority,
    task_t task_pointer,
    void * data)
{
    task_id_t new_id;

    if (!task_pointer)
    {
        return SCHEDULER_INVALID_ID;
    }

    disable_interrupts();

    new_id = scheduler_register_immediate_callback(
        &scheduler,
        priority,
        task_pointer,
        data);

    enabled_interrupts();
    return new_id;
}

task_id_t scheduler_on_event_callback(
    uint8_t priority,
    event_mask_t event_mask,
    event_task_t task_pointer,
    void * data)
{
    task_id_t new_id;

    if (!task_pointer)
    {
        return SCHEDULER_INVALID_ID;
    }

    if (!event_mask)
    {
        return SCHEDULER_INVALID_ID;
    }

    disable_interrupts();

    new_id = scheduler_register_on_event_callback(
        &scheduler,
        priority,
        event_mask,
        task_pointer,
        data);

    enabled_interrupts();
    return new_id;
}

task_id_t scheduler_on_event_callback_without_mask(
    uint8_t priority,
    event_mask_t event_mask,
    task_t task_pointer,
    void * data)
{
    task_id_t new_id;

    if (!task_pointer)
    {
        return SCHEDULER_INVALID_ID;
    }

    if (!event_mask)
    {
        return SCHEDULER_INVALID_ID;
    }

    disable_interrupts();

    new_id = scheduler_register_on_event_callback_without_mask(
        &scheduler,
        priority,
        event_mask,
        task_pointer,
        data);

    enabled_interrupts();
    return new_id;
}

void scheduler_remove(task_id_t id)
{
    if (id < 0)
    {
        return;
    }
    disable_interrupts();

    scheduler_unregister_task_by_id(
        &scheduler,
        id);

    enabled_interrupts();
}

void scheduler_trigger_event(event_mask_t event_mask)
{
    if (!event_mask)
    {
        return;
    }
    disable_interrupts();
    scheduler_internal_trigger_event(&scheduler, event_mask);
    enabled_interrupts();
}

/*
 *  Scheduler Public Task Utilities
 */

task_id_t scheduler_current_task_id(void)
{
    task_id_t task_id;
    disable_interrupts();
    task_id = scheduler_util_current_task_id(&scheduler);
    enabled_interrupts();
    return task_id;
}

task_id_t scheduler_last_task_id(void)
{
    task_id_t task_id;
    disable_interrupts();
    task_id = scheduler_util_last_task_id(&scheduler);
    enabled_interrupts();
    return task_id;
}

uint8_t scheduler_exit_code_of(task_id_t task_id)
{
    uint8_t exit_code;
    disable_interrupts();
    exit_code = scheduler_util_exit_code_of(&scheduler, task_id);
    enabled_interrupts();
    return exit_code;
}

/*
 *  Scheduler Module Control Functions
 */

void scheduler_init(void)
{
    disable_interrupts();
    scheduler_internal_init(&scheduler);
    enabled_interrupts();
}

void scheduler_loop(void)
{
    disable_interrupts();
    scheduler_internal_loop(&scheduler);
    enabled_interrupts();
}
