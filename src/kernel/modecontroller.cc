// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 *
 * $RCSfile$
 *
 * $Log$
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
	if(original.equals(rule))
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
	
	string ruleString;
	string modeString;

	baseParser.parse(ruleString, modeString);

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
				if(modeString=="mdAdvanced")
					mode=mdAdvanced;
				else
					// unknown type of mode
					return false;
	
	// converts ruleString to ModeRule structure
	// type is the first part of string until `.' character and everything
	// behind is name - if deliminer is not found name is empty
	size_t dotPos=ruleString.find_first_of('.');
	if(dotPos!=string::npos)
	{
		// separates
		rule.type.assign(ruleString, 0, dotPos);
		rule.name.assign(ruleString, dotPos, ruleString.length()-dotPos);
	}else
	{
		// no deliminer
		rule.type=ruleString;
		rule.name="";
	}

	// TODO trim strings

	return true;
}
