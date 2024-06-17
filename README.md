Framework for running a normal NT driver on a seperate "isolated" core.

Limitations:
- youre not really supposed to do this so a lot of kernel apis dont work
- you have to boot on less cores
