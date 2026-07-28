# **FSM - Finite State Machine**
Finite State Machine (FSM) module is implemented in C language for usage in embedded system in order to provide backbone of FSM hidden behind intuitive API design. 

FSM module is implemented in object oriented fashion as it provide to have multiple FSM objects as fully separated, individual instances. Each instance of FSM can be configured differently in order to fulfill application needs.

## **Dependencies**

FSM module does not have any dependencies.

## **Limitations**

### **1. Multientry**
Module is not written to be used on multi core/task/interrupts systems.

## **General Embedded C Libraries Ecosystem**
In order to be part of *General Embedded C Libraries Ecosystem* this module must be placed in following path: 

```
root/middleware/fsm/fsm/"module_space"
```

 ## **API**
| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **fsm_init**              | Initialization of FSM module, using dynamically (malloc) allocated instance | fsm_status_t fsm_init(p_fsm_t * p_fsm_inst, const fsm_cfg_t * const p_cfg) |****
| **fsm_init_static**       | Initialization of FSM module, using caller-provided (static) instance | fsm_status_t fsm_init_static(fsm_t * fsm_inst, const fsm_cfg_t * const p_cfg) |
| **fsm_is_init**           | Get initialization flag                   | bool fsm_is_init(const p_fsm_t fsm_inst) |
| **fsm_reset**             | Reset FSM handler                         | fsm_status_t fsm_reset(const p_fsm_t fsm_inst) |
| **fsm_hndl**              | FSM handler                               | fsm_status_t fsm_hndl(const p_fsm_t fsm_inst) |
| **fsm_goto_state**        | Change FSM state                          | fsm_status_t fsm_goto_state(const p_fsm_t fsm_inst, const uint8_t state) |
| **fsm_get_state**         | Get current FSM state                     | uint8_t fsm_get_state(const p_fsm_t fsm_inst) |
| **fsm_get_prev_state**    | Get previous FSM state                    | uint8_t fsm_get_prev_state(const p_fsm_t fsm_inst) |
| **fsm_get_duration**      | Get time spend in state in miliseconds    | uint32_t fsm_get_duration(const p_fsm_t fsm_inst) |
| **fsm_reset_duration**    | Reset time spend in state                 | void fsm_reset_duration(const p_fsm_t fsm_inst) |
| **fsm_get_data**          | Get (read) data from FSM                  | fsm_data_t fsm_get_data(const p_fsm_t fsm_inst) |
| **fsm_set_data**          | Set (write) data to FSM                   | void fsm_set_data(const p_fsm_t fsm_inst, const fsm_data_t data) |
| **fsm_get_first_entry**   | Get first time state entry flag           | bool fsm_get_first_entry(const p_fsm_t fsm_inst) |

## **Usage**

**GENERAL NOTICE: Put all user code between sections: USER CODE BEGIN & USER CODE END!**

1. Copy template files to root directory of module.

2. Configure FSM module for application needs. Configuration options are following:

| Macros | Description | 
| ------------- | ----------- |
| FSM_GET_SYSTICK       | Get system timetick in 32-bit form |
| FSM_CFG_DEBUG_EN      | Enable/Disable debug mode |
| FSM_CFG_ASSERT_EN     | Enable/Disable assertions |
| FSM_DBG_PRINT         | Printing to debug channel |
| FSM_ASSERT            | Assert actions definition |

The example below implements a small "desired motor rotor angle selector" that picks up its
value from one of three possible input sources (potentiometer, SSI encoder, or a Hall
sensor knob), starting from a power-on-reset/idle state that waits for one of the sources
to be detected.

3. Create enumeration for FSM states
```C
/**
 * 	APP FSM states
 */
typedef enum
{
    eAPP_FSM_POR = 0,   // Power-on-reset / idle, waiting for input source to be detected
    eAPP_FSM_POT,       // Potentiometer is the active input source
    eAPP_FSM_SSI,       // SSI encoder is the active input source
    eAPP_FSM_HALL,      // Hall sensor knob is the active input source

    eAPP_FSM_NUM_OF
} app_fsm_state_t;
```

4. Create configuration table for FSM. Here user registers state handlers as pointer functions.

```C
/**
 * 	APP FSM State Configurations
 */
static const fsm_cfg_t g_boot_fsm_cfg_table =
{
    .p_states = (fsm_state_cfg_t[])
    {
        // FSM state           On state entry handler          Normal handler                        On state exit handler         State name
        // ---------------------------------------------------------------------------------------------------------------------------------------
        [eAPP_FSM_POR]      = {.on_entry=app_fsm_generic_entry,  .on_activity=app_fsm_por_hndl,       .on_exit=app_fsm_generic_exit, .name="POR"     },
        [eAPP_FSM_POT]      = {.on_entry=app_fsm_pot_mode_entry, .on_activity=app_fsm_pot_mode_hndl,  .on_exit=app_fsm_pot_mode_exit,.name="POT"     },
        [eAPP_FSM_SSI]      = {.on_entry=app_fsm_generic_entry,  .on_activity=app_fsm_ssi_mode_hndl,  .on_exit=app_fsm_generic_exit, .name="SSI"     },
        [eAPP_FSM_HALL]     = {.on_entry=app_fsm_generic_entry,  .on_activity=app_fsm_hall_mode_hndl, .on_exit=app_fsm_generic_exit, .name="HAL"     },
    },
    .name   = "App FSM",
    .num_of = eAPP_FSM_NUM_OF,
};
```

5. Create variable for FSM instance
```C
/**
 * 	App FSM instance
 */
static p_fsm_t g_app_fsm = NULL;
```

6. Initialize FSM instance

Dynamically (malloc) allocated instance, via *fsm_init*:
```C
if ( eFSM_OK != fsm_init( &g_app_fsm, &g_boot_fsm_cfg_table ))
{
    // Initialization failed...
    // Further actions here...
}
```

Alternatively, using caller-provided (static, malloc-free) storage, via *fsm_init_static*. This requires
the actual `fsm_t` instance (not just the `p_fsm_t` handle from step 5) to be allocated by the caller:
```C
/**
 * 	App FSM instance storage (replaces "static p_fsm_t g_app_fsm = NULL;" from step 5)
 */
static fsm_t   g_app_fsm_inst = {0};
static p_fsm_t g_app_fsm = &g_app_fsm_inst;

if ( eFSM_OK != fsm_init_static( g_app_fsm, &g_boot_fsm_cfg_table ))
{
    // Initialization failed...
    // Further actions here...
}
```

7. Implement state hooks

`on_entry`/`on_exit` don't need to be unique per state — here POR, SSI and HALL share a
generic logging pair, while POT gets its own so it can reset its input filter on entry.
Note that inside `on_entry` the FSM has already moved to the new state, so
*fsm_get_state*/*fsm_get_prev_state* report "entering *this* state, coming from *that*
state"; inside `on_exit` the FSM is still in the state being left, so only
*fsm_get_state* is meaningful there.
```C
static void app_fsm_generic_entry(const p_fsm_t fsm_inst)
{
    FSM_DBG_PRINT( "Selector: %d -> %d", fsm_get_prev_state( fsm_inst ), fsm_get_state( fsm_inst ));
}

static void app_fsm_generic_exit(const p_fsm_t fsm_inst)
{
    FSM_DBG_PRINT( "Selector: leaving %d", fsm_get_state( fsm_inst ));
}

static void app_fsm_por_hndl(const p_fsm_t fsm_inst)
{
    // Detection of a connected input source happens elsewhere (e.g. a HW presence
    // poll/ISR) and calls fsm_goto_state() directly - see step 9.
    (void) fsm_inst;
}
```

The potentiometer input needs debouncing before its reading is trusted: a fresh sample is
taken on first entry, a large jump restarts the settle window via *fsm_reset_duration*, and
once the value has been stable for long enough *fsm_get_duration* confirms it can be used.
The last accepted raw sample is carried between calls with *fsm_get_data*/*fsm_set_data*.
```C
#define APP_FSM_POT_SETTLE_TIME_MS      ( 200U )
#define APP_FSM_POT_JUMP_THRESHOLD      ( 50U )

static void app_fsm_pot_mode_entry(const p_fsm_t fsm_inst)
{
    app_fsm_generic_entry( fsm_inst );

    // Start with an empty filter - first sample is taken on first activity call
    fsm_set_data( fsm_inst, (fsm_data_t) { .u32 = 0U } );
}

static void app_fsm_pot_mode_hndl(const p_fsm_t fsm_inst)
{
    const uint32_t pot_raw = app_pot_read_raw(); // application-specific ADC read

    // First sample after entry - take it as baseline and (re)start the settle window
    if ( true == fsm_get_first_entry( fsm_inst ))
    {
        fsm_set_data( fsm_inst, (fsm_data_t) { .u32 = pot_raw } );
        fsm_reset_duration( fsm_inst );
    }
    else
    {
        const fsm_data_t prev_data = fsm_get_data( fsm_inst );
        const uint32_t   pot_dlt   = ( pot_raw > prev_data.u32 ) ? ( pot_raw - prev_data.u32 ) : ( prev_data.u32 - pot_raw );

        // Knob is still moving - restart the settle window
        if ( pot_dlt >= APP_FSM_POT_JUMP_THRESHOLD )
        {
            fsm_set_data( fsm_inst, (fsm_data_t) { .u32 = pot_raw } );
            fsm_reset_duration( fsm_inst );
        }

        // Value has been stable for long enough - accept it as the desired temperature
        else if ( fsm_get_duration( fsm_inst ) >= APP_FSM_POT_SETTLE_TIME_MS )
        {
            app_desired_temp_set_from_pot( pot_raw ); // application-specific action
        }
        else
        {
            // Still inside the settle window - do nothing yet
        }
    }
}

static void app_fsm_pot_mode_exit(const p_fsm_t fsm_inst)
{
    app_fsm_generic_exit( fsm_inst );
}
```

SSI and HALL activity handlers follow the same shape as `app_fsm_por_hndl` above (read
their respective source, act on it) and are omitted here for brevity.

8. Handle FSM instance
```C
// This is cyclic function
static void app_100ms_hndl(void)
{
    // Only handle once the FSM has been successfully initialized
    if ( true == fsm_is_init( g_app_fsm ))
    {
        fsm_hndl( g_app_fsm );
    }
}
```

9. Change FSM state

Called whenever the application detects which input source is physically connected:
```C
static void app_selector_set_mode(const app_sel_mode_t mode)
{
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
}
```

10. Reset FSM instance (fault recovery)

Unlike *fsm_goto_state*, *fsm_reset* forces the FSM straight back to its initial state
without running any `on_exit`/`on_entry` hooks - useful for a watchdog/fault monitor that
needs a known-safe state without side effects from the normal transition path:
```C
static void app_selector_fault_monitor_hndl(void)
{
    if ( true == app_selector_fault_detected()) // application-specific fault check
    {
        fsm_reset( g_app_fsm );
    }
}
```
