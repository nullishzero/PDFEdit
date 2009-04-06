/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

// static
#include "kernel/static.h"

#include "kernel/modecontroller.h"


// =====================================================================================
using namespace std;

namespace configuration
{

bool ModeMatcher::operator()(const ModeRule & original, const ModeRule & rule,  priority_t * priority)const
{
	bool type_empty=(original.type=="");
	bool name_empty=(original.name=="");

	// most general if original is empty
	if(type_empty && name_empty)
	{
		if(priority)
			*priority=PRIO0;
		return true;
	}

	// type global rule
	if(!type_empty && name_empty)
	{
		if(original.type!=rule.type)
			return false;

		// name matches
		if(priority)
			*priority=PRIO1;
		return true;
	}

	// name global rule
	if(type_empty && !name_empty)
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
