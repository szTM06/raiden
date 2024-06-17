Framework for running a normal NT driver on a separate "isolated" core.
This core acts as a standard thread running at a raised IRQL.

Features:
- automatically locates halted/disabled cores and wakes them
- resumable exception and interrupt handlers
- no hard to debug triple faults, unhandled exceptions trigger BSOD as expected for a standard NT driver
- compatibility with various NT kernel APIs, with some limitations (>=DISPATCH_LEVEL, and not all work)
- stable, tested on AMD and Intel, Win10 and Win11

Limitations:
- youre not really supposed to do this so a lot of kernel apis dont work
- you have to boot on less cores
- your "thread" can still be caught via hooks

the 'bootstrap' folder contains the code for bootstrapping the "woken" core, bringing it into the nt address space and calling the entry point.  
the 'nt' folder contains the nt side of the framework, enumerating cores, and handling IPIs.  
the 'rai' folder contains the code for controlling the woken core, contains interrupt and exception handlers, plus various QoL functions.  
