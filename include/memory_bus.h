
#ifndef __MEMORY_BUS_H_
#define __MEMORY_BUS_H_

#include <stdbool.h>
#include <stdint.h>

enum bus_mode_t { DATA_READ, DATA_WRITE };
enum selected_device_t { NO_DEVICE_SELECTED, MEMORY_SELECTED, GRAPHICS_SELECTED, KEYBOARD_SELECTED };

typedef enum bus_mode_t bus_mode_t;
typedef enum selected_device_t selected_device_t;
typedef struct memory_bus_t memory_bus_t;

memory_bus_t* make_memory_bus(void);

//the device being read/written to is signaling the data is ready
void bus_set_device_ready(memory_bus_t* bus);
//the cpu must clear the ready signal after its read the data
//so we don't get confused
void bus_clear_device_ready(memory_bus_t* bus);
bool bus_is_device_ready(memory_bus_t* bus);

void bus_enable(memory_bus_t* bus);
void bus_disable(memory_bus_t* bus);
bool bus_is_enabled(memory_bus_t* bus);

void bus_set_read_operation(memory_bus_t* bus);
void bus_set_write_operation(memory_bus_t* bus);
bool bus_is_write_operation(memory_bus_t* bus);
bool bus_is_read_operation(memory_bus_t* bus);

void bus_set_address_lines(memory_bus_t* bus, uint32_t addr);
void bus_set_data_lines(memory_bus_t* bus, uint32_t data);
uint32_t bus_get_address_lines(memory_bus_t* bus);
uint32_t bus_get_data_lines(memory_bus_t* bus);

selected_device_t bus_get_selected_device(memory_bus_t* bus);

void bus_cycle(memory_bus_t* bus);

#endif
