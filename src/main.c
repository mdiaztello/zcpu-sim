


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "cpu.h"
//#include "graphics.h"
//#include "keyboard.h"

// include any SDL stuff here






int main(void)
{

    cpu_t* cpu = make_cpu();
    init_cpu(cpu);
    cpu_cycle(cpu);
    cpu_cycle(cpu);
    cpu_cycle(cpu);
    cpu_cycle(cpu);


    return 0;
}
