#ifndef ___FSM_H__
#define ___FSM_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define FSM_MAX_STATES_COUNT (8)
#define FSM_MAX_TRANSITIONS_COUNT (4)

    typedef enum
    {
        FMS_STATE_MODE_INTERVAL,
        FMS_STATE_MODE_POLL,
    } FMS_State_Mode_t;

    typedef struct FSM_State_Config_t
    {
        uint32_t state_no;
        FMS_State_Mode_t mode;
        union
        {
            uint32_t interval;
        } mode_parameters;
        void (*entry_action)(struct FSM_t *, struct FSM_State_t *);
        void (*poll_action)(struct FSM_t *, struct FSM_State_t *);
        void (*exit_action)(struct FSM_t *, struct FSM_State_t *);
    } FSM_State_Config_t;

    typedef struct FSM_State_t
    {
        FSM_State_Config_t *config;
        void *user_data;
        FSM_Transition_t transitions[FSM_MAX_TRANSITIONS_COUNT];
        uint32_t transition_count;
    } FSM_State_t;

    typedef enum
    {
        FSM_TRANSITION_MODE_TIMEOUT,
        FSM_TRANSITION_MODE_EVENT,
        // FSM_TRANSITION_MODE_EVENT_OR_TIMEOUT,
    } FSM_Transition_Mode;

    typedef struct FSM_Transition_Config_t
    {
        FSM_Transition_Mode mode;
        union
        {
            uint32_t event_no;
            uint32_t timeout;
        } mode_parameters;
        uint32_t (*action)(struct FSM_t *, struct FSM_State_t *);
    } FSM_Transition_Config_t;

    typedef struct FSM_Transition_t
    {
        FSM_Transition_Config_t *config;
        FSM_State_t *to;
    } FSM_Transition_t;

    typedef struct FSM_t
    {
        FSM_State_t states[FSM_MAX_STATES_COUNT];
        uint32_t state_count;
        FSM_State_t *current_state;
        uint32_t current_state_enter_tick;
        uint32_t last_update_tick;
        void *user_data;
    } FSM_t;

    void FSM_state_register(FSM_t *fsm, FSM_State_Config_t *config);

    void FSM_transition_register(FSM_t *fsm, FSM_Transition_Config_t *config, uint32_t from, uint32_t to);

    void FSM_start(FSM_t *fsm, uint32_t state_no, void *user_data, uint32_t initial_tick);

    void FSM_update(FSM_t *fsm, uint32_t tick);

    void FSM_update_with_event(FSM_t *fsm, uint32_t event, uint32_t tick);

    void FSM_update_inc(FSM_t *fsm, uint32_t tick_inc);

    void FSM_update_with_event_inc(FSM_t *fsm, uint32_t event, uint32_t tick_inc);

#ifdef __cplusplus
}
#endif

#endif // ___FSM_H__