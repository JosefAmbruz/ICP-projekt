# Communication protocol:

## FSM -> CLIENT

| type                       | payload                       |
|----------------------------|-------------------------------|
| FSM_CONNECTED              | {message}                     |
| FSM_STARTED                | {start_state}                 |
| FSM_ERROR                  | {message}                     |
| CURRENT_STATE              | {state, is_finish}            |
| STATE_ACTION_EXECUTED      | {state_name}                  |
| TRANSITION_TAKEN           | {from_state, to_state, delay} |
| TRANSITION_ACTION_EXECUTED | {from_state, to_state}        |
| VARIABLE_UPDATE            | {name, value}                 |
| FSM_FINISHED               | {finish_state}                |


## CLIENT -> FSM

| type                       | payload                |
|----------------------------|------------------------|
| SET_VARIABLE               | {name, value}          |
| STOP_FSM                   | {}                     |

