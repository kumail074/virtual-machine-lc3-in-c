A C implementation of the **LC-3** virtual machine, an educational computer architecture.


## DESCRIPTION

It containes **16 bits** (or 2 bytes) address space.  
**First 4 bits** [15:12] specify the opcode(operation to be performed), remaining bits [11:0] provide further information.  
**Eight 16-bit registers**, (numbered from 000 to 111) to hold values/result.  
**Three 1-bit registers** for condition codes:  
- **N** (negative)
- **Z** (zero)
- **P** (positive)

**Processor Status Register** (PSR): 16-bit register, containing status information about the currently executing process:  
- **PSR[10:8]** defines **priority level** (from PL7 to PL0, where PL7 is highest)  
- **PSR[15]** defines **privilege mode** e.i. supervisor mode (privileged and PSR[15] = 0) and user mode (unprivileged and PSR[15] = 1)
- **PSR[2:0]** contains the condition codes
- **PSR[2]** is N, PSR[1] is Z, PSR[0] is P
- **PSR[14:3]** are reserved (unused)
