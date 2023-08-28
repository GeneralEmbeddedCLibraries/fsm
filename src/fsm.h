// Copyright (c) 2023 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
* @file     fsm.h
* @brief    Finite State Machine (FSM)
*@author    Ziga Miklosic
*@email     ziga.miklosic@gmail.com
*@date      28.08.2023
*@version   V1.1.0
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
#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 * 	Module version
 */
#define FSM_VER_MAJOR		( 1 )
#define FSM_VER_MINOR		( 1 )
#define FSM_VER_DEVELOP		( 0 )

/**
 * Float 32-bit definition
 */
typedef float float32_t;

/**
 * 	FSM status
 */
typedef enum
{
	eFSM_OK 		= 0,		/**<Normal operation */
	eFSM_ERROR		= 0x01,		/**<General error */
	eFSM_ERROR_INIT	= 0x02,		/**<Initialization error */
} fsm_status_t;

/**
 * 	Pointer to FSM state function
 */
typedef void (*pf_state_t)(void);

/**
 * 	State functions
 */
typedef struct
{
	pf_state_t 	func;	/**<Pointer to function to execute inside state */
	const char* name;	/**<Name of state - for debug purposes */
} fsm_state_cfg_t;

/**
 * 	FSM Configuration table
 */
typedef struct
{
	fsm_state_cfg_t		state[8];	/**<States of FSM */
	const char *		name;		/**<Name of FSM machine */
	float32_t			period;		/**<Period of FSM handling in user specific unit */
	uint8_t	 			num_of;		/**<Number of all states */
} fsm_cfg_t;

/**
 * 	FSM instance type
 */
typedef struct fsm_s * p_fsm_t;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_init               (p_fsm_t * p_fsm_inst, const fsm_cfg_t * const p_cfg);
fsm_status_t fsm_is_init            (p_fsm_t fsm_inst, bool * const p_is_init);
fsm_status_t fsm_hndl               (p_fsm_t fsm_inst);
fsm_status_t fsm_goto_state         (p_fsm_t fsm_inst, const uint8_t state);
uint8_t      fsm_get_state          (p_fsm_t fsm_inst);
float32_t    fsm_get_duration       (p_fsm_t fsm_inst);
bool         fsm_get_first_entry    (p_fsm_t fsm_inst);

#endif // __FSM_H_
////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
