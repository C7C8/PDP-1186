#pragma once
#include "defs.h"

#define 	REGCOUNT	8
#define		SC			(PWORD)1
#define		SV			(PWORD)(1<<1)
#define		SZ			(PWORD)(1<<2)
#define		SN			(PWORD)(1<<3)
#define		ST			(PWORD)(1<<4)

//Flags for the status word of an x86 processor, since inline asm is used in some places
#define		SC_86		1
#define		SV_86		(1<<11)
#define		SZ_86		(1<<6)
#define		SN_86		(1<<7)


//r7 reserved for use as program counter, r6 reserved for stack pointer
enum RegCode {R0 = 0, R1, R2, R3, R4, R5, R6, R7, SP = R6, PC = R7};
enum AdrMode {};

class Processor {
public:
	Processor();
	Processor(const Processor& cpu);
	~Processor();
	void operator=(const Processor& cpu);

	//Registers & status functions
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

	/**************
	 * INSTRUCTIONS
	 **************/

	//Zero operand instructions
	void halt();
	void wait();
	void reset();
	void nop();

	/*
	 * Note that from here on out, everything here will expect *pointers* to memory locations to be provided. Therefore,
	 * addressing modes need to be handled by the *calling* entity. This is done to avoid excessive code duplication.
	 */

	//One-operand instructions
	void clr(PWORD* o1);
	void inc(PWORD* o1);
	void dec(PWORD* o1);
	void adc(PWORD* o1);
	void sbc(PWORD* o1);
	void tst(PWORD* o1);
	void neg(PWORD* o1);
	void com(PWORD* o1);
	void ror(PWORD* o1);
	void rol(PWORD* o1);
	void asr(PWORD* o1);
	void asl(PWORD* o1);
	void swab(PWORD* o1);
	void sxt(PWORD* o1);

	//One-and-a-half-operand instructions
	void mul(RegCode reg, PWORD* o2);
	void div(RegCode reg, PWORD* o2);
	void ash(RegCode reg, PWORD* o2);
	void ashc(RegCode reg, PWORD* o2);
	void xor_(RegCode reg, PWORD* o2); //actually xor in disguise, xor is a cpp keyword

	//Two-operand instructions
	void mov(PWORD* o1, PWORD* o2);
	void add(PWORD* o1, PWORD* o2);
	void sub(PWORD* o1, PWORD* o2);
	void cmp(PWORD* o1, PWORD* o2);
	void bis(PWORD* o1, PWORD* o2);
	void bic(PWORD* o1, PWORD* o2);
	void bit(PWORD* o1, PWORD* o2);

	//Branch instructions
	void br(PWORD* ost);
	void bne(PWORD* ost);
	void beq(PWORD* ost);
	void bpl(PWORD* ost);
	void bmi(PWORD* ost);
	void bvc(PWORD* ost);
	void bvs(PWORD* ost);
	void bhis(PWORD* ost);
	void bcc(PWORD* ost);
	void blo(PWORD* ost);
	void bcs(PWORD* ost);
	void bge(PWORD* ost);
	void blt(PWORD* ost);
	void bgt(PWORD* ost);
	void ble(PWORD* ost);
	void bhi(PWORD* ost);
	void blos(PWORD* ost);

	//Control transfer instructions
	void jmp(PWORD* ost);
	void sob(RegCode reg, PWORD* ost);
	void jsr(RegCode reg, PWORD* ost);
	void rts(RegCode reg);
	void rti();
	void trap(PWORD n);
	void bpt(PWORD n);
	void iot();
	void emt();
	void rtt();

	//Status word instructions
	void spl(PBYTE* lvl);
	void clc();
	void clv();
	void clz();
	void cln();
	void sec();
	void sev();
	void sez();
	void sen();
	void ccc();
	void scc();

private:
	inline void branch(PWORD offset) { registers[PC] += 2* offset;} //<! Laziness.
	inline bool overflow(PWORD o1, PWORD o2, PWORD res);
	inline void valFlags(PWORD o1, PWORD o2, PWORD res);
	inline void bitFlags(PWORD o1, PWORD o2, PWORD res);
	inline void x86Flags(uint64_t flags);

	PWORD registers[REGCOUNT];
	PBYTE ps;
	bool halted;
	union {
		PBYTE* byte;
		PWORD* word;
	} core;
	int coreSizeBytes;
};
