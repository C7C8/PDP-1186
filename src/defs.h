#pragma once
#include <cstdint>

#define		UPPER_W(x)		((PBYTE)(x) & (PBYTE)0xFF00)
#define		LOWER_W(x)		((PBYTE)(x) & (PBYTE)0xFF)
#define 	COMPOSE(u,l)	((PBYTE)((u) << 8) | (PBYTE)((l) & 0xFF))
#define		NEG_BIT			(PWORD)(1<<15)
typedef 	uint16_t		PWORD;
typedef		uint8_t 		PBYTE;

