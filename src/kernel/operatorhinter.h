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

#ifndef _OPERATOR_HINTER_H_
#define _OPERATOR_HINTER_H_

#include "kernel/static.h" 
#include "utils/rulesmanager.h"
#include "utils/confparser.h"

namespace configuration
{

/** Matcher implementation for string tule type.
 *
 * Simple matcher two matching rules:
 * <ul>
 * <li>empty original string matches everything with the lowest priority
 * <li>non empty string matches only same string (case sensitive)
 * </ul>
 */
struct StringMatcher:public rulesmanager::IRuleMatcher<std::string>
{
	/** Priority type. */
	typedef rulesmanager::IRuleMatcher<std::string>::priority_t priority_t;
	
	/** Alias for string RuleType. */
	typedef std::string RuleType;

	/** Any match priority.
	 * Matching to empty original has this priority.
	 */
	static const priority_t LOW_PRIORITY=1;

	/** Exact match priority.
	 */
	static const priority_t EXACT_PRIORITY=2;

	/** Comparing functor.
	 * @param original Original string to compare with.
	 * @param rule Rule string to compare.
	 * @param priority Place where to store matching priority (if non NULL).
	 *
	 * If given original is empty string, then everything matches (with
	 * LOW_PRIORITY priority), otherwis compares original with given rule (if
	 * matches them priority is EXACT_PRIORITY).
	 * <br>
	 * If rule matches and priority is non NULL, stores priority of match to the
	 * place where given priority parameter points.
	 *
	 * @return true if rule matches given original, false otherwise.
	 */
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

/** RulesManager specialized for operation to hints mapping.
 *
 * Keeps string rules which are associated with string hints. Rules stands for
 * operators names (from content stream) and hints are help texts to understand
 * what is operator goot for or how it should be used.
 * <br>
 * Uses StringMatcher internally to check rules.
 * <br>
 * Configuration can be loaded from file by loadFromFile method.
 *
 * @see RulesManager::loadFromFile
 */
class OperatorHinter: public rulesmanager::RulesManager<std::string, std::string>
{	
	/** Default hint string.
	 * This string is used when no rule matches given one. It can be set either
	 * in constructor or by setDefaultHint method.
	 */
	std::string defaultHint;

	/** Matcher implementation for RulesManager.
	 */
	rulesmanager::IRuleMatcher<std::string> *stringMatcher;
	
public:
	/** Type for Configuration parser.
	 * Use this parser type in loadFromFile method.
	 */
	typedef IConfigurationParser<std::string, std::string> ConfParser;

	/** Default constructor.
	 * Initializes defaultHint to empty string and allocates StringMatcher which
	 * is used to set matcher for RulesManager (uses setRuleMatcher method).
	 */
	OperatorHinter():defaultHint(""), stringMatcher(new StringMatcher())
	{
		setRuleMatcher(stringMatcher);
	}	 

	/** Initialization constructor.
	 * Initializes defaultHint to given string and allocates StringMatcher which
	 * is used to set matcher for RulesManager (uses setRuleMatcher method).
	 */
	OperatorHinter(const std::string & defHint):defaultHint(defHint), stringMatcher(new StringMatcher())
	{
		setRuleMatcher(stringMatcher);
	};

	/** Virtual destructor.
	 * Deallocates stringMatcher created in constructor.
	 */
	virtual ~OperatorHinter()
	{
		delete stringMatcher;
	};

	/** Sets defaultHint field.
	 * @param defaultHint String to use for defaultHint field.
	 *
	 * @return previous value of defaultHint field.
	 */
	virtual std::string setDefaultHint(const std::string & defaultHint)
	{
		std::string old=this->defaultHint;

		this->defaultHint=defaultHint;

		return old;
	}
	
	/** Returns defaultHint field value.
	 *
	 * @return defaultHint string.
	 */
	virtual std::string getDefaultHint()const
	{
		return defaultHint;
	}

	/** Gets hint for given operator name.
	 * @param operatorName Name of the operator to search hint for.
	 * @param hintText String where to put hint for given operator.
	 *
	 * Uses findMatching with given operatorName and if finds matches, sets
	 * hintText string value and returns reference to it.
	 *
	 * @return reference to given hintText.
	 */
	virtual std::string & getHint(std::string & operatorName, std::string & hintText)
	{
		if(findMatching(operatorName, &hintText))
			return hintText;

		return defaultHint;
	}
};

} // namespace configuration

#endif

