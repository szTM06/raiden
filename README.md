Framework for running a normal NT driver on a separate "isolated" core.
This core acts as a standard thread running at a raised IRQL.

Limitations:
- youre not really supposed to do this so a lot of kernel apis dont work
- you have to boot on less cores
- your "thread" can still be caught via hooks
