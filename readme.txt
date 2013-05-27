
This simulator is intended to be a "quick-and-dirty" implementation of the
full-blown computer system. The processor will not actually do cycle accurate
simulation of what will happen. Instead, the opcodes will just be implemented
in the most convenient fashion possible. The simulator will, however, deal
with I/O for the keyboard and monitor. This will act as an "executable spec"
that the real hardware can be compared against later.  This simulator should
also allow me to test the compiler and OS independently of having functioning
hardware.  If more accurate simulation is required, this project maintain the
changes in separate repository branches to facilitate switching back and forth
between simulator versions.

