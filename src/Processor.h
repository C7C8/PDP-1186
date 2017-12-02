#pragma once
#include "defs.h"


#define 	REGCOUNT	8
#define		SC			1
#define		SV			1<<1
#define		SZ			1<<2
#define		SN			1<<3
#define		ST			1<<4


//r7 reserved for use as program counter, r6 reserved for stack pointer
enum RegCode {R0 = 0, R1, R2, R3, R4, R5, R6, R7, SP=R6, PC = R7};


class Processor {
public:
	Processor();
	Processor(const Processor& cpu);
	void operator=(const Processor& cpu);

	PWORD reg(RegCode reg) const;
	void reg(RegCode reg, PWORD val);

	PWORD pstat() const;
	bool pstat_carry() const;
	bool pstat_overf() const;
	bool pstat_zero() const;
	bool pstat_neg() const;
	bool pstat_trap() const;
	PWORD priority() const;
	void priority(PWORD prty);
private:
	PWORD registers[REGCOUNT];
	PWORD ps;
};
