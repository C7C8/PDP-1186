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
