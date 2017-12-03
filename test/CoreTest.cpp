#include "gtest/gtest.h"
#include "../src/Core.h"

TEST(core_test, basic_ops){

	//Instantiation, basic IO
	Core* core = new Core(4096);
	const char* data = "The quick brown fox jumps over the lazy dog.";
	const PWORD dsize = (PWORD)strlen(data);

	core->write(0, (PBYTE*)data, dsize);
	ASSERT_STREQ(data, (char*)core->read(0, dsize));

	//Copy constructor
	Core* core2 = new Core(*core);
	ASSERT_STREQ(data, (char*)core2->read(0, dsize));
	core->write(5, (PBYTE*)data, dsize);
	ASSERT_STREQ(data, (char*)core2->read(0, dsize));
	core->write(0, (PBYTE*)data, dsize);

	//Operator overload, because why not
	Core core3 = *core;
	ASSERT_STREQ(data, (char*)core3.read(0, dsize));
	core->write(5, (PBYTE*)data, dsize);
	ASSERT_STREQ(data, (char*)core3.read(0, dsize));

	delete core;
	delete core2;
}
