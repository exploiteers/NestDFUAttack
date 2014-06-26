/* bits.h
 *
 * Copyright (c) 2010 Nest Labs, Inc.
 *
 * Copyright (c) 2004 Texas Instruments
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef __bits_h
#define __bits_h 1

#define ARM_REG_MASK(bits)						((1 << (bits)) - 1)
#define ARM_REG_VAL(bit, mask)					((mask) << (bit))
#define ARM_REG_VAL_ENCODE(shift, mask, value)	(((value) << (shift)) & (mask))
#define ARM_REG_VAL_DECODE(shift, mask, value)	(((value) & (mask)) >> (shift))


#define BIT0  ARM_REG_VAL( 0, 1)
#define BIT1  ARM_REG_VAL( 1, 1)
#define BIT2  ARM_REG_VAL( 2, 1)
#define BIT3  ARM_REG_VAL( 3, 1)
#define BIT4  ARM_REG_VAL( 4, 1)
#define BIT5  ARM_REG_VAL( 5, 1)
#define BIT6  ARM_REG_VAL( 6, 1)
#define BIT7  ARM_REG_VAL( 7, 1)
#define BIT8  ARM_REG_VAL( 8, 1)
#define BIT9  ARM_REG_VAL( 9, 1)
#define BIT10 ARM_REG_VAL(10, 1)
#define BIT11 ARM_REG_VAL(11, 1)
#define BIT12 ARM_REG_VAL(12, 1)
#define BIT13 ARM_REG_VAL(13, 1)
#define BIT14 ARM_REG_VAL(14, 1)
#define BIT15 ARM_REG_VAL(15, 1)
#define BIT16 ARM_REG_VAL(16, 1)
#define BIT17 ARM_REG_VAL(17, 1)
#define BIT18 ARM_REG_VAL(18, 1)
#define BIT19 ARM_REG_VAL(19, 1)
#define BIT20 ARM_REG_VAL(20, 1)
#define BIT21 ARM_REG_VAL(21, 1)
#define BIT22 ARM_REG_VAL(22, 1)
#define BIT23 ARM_REG_VAL(23, 1)
#define BIT24 ARM_REG_VAL(24, 1)
#define BIT25 ARM_REG_VAL(25, 1)
#define BIT26 ARM_REG_VAL(26, 1)
#define BIT27 ARM_REG_VAL(27, 1)
#define BIT28 ARM_REG_VAL(28, 1)
#define BIT29 ARM_REG_VAL(29, 1)
#define BIT30 ARM_REG_VAL(30, 1)
#define BIT31 ARM_REG_VAL(31, 1)

#endif
