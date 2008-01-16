/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#include "testmain.h"
#include "utils/confparser.h"
#include "kernel/modecontroller.h"
#include "kernel/operatorhinter.h"

template<typename T=pdfobjects::IProperty>
class Observer:public observer::IObserver<T>
{
public:
	typedef typename observer::IObserver<T>::priority_t priority_t;

	Observer(priority_t prio):priority(prio){};

	virtual ~Observer()throw(){};

	void notify(boost::shared_ptr<T> , boost::shared_ptr<const observer::IChangeContext<T> > )const throw()
	{
		counter=10*counter+priority;
	}

	priority_t getPriority()const throw()
	{
		return priority;
	}

	static int counter;

private:
	priority_t priority;
};

template<typename T> int Observer<T>::counter=0;

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

#ifndef BAD_MODE_CONF_FILE
#define BAD_MODE_CONF_FILE "tests/mode_bad.conf"
#endif

#ifndef EMPTY_MODE_CONF_FILE
#define EMPTY_MODE_CONF_FILE "tests/mode_empty.conf"
#endif

#ifndef MODE_CONF_FILE
#define MODE_CONF_FILE "tests/mode.conf"
#endif 
	bool modeControllerTC()
	{
	using namespace configuration;

		OUTPUT << __FUNCTION__ << endl;

		OUTPUT << "TC01:\tEmpty configuration file\n";
		ModeController modeControler;
		StringConfigurationParser baseParser(StringConfigurationParser(NULL));
		ModeController::ConfParser parser(baseParser);
		CPPUNIT_ASSERT(modeControler.loadFromFile(EMPTY_MODE_CONF_FILE, parser)==0);
		
		OUTPUT << "TC02:\tBad configuration file\n";
		CPPUNIT_ASSERT(modeControler.loadFromFile(BAD_MODE_CONF_FILE, parser)==-1);

		OUTPUT << "TC03:\tCorrect configuration file\n";
		CPPUNIT_ASSERT(modeControler.loadFromFile(MODE_CONF_FILE, parser)>0);
		
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
#ifndef HINTER_CONF_FILE
#define HINTER_CONF_FILE "tests/operator.conf"
#endif

	bool operatorHinterTC()
	{
	using namespace configuration;
	using namespace std;

		OperatorHinter opHinter;
		StringConfigurationParser baseParser(StringConfigurationParser(NULL));
		StringConfigurationParser parser(baseParser);
		CPPUNIT_ASSERT(opHinter.loadFromFile(HINTER_CONF_FILE, parser)>0);

		// This is specific for conf file - this fits to following configuration
		// :Hint for this is not available
		// Tj:Text visualization operator 
		// g  : Any idea what it is
		// q : # empty text
		string hintText;
		string defaultHint="Hint for this is not available";
		string tjHint="Text visualization operator";
		string gHint="Any idea what it is";
		string op;
		
		op="";
		CPPUNIT_ASSERT(opHinter.getHint(op, hintText)==defaultHint);

		op="Nonsense";
		CPPUNIT_ASSERT(opHinter.getHint(op, hintText)==defaultHint);

		op="Tj";
		CPPUNIT_ASSERT(opHinter.getHint(op, hintText)==tjHint);

		op="g";
		CPPUNIT_ASSERT(opHinter.getHint(op, hintText)==gHint);
		
		op="q";
		CPPUNIT_ASSERT(opHinter.getHint(op, hintText)=="");

		return true;
	}

	bool observerHandlerTC()
	{
	using namespace observer;
	using namespace std;
	using namespace boost;

		OUTPUT << __FUNCTION__ << endl;
		ObserverHandler<int> observerHandler;
		shared_ptr<IObserver<int> > observer1(new Observer<int>(1));
		shared_ptr<IObserver<int> > observer2(new Observer<int>(2));
		shared_ptr<IObserver<int> > observer3(new Observer<int>(3));
		shared_ptr<IObserver<int> > observer4(new Observer<int>(4));
		shared_ptr<IObserver<int> > observer5(new Observer<int>(5));
		shared_ptr<IObserver<int> > observer6(new Observer<int>(6));
		shared_ptr<IObserver<int> > observer7(new Observer<int>(7));
		shared_ptr<IObserver<int> > observer8(new Observer<int>(8));

		observerHandler.registerObserver(observer8);
		observerHandler.registerObserver(observer7);
		observerHandler.registerObserver(observer6);
		observerHandler.registerObserver(observer5);
		observerHandler.registerObserver(observer4);
		observerHandler.registerObserver(observer3);
		observerHandler.registerObserver(observer2);
		observerHandler.registerObserver(observer1);

		shared_ptr<int> newValue(new int(3));
		shared_ptr<IChangeContext<int> > context;

		observerHandler.notifyObservers(newValue, context);
		OUTPUT << "TC01:\tnotifyObservers - correct order of notification test" << endl;
		CPPUNIT_ASSERT(Observer<int>::counter==12345678);

		OUTPUT << "TC02:\tregisterObserver & notifyObservers - correct position of new observer test" << endl;
		shared_ptr<IObserver<int> > observer9(new Observer<int>(9));
		observerHandler.registerObserver(observer9);
		Observer<int>::counter=0;
		observerHandler.notifyObservers(newValue, context);
		CPPUNIT_ASSERT(Observer<int>::counter==123456789);
		
		OUTPUT << "TC03:\tunregisterObserver doesn't brake correct order of other observer" << endl;
		// remove from from inside
		observerHandler.unregisterObserver(observer2);
		Observer<int>::counter=0;
		observerHandler.notifyObservers(newValue, context);
		CPPUNIT_ASSERT(Observer<int>::counter==13456789);
		// remove from from head
		observerHandler.unregisterObserver(observer1);
		Observer<int>::counter=0;
		observerHandler.notifyObservers(newValue, context);
		CPPUNIT_ASSERT(Observer<int>::counter==3456789);
		// remove from from tail
		observerHandler.unregisterObserver(observer9);
		Observer<int>::counter=0;
		observerHandler.notifyObservers(newValue, context);
		CPPUNIT_ASSERT(Observer<int>::counter==345678);

		OUTPUT << "TC4:\tunregisterObserver fails if observer is not registered."<<endl;
		try
		{
			observerHandler.unregisterObserver(observer1);
			CPPUNIT_FAIL("unregisterObserver should have failed");
		}catch(ObserverException & e)
		{
			// ok
		}

		return true;
	}

	void Test()
	{
		CPPUNIT_ASSERT(tokenizerTC());
		CPPUNIT_ASSERT(modeControllerTC());
		CPPUNIT_ASSERT(operatorHinterTC());
		CPPUNIT_ASSERT(observerHandlerTC());
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(TestUtils);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestUtils, "TEST_UTILS");
