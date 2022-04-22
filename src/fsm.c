// Copyright (c) 2022 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
* @file     fsm.h
* @brief    FSM manipulations
* @author   Ziga Miklosic
* @date     22.04.2022
* @version	V1.0.0
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
 * 	FSM States
 */
typedef struct
{
	uint8_t cur;		/**<Current state */
	uint8_t next;		/**<Next/Requested state */
} fsm_state_t;

/**
 * 	FSM data
 */
typedef struct fsm_s
{
	fsm_cfg_t *	p_cfg;			/**<FSM setup */
	uint32_t 	loop_cnt;		/**<Loop counter - number of loops inside state */
	fsm_state_t	state;			/**<Current state of FSM */
	bool		first_entry;	/**<First entry of state */
	bool		is_init;		/**<Initialization guard */
} fsm_t;

/**
 * 	Limit loop counts
 *
 * 	Unit: period of fsm handler
 */
#define FSM_LIMIT_LOOP_CNT(cnt)					(( cnt >= 0x1FFFFFFFUL ) ? ( 0x1FFFFFFFUL ) : ( cnt ))

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
static void fsm_manager(p_fsm_t fsm_inst);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
/**
*   	FSM manager
*
* @param[in]	fsm_inst	- FSM instance
* @return   	void
*/
////////////////////////////////////////////////////////////////////////////////
static void fsm_manager(p_fsm_t fsm_inst)
{
	// State change
	if ( fsm_inst->state.cur != fsm_inst->state.next )
	{
		FSM_DBG_PRINT( "%s transition: %d -> %d", fsm_inst->p_cfg->name, fsm_inst->state.cur, fsm_inst->state.next );

		fsm_inst->state.cur 	= fsm_inst->state.next;
		fsm_inst->first_entry 	= true;
		fsm_inst->loop_cnt 		= 0UL;
	}

	// Same state
	else
	{
		fsm_inst->state.next = fsm_inst->state.cur;
		fsm_inst->first_entry = false;

		// Measure time
		fsm_inst->loop_cnt += 1UL;
		fsm_inst->loop_cnt = FSM_LIMIT_LOOP_CNT( fsm_inst->loop_cnt );
	}
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
* 	Following functions are part of API calls.
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialise FSMs
*
* @param[out]	p_fsm_inst	- Pointer to FSM instance
* @param[in]	p_cfg		- Pointer to FSM configuration table
* @return   	status		- Status of initialisation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_init(p_fsm_t * p_fsm_inst, const fsm_cfg_t * const p_cfg)
{
	fsm_status_t status = eFSM_OK;

	FSM_ASSERT( NULL != p_fsm_inst );
	FSM_ASSERT( NULL != p_cfg );

	if 	(	( NULL != p_fsm_inst )
		&&	( NULL != p_cfg ))
	{
		// Allocate space
		*p_fsm_inst = malloc( sizeof(p_fsm_t));

		FSM_ASSERT( NULL != *p_fsm_inst );

		// Check if allocation succeed
		if ( NULL != *p_fsm_inst )
		{
			// Get setup
			(*p_fsm_inst)->p_cfg = (fsm_cfg_t*) p_cfg;

			// Init internal data
			(*p_fsm_inst)->state.cur = 0;
			(*p_fsm_inst)->state.next = (*p_fsm_inst)->state.cur;
			(*p_fsm_inst)->first_entry = false;
			(*p_fsm_inst)->loop_cnt = 0;
			(*p_fsm_inst)->is_init = true;
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
* @param[in]	fsm_inst	- FSM instance
* @param[out]	p_is_init 	- Initialisation flag
* @return   	status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_is_init(p_fsm_t fsm_inst, bool * const p_is_init)
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
*   	FSM handler
*
* @note 	Each instance of FSM must call its own handler!
*
* @param[in]	fsm_inst	- FSM instance
* @return   	status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_hndl(p_fsm_t fsm_inst)
{
	fsm_status_t status = eFSM_OK;

	if ( NULL != fsm_inst )
	{
		if ( false == fsm_inst->is_init )
		{
			// Manage FSM
			fsm_manager( fsm_inst );

			// Execute current FSM state
			if ( NULL != fsm_inst->p_cfg->func[ fsm_inst->state.cur ] )
			{
				fsm_inst->p_cfg->func[ fsm_inst->state.cur ]();
			}
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
*   	Set next FSM state
*
* @param[in]	fsm_inst	- FSM instance
* @param[in]	state 		- Next state
* @return   	status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_goto_state(p_fsm_t fsm_inst, const uint8_t state)
{
	fsm_status_t status = eFSM_OK;

	if ( NULL != fsm_inst )
	{
		if ( state < fsm_inst->p_cfg->num_of )
		{
			fsm_inst->state.next = state;
		}
		else
		{
			status = eFSM_ERROR;
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
*   	Get current FSM state
*
* @param[in]	fsm_inst	- FSM instance
* @param[out]	p_state 	- Pointer to current FMS state
* @return   	status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_get_state(p_fsm_t fsm_inst, uint8_t * const p_state)
{
	fsm_status_t status = eFSM_OK;

	if 	(	( NULL != fsm_inst )
		&& 	( NULL != p_state ))
	{
		*p_state = fsm_inst->state.cur;
	}
	else
	{
		status = eFSM_ERROR;
	}


	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   	Get FSM state duration in counts of loop
*
* @note 	This function does not return absolute time of current state
* 			duration but rather number of time that it is being
* 			executed.
*
* @param[in]	fsm_inst		- FSM instance
* @param[out]	p_loop_cnt 		- Pointer to loop counts
* @return   	status			- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_get_duration(p_fsm_t fsm_inst, uint32_t * const p_loop_cnt)
{
	fsm_status_t status = eFSM_OK;

	if 	(	( NULL != fsm_inst )
		&& 	( NULL != p_loop_cnt ))
	{
		*p_loop_cnt = fsm_inst->loop_cnt;
	}
	else
	{
		status = eFSM_ERROR;
	}


	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   	Get first state entry flag
*
* @param[in]	fsm_inst		- FSM instance
* @param[out]	p_first_entry 	- Pointer to first entry
* @return   	status			- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
fsm_status_t fsm_get_first_entry(p_fsm_t fsm_inst, bool * const p_first_entry)
{
	fsm_status_t status = eFSM_OK;

	if 	(	( NULL != fsm_inst )
		&& 	( NULL != p_first_entry ))
	{
		*p_first_entry = fsm_inst->first_entry;
	}
	else
	{
		status = eFSM_ERROR;
	}


	return status;
}

////////////////////////////////////////////////////////////////////////////////
/*!
 * @} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////
