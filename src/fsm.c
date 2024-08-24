// Copyright (c) 2024 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
* @file     fsm.h
* @brief    Finite State Machine (FSM)
*@author    Ziga Miklosic
*@email     ziga.miklosic@gmail.com
*@date      27.09.2023
*@author    Matej Otic
*@email     otic.matej@dancing-bits.com
*@version   V1.2.1
*
*@section Description
*
*     This module contains general finite state machine code for general
*     purpose usage. Each FSM is created as individual, separate instance
*     creating high level of flexibility.
*
*@section Code_example
*@code
*
*
*    // ------------------------------------
*    //         VARIABLES
*    // ------------------------------------
*
*    //     APP FSM State Configurations
*    const static fsm_cfg_t g_fsm_cfg_table =
*    {
*        //         State functions
*        //
*        //     NOTE: Sequence matters!
*        .func = {      NULL,                        // eAPP_FSM_POR - No need for function
*                       app_fsm_pot_mode_hndl,        // eAPP_FSM_POT
*                       app_fsm_ssi_mode_hndl,        // eAPP_FSM_SSI
*                       app_fsm_hall_mode_hndl,        // eAPP_FSM_HALL
*                },
*        .name = "App FSM",
*        .num_of = eAPP_FSM_NUM_OF,
*    };
*
*    //     App FSM instance
*    static p_fsm_t g_app_fsm = NULL;
*
*
*   // ------------------------------------
*    //         PROGRAM
*    // ------------------------------------
*
*    // 1. Init
*    fsm_init( &g_app_fsm, &g_fsm_cfg_table )
*
*    // 2. Handle fsm
*    @x_ms
*    {
*        fsm_hndl( g_app_fsm );
*    }
*
*    // 3. Fill up actions inside FSM states
*    static void app_fsm_ssi_mode_hndl(void)
*    {
*        // First entry
*        if ( true == fsm_get_first_entry( g_app_fsm ))
*        {
*            // First entry actions here...
*        }
*
*    }
*
*@endcode
*
*/
////////////////////////////////////////////////////////////////////////////////
/**
 * @addtogroup FSM
 * @{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "fsm.h"
#include "../../fsm_cfg.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 *     FSM States
 */
typedef struct
{
    bool is_init;   /**<Is current state initial state? */
    uint8_t cur;    /**<Current state */
    uint8_t next;   /**<Next/Requested state */
} fsm_state_t;

/**
 *     FSM data
 */
typedef struct fsm_s
{
    fsm_cfg_t *     p_cfg;          /**<FSM setup */
    uint32_t        duration;       /**<Time duration in ms */
    uint32_t        tick_prev;      /**<Previous tick in ms, for duration calculations*/
    fsm_state_t     state;          /**<Current state of FSM */
    fsm_data_t      data;           /**<Data shared across states */
    bool            first_entry;    /**<First entry of state */
    bool            is_init;        /**<Initialization guard */
} fsm_t;

/**
 *     Limit loop counts
 */
#define FSM_LIMIT_DURATION(cnt)    (( cnt >= 0x1FFFFFFFUL ) ? ( 0x1FFFFFFFUL ) : ( cnt ))

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
static void fsm_exit_cur_state  (const p_fsm_t fsm_inst);
static void fsm_enter_next_state(const p_fsm_t fsm_inst);
static void fsm_handle_cur_state(const p_fsm_t fsm_inst);
static void fsm_manager         (const p_fsm_t fsm_inst);
static void fsm_reset_state     (const p_fsm_t fsm_inst);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*       Exit current FSM state by calling its exit function
*
* @param[in]    fsm_inst    - FSM instance
* @return       void
*/
////////////////////////////////////////////////////////////////////////////////
static void fsm_exit_cur_state(const p_fsm_t fsm_inst)
{
    if ( NULL != fsm_inst->p_cfg->p_states[ fsm_inst->state.cur ].on_exit )
    {
        fsm_inst->p_cfg->p_states[ fsm_inst->state.cur ].on_exit(fsm_inst);
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Enter next FSM state by calling its entry function
*
*       This function resets state duration before entry is executed.
*
* @param[in]    fsm_inst    - FSM instance
* @return       void
*/
////////////////////////////////////////////////////////////////////////////////
static void fsm_enter_next_state(const p_fsm_t fsm_inst)
{
    fsm_inst->tick_prev = FSM_GET_SYSTICK();
    fsm_inst->duration = 0.0f; // Make sure when state entry is executed duration is 0
    if ( NULL != fsm_inst->p_cfg->p_states[ fsm_inst->state.next ].on_entry )
    {
        fsm_inst->p_cfg->p_states[ fsm_inst->state.next ].on_entry(fsm_inst);
    }
    fsm_inst->state.cur = fsm_inst->state.next;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Handle current FSM state by calling its activity function
*
*       This function increments state duration and saturates it before activity
*       is executed.
*
* @param[in]    fsm_inst    - FSM instance
* @return       void
*/
////////////////////////////////////////////////////////////////////////////////
static void fsm_handle_cur_state(const p_fsm_t fsm_inst)
{
    // Accumulate time
    const uint32_t tick_now = FSM_GET_SYSTICK();
    fsm_inst->duration += (uint32_t) ( tick_now - fsm_inst->tick_prev );
    fsm_inst->duration = FSM_LIMIT_DURATION( fsm_inst->duration );
    fsm_inst->tick_prev = tick_now;

    // Execute current state
    if ( NULL != fsm_inst->p_cfg->p_states[ fsm_inst->state.cur ].on_activity )
    {
        fsm_inst->p_cfg->p_states[ fsm_inst->state.cur ].on_activity(fsm_inst);
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       FSM manager
*
* @param[in]    fsm_inst    - FSM instance
* @return       void
*/
////////////////////////////////////////////////////////////////////////////////
static void fsm_manager(const p_fsm_t fsm_inst)
{
    if (fsm_inst->state.is_init)
    {
        #if ( FSM_CFG_DEBUG_EN )
            if (NULL != fsm_inst->p_cfg->name)
            {
                if  ( NULL != fsm_inst->p_cfg->p_states[fsm_inst->state.next].name )
                {
                    FSM_DBG_PRINT( "%s transition: 'initial' -> %s", fsm_inst->p_cfg->name, fsm_inst->p_cfg->p_states[fsm_inst->state.next].name );
                }
                else
                {
                    FSM_DBG_PRINT( "%s transition: initial -> %d", fsm_inst->p_cfg->name, fsm_inst->state.next );
                }
            }
        #endif

        fsm_inst->state.is_init = false;
        // Execute entry of next state only; initial state does not have an exit activity
        fsm_enter_next_state(fsm_inst);
    }
    else if ( fsm_inst->state.cur != fsm_inst->state.next )
    {
        #if ( FSM_CFG_DEBUG_EN )
            if (NULL != fsm_inst->p_cfg->name)
            {
                if  (   ( NULL != fsm_inst->p_cfg->p_states[fsm_inst->state.cur].name )
                    &&  ( NULL != fsm_inst->p_cfg->p_states[fsm_inst->state.next].name ))
                {
                    FSM_DBG_PRINT( "%s transition: %s -> %s", fsm_inst->p_cfg->name, fsm_inst->p_cfg->p_states[fsm_inst->state.cur].name, fsm_inst->p_cfg->p_states[fsm_inst->state.next].name );
                }
                else
                {
                    FSM_DBG_PRINT( "%s transition: %d -> %d", fsm_inst->p_cfg->name, fsm_inst->state.cur, fsm_inst->state.next );
                }
            }
        #endif

        fsm_exit_cur_state(fsm_inst);        
        fsm_enter_next_state(fsm_inst);
        fsm_inst->first_entry = true;
    }

    // Same state
    else
    {
        fsm_inst->first_entry = false;
    }

    fsm_handle_cur_state(fsm_inst);
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Reset FSM states, and timing, back to initial (default) values
*
* @param[in]    fsm_inst    - FSM instance
* @return       void
*/
////////////////////////////////////////////////////////////////////////////////
static void fsm_reset_state(const p_fsm_t fsm_inst)
{
    fsm_inst->state.cur     = 0U;
    fsm_inst->state.next    = fsm_inst->state.cur;
    fsm_inst->state.is_init = true;
    fsm_inst->duration      = 0U;
    fsm_inst->tick_prev     = 0U;
    fsm_inst->is_init       = true;
    fsm_inst->first_entry   = false;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup FSM_API
* @{ <!-- BEGIN GROUP -->
*
*     Following functions are part of API calls.
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialise FSMs
*
* @param[out]    p_fsm_inst - Pointer to FSM instance
* @param[in]    p_cfg       - Pointer to FSM configuration table
* @return       status      - Status of initialisation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_init(p_fsm_t * p_fsm_inst, const fsm_cfg_t * const p_cfg)
{
    fsm_status_t status = eFSM_OK;

    FSM_ASSERT( NULL != p_fsm_inst );
    FSM_ASSERT( NULL != p_cfg );

    if     (    ( NULL != p_fsm_inst )
        &&    ( NULL != p_cfg ))
    {
        // Allocate space
        *p_fsm_inst = malloc( sizeof( fsm_t ));

        FSM_ASSERT( NULL != *p_fsm_inst );
        FSM_ASSERT( p_cfg->num_of > 0  );

        // Check if allocation succeed
        if  (   ( NULL != *p_fsm_inst )
            &&  ( p_cfg->num_of > 0 ))
        {
            // Get setup
            (*p_fsm_inst)->p_cfg = (fsm_cfg_t*) p_cfg;

            fsm_reset_state(*p_fsm_inst);
        }
        else
        {
            status = eFSM_ERROR_INIT;
        }
    }
    else
    {
        status = eFSM_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Get FSM initialisation flag
*
* @param[in]    fsm_inst    - FSM instance
* @param[out]    p_is_init  - Initialisation flag
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_is_init(const p_fsm_t fsm_inst, bool * const p_is_init)
{
    fsm_status_t status = eFSM_OK;

    if ( NULL != fsm_inst )
    {
        *p_is_init = fsm_inst->is_init;
    }
    else
    {
        status = eFSM_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Reset FSM back to its initial state without triggering any state behaviors
*
* @param[in]    fsm_inst    - FSM instance
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_reset(const p_fsm_t fsm_inst)
{
    fsm_status_t status = eFSM_OK;

    if ( NULL != fsm_inst )
    {
        fsm_reset_state(fsm_inst);
    }
    else
    {
        status = eFSM_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       FSM handler
*
* @note     Each instance of FSM must call its own handler!
*
* @param[in]    fsm_inst    - FSM instance
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_hndl(const p_fsm_t fsm_inst)
{
    fsm_status_t status = eFSM_OK;

    if ( NULL != fsm_inst )
    {
        if ( true == fsm_inst->is_init )
        {
            fsm_manager( fsm_inst );
        }
        else
        {
            status = eFSM_ERROR_INIT;
        }
    }
    else
    {
        status = eFSM_ERROR;
    }


    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Set next FSM state
*
* @param[in]    fsm_inst    - FSM instance
* @param[in]    state       - Next state
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_goto_state(const p_fsm_t fsm_inst, const uint8_t state)
{
    fsm_status_t status = eFSM_OK;

    FSM_ASSERT( NULL != fsm_inst );
    FSM_ASSERT( state < fsm_inst->p_cfg->num_of );

    if  (   ( NULL != fsm_inst )
        &&  ( state < fsm_inst->p_cfg->num_of ))
    {
        fsm_inst->state.next = state;
    }
    else
    {
        status = eFSM_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get current FSM state
*
* @param[in]    fsm_inst    - FSM instance
* @return       state       - Current state of FSM
*/
////////////////////////////////////////////////////////////////////////////////
uint8_t fsm_get_state(const p_fsm_t fsm_inst)
{
    uint8_t state = 0U;

    FSM_ASSERT( NULL != fsm_inst );

    if ( NULL != fsm_inst )
    {
        state = fsm_inst->state.cur;
    }

    return state;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get FSM state time duration in miliseconds
*
* @param[in]    fsm_inst    - FSM instance
* @return       duration    - Duration inside state in ms
*/
////////////////////////////////////////////////////////////////////////////////
uint32_t fsm_get_duration(const p_fsm_t fsm_inst)
{
    uint32_t duration = 0;

    FSM_ASSERT( NULL != fsm_inst );

    if ( NULL != fsm_inst )
    {
        duration = fsm_inst->duration;
    }

    return duration;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Reset FSM state time duration
*
* @param[in]    fsm_inst    - FSM instance
* @return       void
*/
////////////////////////////////////////////////////////////////////////////////
void fsm_reset_duration(const p_fsm_t fsm_inst)
{
    FSM_ASSERT( NULL != fsm_inst );

    if ( NULL != fsm_inst )
    {
        fsm_inst->duration = 0;
        fsm_inst->tick_prev = FSM_GET_SYSTICK();
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get FSM shared data
*
* @param[in]    fsm_inst    - FSM instance
* @return       data        - Data
*/
////////////////////////////////////////////////////////////////////////////////
fsm_data_t fsm_get_data(const p_fsm_t fsm_inst)
{
    fsm_data_t data = {0};

    FSM_ASSERT( NULL != fsm_inst );

    if ( NULL != fsm_inst )
    {
        data = fsm_inst->data;
    }

    return data;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Set FSM shared data
*
* @param[in]    fsm_inst    - FSM instance
* @param[in]    data        - Data
* @return       void
*/
////////////////////////////////////////////////////////////////////////////////
void fsm_set_data(const p_fsm_t fsm_inst, const fsm_data_t data)
{
    if ( NULL != fsm_inst )
    {
        fsm_inst->data = data;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get first state entry flag
*
* @param[in]    fsm_inst        - FSM instance
* @return       first_entry     - First entry into state
*/
////////////////////////////////////////////////////////////////////////////////
bool fsm_get_first_entry(const p_fsm_t fsm_inst)
{
    bool first_entry = false;

    if ( NULL != fsm_inst )
    {
        first_entry = fsm_inst->first_entry;
    }

    return first_entry;
}

////////////////////////////////////////////////////////////////////////////////
/*!
 * @} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////
