// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.1  2006/06/01 14:07:36  hockm0bm
 * * New test module for utils used by kernel
 *         - tokenizer function tested
 *         - ModeController class tested
 * * libutils is linked with kernel test program
 *
 *
 */
#include "testmain.h"
#include "../utils/confparser.h"
#include "../modecontroller.h"

class TestUtils: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(TestUtils);
		CPPUNIT_TEST(Test);
	CPPUNIT_TEST_SUITE_END();

public:
	virtual ~TestUtils()
	{
	}

	void setUp()
	{
	}


	void tearDown()
	{
	}

	bool tokenizerTC()
	{
	using namespace std;
	using namespace configuration::utils;
		
		OUTPUT << __FUNCTION__<<endl;

		vector<string> testStrings;
		testStrings.push_back(" ");
		testStrings.push_back("simple text with five words");
		testStrings.push_back("deliminer at the end ");
		testStrings.push_back("different:deliminers;in one");
		testStrings.push_back("empty tokens;;inside");
		testStrings.push_back("one word with this deliminers");
			
		vector<string> output;
		OUTPUT << "TC01:\tEmpty string provides no tokens\n";	
		CPPUNIT_ASSERT(tokenizer("", ":,. ", output)==0);
		CPPUNIT_ASSERT(output.size()==0);
		
		OUTPUT << "TC02:\tEmpty deliminers set provides one token\n";
		for(vector<string>::const_iterator i=testStrings.begin(); i!=testStrings.end(); i++)
		{
			CPPUNIT_ASSERT(tokenizer(*i, "", output)==1);
			CPPUNIT_ASSERT(output.size()==1);
			output.clear();
		}

		OUTPUT << "TC03:\tOne empty token\n";
		CPPUNIT_ASSERT(tokenizer(" ", " ", output)==1);
		CPPUNIT_ASSERT(output.size()==1);
		CPPUNIT_ASSERT(output[0]=="");
		output.clear();
		
		OUTPUT << "TC04:\tOne deliminer with multiple words\n";
		CPPUNIT_ASSERT(tokenizer(testStrings[1], " ", output)==5);
		CPPUNIT_ASSERT(output.size()==5);
		CPPUNIT_ASSERT(output[0]=="simple");
		CPPUNIT_ASSERT(output[1]=="text");
		CPPUNIT_ASSERT(output[2]=="with");
		CPPUNIT_ASSERT(output[3]=="five");
		CPPUNIT_ASSERT(output[4]=="words");
		output.clear();

		OUTPUT << "TC05:\tDeliminer at the end\n";
		CPPUNIT_ASSERT(tokenizer(testStrings[2], " ", output)==4);
		CPPUNIT_ASSERT(output.size()==4);
		CPPUNIT_ASSERT(output[0]=="deliminer");
		CPPUNIT_ASSERT(output[1]=="at");
		CPPUNIT_ASSERT(output[2]=="the");
		CPPUNIT_ASSERT(output[3]=="end");
		output.clear();
		
		OUTPUT << "TC06:\tMultiple deliminers\n";
		CPPUNIT_ASSERT(tokenizer(testStrings[3], " :;", output)==4);
		CPPUNIT_ASSERT(output.size()==4);
		CPPUNIT_ASSERT(output[0]=="different");
		CPPUNIT_ASSERT(output[1]=="deliminers");
		CPPUNIT_ASSERT(output[2]=="in");
		CPPUNIT_ASSERT(output[3]=="one");
		output.clear();
		
		OUTPUT << "TC07:\tEmpty tokens inside\n";
		CPPUNIT_ASSERT(tokenizer(testStrings[4], " ;", output)==4);
		CPPUNIT_ASSERT(output.size()==4);
		CPPUNIT_ASSERT(output[0]=="empty");
		CPPUNIT_ASSERT(output[1]=="tokens");
		CPPUNIT_ASSERT(output[2]=="");
		CPPUNIT_ASSERT(output[3]=="inside");
		output.clear();

		OUTPUT << "TC08:\tOne word test (deliminers specified but not present)\n";
		CPPUNIT_ASSERT(tokenizer(testStrings[5], ";:\t\n", output)==1);
		CPPUNIT_ASSERT(output.size()==1);

		return true;
	}

#ifndef BAD_CONF_FILE
#define BAD_CONF_FILE "bad_conf"
#endif

#ifndef EMPTY_CONF_FILE
#define EMPTY_CONF_FILE "empty_conf"
#endif

#ifndef CONF_FILE
#define CONF_FILE "conf"
#endif 
	bool modeControllerTC()
	{
	using namespace configuration;

		OUTPUT << __FUNCTION__ << endl;

		OUTPUT << "TC01:\tEmpty configuration file\n";
		ModeController modeControler;
		StringConfigurationParser baseParser(StringConfigurationParser(NULL));
		ModeController::ConfParser parser(baseParser);
		CPPUNIT_ASSERT(modeControler.loadFromFile(EMPTY_CONF_FILE, parser)==0);
		
		OUTPUT << "TC02:\tBad configuration file\n";
		CPPUNIT_ASSERT(modeControler.loadFromFile(BAD_CONF_FILE, parser)==-1);

		OUTPUT << "TC03:\tCorrect configuration file\n";
		CPPUNIT_ASSERT(modeControler.loadFromFile(CONF_FILE, parser)>0);
		
		// This is specific for conf file - this fits to following configuration
		// .		: mdUnknown
		// Test.	: mdNormal
		// .Type	: mdReadOnly
		// Test.Type: mdHidden
		// 			: mdAdvanced
		OUTPUT << "\t\tMost generic rule check\n";
		CPPUNIT_ASSERT(modeControler.getMode("","")==mdAdvanced);
		CPPUNIT_ASSERT(modeControler.getMode("Foo","")==mdAdvanced);
		CPPUNIT_ASSERT(modeControler.getMode("Foo","Foo")==mdAdvanced);
		CPPUNIT_ASSERT(modeControler.getMode("","Foo")==mdAdvanced);

		OUTPUT << "\t\tType specifi rule check\n";
		CPPUNIT_ASSERT(modeControler.getMode("Test","")==mdNormal);
		CPPUNIT_ASSERT(modeControler.getMode("Test","foo")==mdNormal);

		OUTPUT << "\t\tName specific rule check\n";
		CPPUNIT_ASSERT(modeControler.getMode("","Type")==mdReadOnly);
		CPPUNIT_ASSERT(modeControler.getMode("Foo","Type")==mdReadOnly);

		OUTPUT << "\t\tExact match rule check\n";
		CPPUNIT_ASSERT(modeControler.getMode("Test","Type")==mdHidden);

		return true;
	}

	void Test()
	{
		CPPUNIT_ASSERT(tokenizerTC());
		CPPUNIT_ASSERT(modeControllerTC());
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(TestUtils);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestUtils, "TEST_UTILS");
