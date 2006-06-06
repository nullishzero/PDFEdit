// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 *
 * $RCSfile$
 *
 * $Log$
 * Revision 1.13  2006/06/06 11:46:03  hockm0bm
 * Refactoring changes
 *
 * * ModeMatcher
 *         - uses == instead of equals (sync with RulesManager change)
 * * ModeRule
 *         - operator == replaces equals method (implementation same)
 *
 * Revision 1.12  2006/06/06 10:15:12  hockm0bm
 * loadFromFile method removed
 *         - uses default implementation from RulesManager
 *
 * Revision 1.11  2006/06/06 09:19:45  hockm0bm
 * * ModeController moved to configuration namespace
 * * ModeController::loadFromFile
 *         - fileName is const string & now
 *
 * Revision 1.10  2006/06/01 18:49:06  hockm0bm
 * typo fix
 *
 * Revision 1.9  2006/06/01 14:12:47  hockm0bm
 * * ModeConfigurationParser
 *         - parse method corrected: mdReadOnly was missing, handling of
 *           baseParser.parse return value, trims modeString and output type
 *           and name, uses tokenizer function for ruleString parsing
 *         - loadFromFile implemented
 *         - documentation update
 * * ModeController
 *         - doc update
 *
 * Revision 1.8  2006/05/31 22:36:58  hockm0bm
 * ModeConfigurationParser
 *         - setStream method implemented
 *         - first scratch of loadFromFile
 *
 * Revision 1.7  2006/05/31 21:43:54  hockm0bm
 * gcc
 *
 * Revision 1.6  2006/05/31 20:10:22  hockm0bm
 * * PropertyMode enhanced and documented
 * * ModeRule, ModeMatcher, ModeConfigurationParser classes added
 *         - used by Modecontroller
 * * New Modecontroller implementation
 *         - IModeController removed
 *         - enherits from ModeRulesManager
 *         - setDefaultMode method added
 *         - loadFromFile added - not implemented yet (parser ready)
 *
 * Revision 1.5  2006/05/31 07:06:24  hockm0bm
 * doc update and prepared to be implemented
 *         - just for sync
 *
 */

// static
#include "static.h"

#include "modecontroller.h"


// =====================================================================================
using namespace std;

namespace configuration
{

bool ModeMatcher::operator()(const ModeRule & original, const ModeRule & rule,  priority_t * priority)const
{
	// most general if original is empty
	if(original.type=="" && original.name=="")
	{
		if(priority)
			*priority=PRIO0;
		return true;
	}

	// type global rule
	if(original.type!="" && original.name=="")
	{
		if(original.type!=rule.type)
			return false;

		// name matches
		if(priority)
			*priority=PRIO1;
		return true;
	}

	// name global rule
	if(original.type=="" && original.name!="")
	{
		if(original.name!=rule.name)
			return false;

		if(priority)
			*priority=PRIO2;
		return true;
	}

	// type, name specific rule
	if(original==rule)
	{
		if(priority)
			*priority=PRIO3;
		return true;
	}
	
	// doesn't match
	return false;
}


bool ModeConfigurationParser::parse(ModeRule & rule, PropertyMode & mode)
{
using namespace std;
using namespace configuration::utils;
	
	string ruleString;
	string modeString;

	if(!baseParser.parse(ruleString, modeString))
		return false;

	// get rid of mess around mode string
	trim(modeString);
	
	// sets mode - string must be same as enum name
	if(modeString=="mdUnknown")
		mode=mdUnknown;
	else
		if(modeString=="mdNormal")
			mode=mdNormal;
		else
			if(modeString=="mdHidden")
				mode=mdHidden;
			else
				if(modeString=="mdReadOnly")
					mode=mdReadOnly;
				else
					if(modeString=="mdAdvanced")
						mode=mdAdvanced;
					else
						// unknown type of mode
						return false;
	
	// converts ruleString to ModeRule structure - uses tokenizer with `.'
	// separator
	vector<string> tokens;
	if(tokenizer(ruleString, ".", tokens)>2)
	{
		// TODO warning that line is bad
		return false;
	}

	// initializes to empty state and set only if parsed
	rule.type="";
	rule.name="";
	if(tokens.size()>0)
	{
		rule.type=trim(tokens[0]);
		if(tokens.size()>1)
			rule.name=trim(tokens[1]);
	}

	return true;
}

} // namespace configuration
