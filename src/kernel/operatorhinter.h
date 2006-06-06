// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 *
 * $RCSfile$
 *
 * $Log$
 * Revision 1.1  2006/06/06 11:46:52  hockm0bm
 * OperatorHinter rules manager added
 *         - already tested
 *
 *
 */

#ifndef _OPERATOR_HINTER_H_
#define _OPERATOR_HINTER_H_

#include "static.h" 
#include "utils/rulesmanager.h"
#include "utils/confparser.h"

namespace configuration
{

// TODO doc
struct StringMatcher:public rulesmanager::IRuleMatcher<std::string>
{
	typedef rulesmanager::IRuleMatcher<std::string>::priority_t priority_t;
	typedef std::string RuleType;

	static const priority_t LOW_PRIORITY=1;
	static const priority_t EXACT_PRIORITY=2;

	virtual bool operator()(const RuleType & original, const RuleType & rule, priority_t * priority)const
	{
		if(original=="")
		{
			if(priority)
				*priority=LOW_PRIORITY;
			return true;
			
		}
		if(original==rule)
		{
			if(priority)
				*priority=EXACT_PRIORITY;
			return true;
		}

		return false;
	}
};

// TODO doc
class OperatorHinter: public rulesmanager::RulesManager<std::string, std::string>
{	
	std::string defaultHint;
	rulesmanager::IRuleMatcher<std::string> *stringMatcher;
	
public:
	typedef IConfigurationParser<std::string, std::string> ConfParser;

	OperatorHinter():defaultHint(""), stringMatcher(new StringMatcher())
	{
		setRuleMatcher(stringMatcher);
	}	 

	OperatorHinter(const std::string & defHint):defaultHint(defHint), stringMatcher(new StringMatcher())
	{
	};

	virtual ~OperatorHinter()
	{
		delete stringMatcher;
	};

	virtual std::string setDefaultHint(const std::string & defaultHint)
	{
	using namespace std;
		string old=this->defaultHint;

		this->defaultHint=defaultHint;

		return old;
	}
	
	virtual std::string getDefaultHint()const
	{
		return defaultHint;
	}

	virtual std::string & getHint(std::string & operatorName, std::string & hintText)
	{
	using namespace std;
		if(findMatching(operatorName, &hintText))
			return hintText;

		return defaultHint;
	}
};

} // namespace configuration

#endif

