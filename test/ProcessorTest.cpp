#include "gtest/gtest.h"
#include "../src/Processor.h"

/**
 * Basic register assignment, copy, and priority tests
 */
TEST(processor_test, basic_ops){
	//Basic registers
	Processor proc;
	proc.reg(R0, 0xF00F);
	proc.reg(R1, 0xC7C8);
	ASSERT_EQ(0xF00F, proc.reg(R0));
	ASSERT_EQ(0xC7C8, proc.reg(R1));
	for (int i = 2; i < REGCOUNT; i++)
		ASSERT_EQ(0, proc.reg((RegCode) i));

	//Copy & set
	Processor proc1(proc), proc2 = proc;
	proc.reg(R0, 0);
	proc.reg(R1, 0);

	ASSERT_EQ(0xF00F, proc1.reg(R0));
	ASSERT_EQ(0xC7C8, proc1.reg(R1));
	for (int i = 2; i < REGCOUNT; i++)
		ASSERT_EQ(0, proc1.reg((RegCode) i));

	ASSERT_EQ(0xF00F, proc2.reg(R0));
	ASSERT_EQ(0xC7C8, proc2.reg(R1));
	for (int i = 2; i < REGCOUNT; i++)
		ASSERT_EQ(0, proc2.reg((RegCode) i));

	//Priority bit setting
	for (PWORD i = 1; i < 8; i++) {
		ASSERT_EQ(0, proc.priority());
		proc.priority(i);
		for (int j = 0; j < 5; j++)
			ASSERT_EQ(0, proc.pstat() & (1 << j));
		ASSERT_EQ(i, proc.priority());
		proc.priority(0);
	}
}

TEST(processor_test, one_arg_instructions){
	Processor proc;
	PWORD o1 = 1;

	//clr: zero a number
	proc.clr(&o1);
	ASSERT_EQ(o1, 0);
	ASSERT_FALSE(proc.pstat_neg() || proc.pstat_overf() || proc.pstat_carry());
	ASSERT_TRUE(proc.pstat_zero());
	//o1=0

	//inc: increment number by one
	proc.ccc();
	proc.inc(&o1);
	ASSERT_EQ(o1, 1);
	ASSERT_FALSE(proc.pstat_neg() || proc.pstat_carry() || proc.pstat_zero() || proc.pstat_overf());
	//o1=1

	//inc: decrement number by one
	proc.ccc();
	proc.dec(&o1);
	ASSERT_EQ(o1, 0);
	ASSERT_FALSE(proc.pstat_neg() || proc.pstat_carry() || proc.pstat_overf());
	ASSERT_TRUE(proc.pstat_zero());
	//o1=0

	//adc: add 1 if carry flag set
	proc.ccc();
	proc.adc(&o1);
	ASSERT_EQ(o1, 0);
	ASSERT_TRUE(proc.pstat_zero());
	proc.sec();
	proc.adc(&o1);
	ASSERT_EQ(o1, 1);
	ASSERT_FALSE(proc.pstat_zero());
	//o1=1

	//sbc: subtract 1 if carry flag set
	proc.ccc();
	proc.sbc(&o1);
	ASSERT_EQ(o1, 1);
	ASSERT_FALSE(proc.pstat_zero());
	proc.sec();
	proc.sbc(&o1);
	ASSERT_EQ(o1, 0);
	ASSERT_TRUE(proc.pstat_zero());
	//o1=0

	//tst: set condition codes by value
	proc.ccc();
	proc.tst(&o1);
	ASSERT_FALSE(proc.pstat_carry() || proc.pstat_overf() || proc.pstat_neg());
	ASSERT_TRUE(proc.pstat_zero());
	o1 = NEG_BIT;
	proc.tst(&o1);
	ASSERT_TRUE(proc.pstat_neg());
	ASSERT_FALSE(proc.pstat_zero());
	//o1=NEG_BIT

	//neg: invert value
	o1 = 1;
	proc.ccc();
	proc.neg(&o1);
	ASSERT_EQ(o1, 0xFFFF);
	ASSERT_TRUE(proc.pstat_neg() && proc.pstat_carry() && proc.pstat_overf());
	ASSERT_FALSE(proc.pstat_zero());
	//o1=0xFFFF

	//com: one's complement of value
	proc.ccc();
	proc.com(&o1);
	ASSERT_EQ(o1, 0);
	ASSERT_TRUE(proc.pstat_zero() && proc.pstat_carry());
	ASSERT_FALSE(proc.pstat_neg() || proc.pstat_overf());
	//o1=0

	//ror: rotate value right
	proc.ccc();
	proc.ror(&o1);
	ASSERT_EQ(o1, 0);
	o1 = 0b10101010;
	proc.ror(&o1);
	ASSERT_EQ(o1, 0b01010101);
	ASSERT_FALSE(proc.pstat_zero() || proc.pstat_neg());
	ASSERT_TRUE(proc.pstat_carry() && proc.pstat_overf()); //???
	proc.ror(&o1);
	ASSERT_EQ(o1, 0b1000000000101010);

	//rol: rotate value left
	o1 = 0b01010101;
	proc.ccc();
	proc.rol(&o1);
	ASSERT_EQ(o1, 0b10101010);
	ASSERT_FALSE(proc.pstat_zero() || proc.pstat_neg());
	ASSERT_TRUE(proc.pstat_carry() && proc.pstat_overf()); //???
	o1 = 0b1000000000101010;
	proc.rol(&o1);
	ASSERT_EQ(o1, 0b01010101);

	//asr: arithmetic right shift
	o1 = 0xC000;
	proc.ccc();
	proc.asr(&o1);
	ASSERT_EQ(o1, 0xE000);
	ASSERT_TRUE(proc.pstat_neg() && proc.pstat_overf());
	ASSERT_FALSE(proc.pstat_carry() || proc.pstat_zero());

	//asl: left shift
	o1 = 1;
	proc.ccc();
	proc.asl(&o1);
	ASSERT_EQ(o1, 2);
	ASSERT_FALSE(proc.pstat_carry() || proc.pstat_zero() || proc.pstat_neg() || proc.pstat_overf());

	//swab: swap bytes in a word
	o1 = 0xFF00;
	proc.ccc();
	proc.swab(&o1);
	ASSERT_EQ(o1, 0x00FF);
	ASSERT_FALSE(proc.pstat_carry() || proc.pstat_overf() || proc.pstat_zero());
	ASSERT_TRUE(proc.pstat_neg());
	proc.swab(&o1);
	ASSERT_EQ(o1, 0xFF00);
	ASSERT_FALSE(proc.pstat_carry() || proc.pstat_overf() || proc.pstat_neg());
	ASSERT_TRUE(proc.pstat_zero());
	//o1=0xFF00

	//sign extend: sets all bits to be the same as the code N.
	proc.tst(&o1);
	proc.sxt(&o1);
	ASSERT_EQ(o1, 0xFFFF);
	ASSERT_FALSE(proc.pstat_zero());
	o1 = 1;
	proc.tst(&o1);
	proc.sxt(&o1);
	ASSERT_EQ(o1, 0);
	ASSERT_TRUE(proc.pstat_zero());
}

TEST(processor_test, one_half_arg_instructions){
	Processor proc;
	PWORD o1 = 5;
	proc.reg(R0, o1);

	//mul
	ASSERT_EQ(proc.reg(R0), 5);
	proc.mul(R0, &o1);
	ASSERT_EQ(proc.reg(R0), 25);
	proc.mul(R0, &o1);
	ASSERT_EQ(proc.reg(R0), 125);

	//div
	proc.div(R0, &o1);
	ASSERT_EQ(proc.reg(R0), 25);
	proc.div(R0, &o1);
	ASSERT_EQ(proc.reg(R0), 5);
	proc.div(R0, &o1);
	ASSERT_EQ(proc.reg(R0), 1);

	//ash
	o1 = 15;
	proc.reg(R0, 1);
	proc.ash(R0, &o1);
	ASSERT_EQ(proc.reg(R0), 1 << 15);
	o1 = 0b110001; //two's complement for -15
	proc.ash(R0, &o1);
	ASSERT_EQ(proc.reg(R0), 1);

	//xor_
	o1 = 0xAAAA;
	proc.reg(R0, 0x5555);
	proc.xor_(R0, &o1);
	ASSERT_EQ(proc.reg(R0), 0xFFFF);
	proc.reg(R0, 0xAAAA);
	proc.xor_(R0, &o1);
	ASSERT_EQ(proc.reg(R0), 0);
}

TEST(processor_test, two_arg_instructions){
	PWORD o1 = 1, o2 = 5;
	Processor proc;

	//mov: move data
	proc.mov(&o2, &o1);
	ASSERT_EQ(o2, o1);
	//o1=o2=5

	//add
	proc.add(&o1, &o2);
	ASSERT_EQ(o2, o1 * 2);
	o1 = 5;
	o2 = 10;

	//sub
	proc.sub(&o1, &o2);
	ASSERT_EQ(o2, o1);
	proc.sub(&o1, &o2);
	proc.sub(&o1, &o2); // force negative
	ASSERT_TRUE(proc.pstat_neg());
	o1 = 5;
	o2 = 5;

	//cmp: compare by performing src - dest, but discard the result
	proc.cmp(&o1, &o2);
	ASSERT_FALSE(proc.pstat_neg());
	ASSERT_FALSE(proc.pstat_overf());
	ASSERT_FALSE(proc.pstat_carry());
	ASSERT_TRUE(proc.pstat_zero());
	o1 = 10;
	proc.cmp(&o1, &o2);
	ASSERT_FALSE(proc.pstat_neg());
	ASSERT_FALSE(proc.pstat_zero());
	o2 = 15;
	proc.cmp(&o1, &o2);
	ASSERT_TRUE(proc.pstat_neg());
	ASSERT_FALSE(proc.pstat_zero());

	//bis: bitwise OR
	o1 = 0xFF00;
	o2 = 0x00FF;
	proc.bis(&o1, &o2);
	ASSERT_EQ(o2, 0xFFFF);
	o2 = 0xFF00;
	proc.bis(&o1, &o2);
	ASSERT_EQ(o2, 0xFF00);

	//bic: bitwise AND
	o1 = 0xFF00;
	o2 = 0xFF00;
	proc.bic(&o1, &o2);
	ASSERT_EQ(o2, 0xFF00);
	ASSERT_FALSE(proc.pstat_zero()); //why not
	o2 = 0x00FF;
	proc.bic(&o1, &o2);
	ASSERT_EQ(o2, 0);
	ASSERT_TRUE(proc.pstat_zero());

	//bit: bitwise test, compute bitwise AND but only set flags. Basically the same test series as before.
	o1 = 0xFF00;
	o2 = 0xFF00;
	proc.bit(&o1, &o2);
	ASSERT_FALSE(proc.pstat_zero()); //why not
	o2 = 0x00FF;
	proc.bit(&o1, &o2);
	ASSERT_TRUE(proc.pstat_zero());
	ASSERT_FALSE(proc.pstat_neg());
}