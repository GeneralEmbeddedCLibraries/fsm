// Copyright (c) 2023 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
* @file     fsm.h
* @brief    Finite State Machine (FSM)
*@author    Ziga Miklosic
*@email     ziga.miklosic@gmail.com
*@date      27.09.2023
*@version   V1.2.1
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
#define FSM_VER_MAJOR       ( 1 )
#define FSM_VER_MINOR       ( 2 )
#define FSM_VER_DEVELOP     ( 1 )

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
 *     Pointer to FSM state function
 */
typedef void (*pf_state_t)(void * p_ctx);

/**
 *     State functions
 */
typedef struct
{
    pf_state_t  func;    /**<Pointer to function to execute inside state */
    const char* name;    /**<Name of state - for debug purposes */
} fsm_state_cfg_t;

/**
 *     FSM Configuration table
 */
typedef struct
{
    fsm_state_cfg_t state[FSM_CFG_STATE_MAX];   /**<States of FSM */
    const char *    name;                       /**<Name of FSM machine */
    uint8_t         num_of;                     /**<Number of all states */
} fsm_cfg_t;

/**
 *     FSM instance type
 */
typedef struct fsm_s * p_fsm_t;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_init               (p_fsm_t * p_fsm_inst, const fsm_cfg_t * const p_cfg);
fsm_status_t fsm_is_init            (p_fsm_t fsm_inst, bool * const p_is_init);
fsm_status_t fsm_hndl               (p_fsm_t fsm_inst, void * p_ctx);
fsm_status_t fsm_goto_state         (p_fsm_t fsm_inst, const uint8_t state);
uint8_t      fsm_get_state          (const p_fsm_t fsm_inst);
uint32_t     fsm_get_duration       (const p_fsm_t fsm_inst);
bool         fsm_get_first_entry    (const p_fsm_t fsm_inst);

#endif // __FSM_H_

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
