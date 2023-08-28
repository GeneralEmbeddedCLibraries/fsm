# **FSM - Finite State Machine**
This module contains FSM C code implementation for general purpose usage. Each FSM are created as individual, separated instances so different instances of FSM can be configured differently in order to addopt application needs.

For now this module is not multientry. 

## **Dependencies**

No dependencies.

## **General Embedded C Libraries Ecosystem**
In order to be part of *General Embedded C Libraries Ecosystem* this module must be placed in following path: 

```
root/middleware/fsm/fsm/"module_space"
```

 ## **API**
| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **fsm_init** | Initialization of FSM module | fsm_status_t fsm_init(p_fsm_t * p_fsm_inst, const fsm_cfg_t * const p_cfg) |****
| **fsm_is_init** | Get initialization flag | fsm_status_t fsm_is_init(p_fsm_t fsm_inst, bool * const p_is_init) |
| **fsm_hndl** | FSM main handler | fsm_status_t fsm_hndl(p_fsm_t fsm_inst) |
| **fsm_goto_state** | Change FSM state | fsm_status_t fsm_goto_state(p_fsm_t fsm_inst, const uint8_t state) |
| **fsm_get_state** | Get current FSM state | uint8_t fsm_get_state(p_fsm_t fsm_inst) |
| **fsm_get_duration** | Get time spend in state in miliseconds | uint32_t fsm_get_duration(p_fsm_t fsm_inst) |
| **fsm_get_first_entry** | Get first time state entry flag | bool fsm_get_first_entry(p_fsm_t fsm_inst) |

## **Usage**

**GENERAL NOTICE: Put all user code between sections: USER CODE BEGIN & USER CODE END!**

1. Copy template files to root directory of module.

2. Configure FSM module for application needs. Configuration options are following:

| Macros | Description | Range | Default | 
| ------------- | ----------- | ----- | --- |
| FSM_CFG_DEBUG_EN | Enable/Disable debug mode | 0-1 | 1
| FSM_CFG_ASSERT_EN | Enable/Disable assertions | 0-1 | 1

| Functions | Description | Comment |
| ------------- | ----------- | ----- |
| FSM_DBG_PRINT | Printing to debug channel | Used only if FSM_CFG_DEBUG_EN = 1 |
| FSM_ASSERT | Assert actions definition | Used only if FSM_CFG_ASSERT_EN = 1 |

1. Create enumeration for FSM states
```C
/**
 * 	APP FSM states
 */
typedef enum
{
	eAPP_FSM_POR = 0,
	eAPP_FSM_POT,
	eAPP_FSM_SSI,
	eAPP_FSM_HALL,

	eAPP_FSM_NUM_OF
} app_fsm_state_t;
```

2. Create configuration table for FSM. Here user registers state handlers as pointer functions.

```C
/**
 * 	APP FSM State Configurations
 */
const static fsm_cfg_t g_fsm_cfg_table =
{
	/**
	 * 		State functions
	 *
	 * 	NOTE: Sequence matters!
	 */
	.state = { 	{ .func = app_fsm_por_mode_hndl, 	.name = "POR" },
				{ .func = app_fsm_pot_mode_hndl, 	.name = "POT" },
				{ .func = app_fsm_ssi_mode_hndl, 	.name = "SSI" },
				{ .func = app_fsm_hall_mode_hndl, 	.name = "HALL" },
			},
	.name = "App FSM",
	.num_of = eAPP_FSM_NUM_OF,
	.period = 100UL // ms
};
```

3. Create variable for FSM instance
```C
/**
 * 	App FSM instance
 */
static p_fsm_t g_app_fsm = NULL;
```

4. Initialize FSM instance
```C
if ( eFSM_OK != fsm_init( &g_app_fsm, &g_fsm_cfg_table ))
{
    // Initialization failed...
    // Further actions here...
}
```

5. Handle FSM instance
```C
// This is cyclic function
static void app_100ms_hndl(void)
{
	// Handle app FSM
	fsm_hndl( g_app_fsm );
}

```

6. Change state mode, checking first entry (example usage)

Example of mode change:
```C
switch( mode )
{
    case eSEL_POT:
        fsm_goto_state( g_app_fsm, eAPP_FSM_POT );
        break;

    case eSEL_SSI:
        fsm_goto_state( g_app_fsm, eAPP_FSM_SSI );
        break;

    case eSEL_HALL:
        fsm_goto_state( g_app_fsm, eAPP_FSM_HALL );
        break;

    default:
        // No actions...
        break;
}
```

Checking first entry:
```C
static void app_fsm_hall_mode_hndl(void)
{
    // First entry
    if ( true == fsm_get_first_entry( g_app_fsm ))
    {
        // Action on first entry into "eAPP_FSM_HALL" mode
    }

    // Other state actions
}
```
