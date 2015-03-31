
//This collection of bit-twiddling macros is intended to make life easier by
//reducing repetitive code for setting/clearing/checking bits in our registers

#ifndef __BIT_TWIDDLING_H_
#define __BIT_TWIDDLING_H_

#define BIT_SET(reg, bit) (reg = reg | ((1u) << bit))
#define BIT_CLEAR(reg, bit) (reg = reg & ~((1u) << bit))
#define BIT_TOGGLE(reg, bit) (reg = reg ^ ((1u) << bit))

#define INCLUSIVE_RANGE_SIZE(start, end)    ((end) - (start) + 1)

//This macro generates a bitvector of consecutive ones that are right-justified
#define GENERATE_ONES(number_of_ones)   (((1u) << (number_of_ones)) - 1)

//This macro generates a bitmask of one's that is as wide as requested that is aligned with the bits of interest
#define GENERATE_MULTIBIT_MASK(start_bit, end_bit)   (GENERATE_ONES(INCLUSIVE_RANGE_SIZE(start_bit, end_bit)) << start_bit)


#if 0
//This macro clears all of the bits between start_bit and end_bit
#define MULTI_BIT_CLEAR(reg, start_bit, end_bit)    (reg = reg & ~(GENERATE_MULTIBIT_MASK((end_bit - start_bit)) << start_bit))

//This macro clears all of the bits between start_bit and end_bit
#define MULTI_BIT_SET(reg, start_bit, end_bit)  (reg = reg | (GENERATE_MULTIBIT_MASK((end_bit - start_bit)) << start_bit))
#endif

//This macro gets all of the bits within the specified range between start_bit and end_bit (the value is right-justified)
#define GET_BITS_IN_RANGE(reg, start_bit, end_bit)  (((reg) & GENERATE_MULTIBIT_MASK(start_bit, end_bit)) >> start_bit)

//This macro writes the requested value between the start and end bits
#define MULTI_BIT_WRITE_VALUE(reg, value, start_bit, end_bit) do {\
    MULTI_BIT_CLEAR(reg, start_bit, end_bit); \
    (reg = reg | (value << start_bit)); \
} while(0)

#define CHECK_BIT_SET(reg, bit)  (reg & ((1u) << bit))
#define CHECK_BIT_CLEAR(reg, bit) (! CHECK_BIT_SET(reg, bit) )



#endif
