// Copyright (c) 2025 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
* @file     fsm.h
* @brief    Finite State Machine (FSM)
*@author    Ziga Miklosic
*@email     ziga.miklosic@gmail.com
*@author    Matej Otic
*@email     otic.matej@dancing-bits.com
*@date      24.04.2025
*@version   V2.1.0
*/
////////////////////////////////////////////////////////////////////////////////
/**
* @addtogroup FSM_API
* @{ <!-- BEGIN GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

#ifndef __FSM_H_
#define __FSM_H_

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../fsm_cfg.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 *     Module version
 */
#define FSM_VER_MAJOR       ( 2 )
#define FSM_VER_MINOR       ( 1 )
#define FSM_VER_DEVELOP     ( 0 )

/**
 *     FSM status
 */
typedef enum
{
    eFSM_OK         = 0x00U,        /**<Normal operation */
    eFSM_ERROR      = 0x01U,        /**<General error */
    eFSM_ERROR_INIT = 0x02U,        /**<Initialization error */
} fsm_status_t;

/**
 *    Generic data type that is shared across FSM states
 */
typedef union fsm_data
{
    void *p;        /**<Generic pointer */
    uint32_t u32;   /**<Single 32-bit unsigned integer */
    int32_t i32;    /**<Single 32-bit signed integer */
    struct
    {
        uint8_t b1; /**<1st byte */
        uint8_t b2; /**<2nd byte */
        uint8_t b3; /**<3rd byte */
        uint8_t b4; /**<4th byte */
    } u8x4;         /**<Four 8-bit unsigned integers */
} fsm_data_t;

/**
 *     FSM instance type
 */
typedef struct fsm_s * p_fsm_t;

/**
 *     Pointer to FSM state function
 *
 * @param[in]   fsm_inst - FMS instance
 */
typedef void (*pf_state_t)(const p_fsm_t fsm_inst);

/**
 *     State functions
 */
typedef struct
{
    pf_state_t on_entry;    /**<State entry function */
    pf_state_t on_activity; /**<State activity function */
    pf_state_t on_exit;     /**<State exit function */
    const char* name;       /**<Name of state - for debug purposes */
} fsm_state_cfg_t;

/**
 *     FSM Configuration table
 */
typedef struct
{
    const fsm_state_cfg_t * p_states;   /**<Pointer to states of FSM */
    const char *            name;       /**<Name of FSM machine */
    uint8_t                 num_of;     /**<Number of all states */
} fsm_cfg_t;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_init               (p_fsm_t * p_fsm_inst, const fsm_cfg_t * const p_cfg);
fsm_status_t fsm_is_init            (const p_fsm_t fsm_inst, bool * const p_is_init);
fsm_status_t fsm_reset              (const p_fsm_t fsm_inst);
fsm_status_t fsm_hndl               (const p_fsm_t fsm_inst);
fsm_status_t fsm_goto_state         (const p_fsm_t fsm_inst, const uint8_t state);
uint8_t      fsm_get_state          (const p_fsm_t fsm_inst);
uint32_t     fsm_get_duration       (const p_fsm_t fsm_inst);
void         fsm_reset_duration     (const p_fsm_t fsm_inst);
fsm_data_t   fsm_get_data           (const p_fsm_t fsm_inst);
void         fsm_set_data           (const p_fsm_t fsm_inst, const fsm_data_t data);
bool         fsm_get_first_entry    (const p_fsm_t fsm_inst);

#endif // __FSM_H_

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
