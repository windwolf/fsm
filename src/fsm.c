#include "fsm/fsm.h"

static FSM_State_t *FSM_find_state(FSM_t *fsm, uint32_t state_no);

static void FSM_state_entry(FSM_t *fsm, FSM_State_t *state, uint32_t tick);

static void FSM_state_exit(FSM_t *fsm, FSM_State_t *state);

static void FSM_state_poll(FSM_t *fsm, FSM_State_t *state);

static void FSM_try_transition_by_timeout(FSM_t *fsm, FSM_State_t *state, uint32_t tick);
static void FSM_try_transition_by_event(FSM_t *fsm, FSM_State_t *state, uint32_t event, uint32_t tick);

void FSM_init(FSM_t *fsm)
{
    fsm->state_count = 0;
    fsm->current_state = NULL;
    fsm->current_state_enter_tick = 0;
    fsm->user_data = NULL;
}

void FSM_state_register(FSM_t *fsm, FSM_State_Config_t *config)
{
    if (fsm->state_count >= FSM_MAX_STATES_COUNT)
    {
        return;
    }
    fsm->states[fsm->state_count].config = config;
    fsm->states[fsm->state_count].user_data = NULL;
    fsm->states[fsm->state_count].transition_count = 0;
    fsm->state_count++;
}

void FSM_transition_register(FSM_t *fsm, FSM_Transition_Config_t *config, uint32_t from, uint32_t to)
{
    FSM_State_t *fromState = FSM_find_state(fsm, from);
    FSM_State_t *toState = FSM_find_state(fsm, to);
    if (fromState == NULL || toState == NULL)
    {
        return;
    }

    fromState->transitions[fromState->transition_count].config = config;
    fromState->transitions[fromState->transition_count].to = toState;
    fromState->transition_count++;
}

void FSM_start(FSM_t *fsm, uint32_t state_no, void *user_data, uint32_t initial_tick)
{
    FSM_State_t *state = FSM_find_state(fsm, state_no);
    if (state == NULL)
    {
        return;
    }
    fsm->user_data = user_data;
    fsm->last_update_tick = initial_tick;
    FSM_state_entry(fsm, state, initial_tick);
}

void FSM_update_with_event(FSM_t *fsm, uint32_t event, uint32_t tick)
{
    FSM_State_t *state = fsm->current_state;
    if (state == NULL)
    {
        return;
    }
    FSM_state_poll(fsm, state);
    FSM_try_transition_by_event(fsm, state, event, tick);
    FSM_try_transition_by_timeout(fsm, state, tick);
    fsm->last_update_tick = tick;
}

void FSM_update(FSM_t *fsm, uint32_t tick)
{
    FSM_State_t *state = fsm->current_state;
    if (state == NULL)
    {
        return;
    }
    FSM_state_poll(fsm, state);
    FSM_try_transition_by_timeout(fsm, state, tick);
    fsm->last_update_tick = tick;
}

void FSM_update_inc(FSM_t *fsm, uint32_t tick_inc)
{
    FSM_upate(fsm, fsm->last_update_tick + tick_inc);
}

void FSM_update_with_event_inc(FSM_t *fsm, uint32_t event, uint32_t tick_inc)
{
    FSM_update_with_event(fsm, event, fsm->last_update_tick + tick_inc);
}

static FSM_State_t *FSM_find_state(FSM_t *fsm, uint32_t state_no)
{
    for (uint32_t i = 0; i < fsm->state_count; i++)
    {
        if (fsm->states[i].config->state_no == state_no)
        {
            return &(fsm->states[i]);
        }
    }

    return NULL;
}

static void FSM_state_entry(FSM_t *fsm, FSM_State_t *state, uint32_t tick)
{
    fsm->current_state = state;
    fsm->current_state_enter_tick = tick;
    if (state->config->entry_action != NULL)
    {
        state->config->entry_action(fsm, state);
    }
}

static void FSM_state_exit(FSM_t *fsm, FSM_State_t *state)
{
    if (state->config->exit_action != NULL)
    {
        state->config->exit_action(fsm, state);
    }
}

static void FSM_state_poll(FSM_t *fsm, FSM_State_t *state)
{
    if (state->config->poll_action != NULL)
    {
        state->config->poll_action(fsm, state);
    }
}

static void FSM_try_transition_by_timeout(FSM_t *fsm, FSM_State_t *state, uint32_t tick)
{
    for (uint32_t i = 0; i < state->transition_count; i++)
    {
        FSM_Transition_t *transition = &state->transitions[i];
        if (transition->config->mode == FSM_TRANSITION_MODE_TIMEOUT && transition->config->mode_parameters.timeout <= (tick - fsm->current_state_enter_tick))
        {
            FSM_state_exit(fsm, state);
            if (transition->config->action != NULL)
            {
                transition->config->action(fsm, state);
            }
            FSM_state_entry(fsm, transition->to, tick);
            return;
        }
    }
}

static void FSM_try_transition_by_event(FSM_t *fsm, FSM_State_t *state, uint32_t event, uint32_t tick)
{
    for (uint32_t i = 0; i < state->transition_count; i++)
    {
        FSM_Transition_t *transition = &state->transitions[i];
        if (transition->config->mode == FSM_TRANSITION_MODE_EVENT && transition->config->mode_parameters.event_no == event)
        {
            FSM_state_exit(fsm, state);
            if (transition->config->action != NULL)
            {
                transition->config->action(fsm, state);
            }
            FSM_state_entry(fsm, transition->to, tick);
            fsm->last_update_tick = tick;
            return;
        }
    }
}