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
	void tst(const PWORD* o1);
	void neg(PWORD* o1);
	void com(PWORD* o1);
	void ror(const PWORD* o1);
	void rol(const PWORD* o1);
	void asr(const PWORD* o1);
	void asl(const PWORD* o1);
	void swab(const PWORD* o1);
	void sxt(PWORD* o1);

	//One-and-a-half-operand instructions
	void mul(RegCode reg, const PWORD* o2);
	void div(RegCode reg, const PWORD* o2);
	void ash(RegCode reg, const PWORD* o2);
	void ashc(RegCode reg, PWORD* o2);
	void xor_(RegCode reg, const PWORD* o2); //actually xor in disguise, xor is a cpp keyword

	//Two-operand instructions
	void mov(const PWORD* o1, PWORD* o2);
	void add(const PWORD* o1, PWORD* o2);
	void sub(const PWORD* o1, PWORD* o2);
	void cmp(const PWORD* o1, const PWORD* o2);
	void bis(const PWORD* o1, PWORD* o2);
	void bic(const PWORD* o1, PWORD* o2);
	void bit(const PWORD* o1, const PWORD* o2);

	//Branch instructions
	void br(const PWORD* ost);
	void bne(const PWORD* ost);
	void beq(const PWORD* ost);
	void bpl(const PWORD* ost);
	void bmi(const PWORD* ost);
	void bvc(const PWORD* ost);
	void bvs(const PWORD* ost);
	void bhis(const PWORD* ost);
	void bcc(const PWORD* ost);
	void blo(const PWORD* ost);
	void bcs(const PWORD* ost);
	void bge(const PWORD* ost);
	void blt(const PWORD* ost);
	void bgt(const PWORD* ost);
	void ble(const PWORD* ost);
	void bhi(const PWORD* ost);
	void blos(const PWORD* ost);

	//Control transfer instructions
	void jmp(const PWORD* ost);
	void sob(RegCode reg, const PWORD* ost);
	void jsr(RegCode reg, const PWORD* ost);
	void rts(RegCode reg);
	void rti();
	void trap(PWORD n);
	void bpt(PWORD n);
	void iot();
	void emt();
	void rtt();

	//Status word instructions
	void spl(const PBYTE* lvl);
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
