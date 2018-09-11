==SEGS Action Replay utility.==

The `action_replay` application is reproducing the state of the server components to debug a crash or reproduce an invalid state.
The support `replay_lib` library can also be used by unit tests to test regressions.


* Usage:
```
action_replay dump_file_name [steps]
```
Running the action_replay will read the dump file, recreate all EventProcessors defined inside, deserialize their state and fill their event queues with deserialized events.
Then all EventProcessors will be ran by processing the events step-by-step, either until the `steps` count is reached, or the event queues are empty.
