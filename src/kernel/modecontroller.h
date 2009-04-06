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

#ifndef _MODECONTROLLER_H_
#define _MODECONTROLLER_H_

#include "kernel/static.h"

//=====================================================================================

/** 
 * Enum describing property mode. 
 * 
 */
enum PropertyMode
{
	/** Unknown mode.
	 */
	mdUnknown, 
	
	/** Normal mode.
	 * Should be visible and read write. Normal property.
	 */
	mdNormal, 
	
	/** Hidden mode.
	 * Should not be visible but read write (when it is shown). Property is not
	 * very important to see (just technical detail) but when it is visible, it
	 * should be changeable.
	 */
	mdHidden, 
	
	/** Read-only mode.
	 * Should not be editable when it is visible. It doesn't make much sense to
	 * change such properties.
	 */
	mdReadOnly, 
	
	/** Advanced mode.
	 * Should not be visible and editable. It is very dangeroous to change such
	 * properties.
	 */
	mdAdvanced
};

namespace configuration
{

/** Rule structure for mode.
 * This type is used for RulesManager RuleType in ModeController.
 *
 * @see RulesManager
 * @see ModeController
 */
struct ModeRule
{
	/** Complex value type.
	 */
	std::string type;

	/** Field name in complex type.
	 */
	std::string name;

	/** Comparator method.
	 * @param rule Rule to compare with.
	 *
	 * Given rule is same as given one if its type and name fields are same as
	 * these.
	 *
	 * @return true if given rule is same, false otherwise.
	 */
	bool operator==(const ModeRule & rule)const
	{
		return type==rule.type && name==rule.name;
	}
};

/** Typedef to IRuleMatcher for ModeMatcher.
 */
typedef rulesmanager::IRuleMatcher<ModeRule> IModeMatcher; 

/** Implementator of IRuleMatcher for ModeRule.
 *
 * See functor operator for more precise information about mode rules
 * matching evaluation.
 */
class ModeMatcher: public IModeMatcher
{
public:
	typedef IModeMatcher::priority_t priority_t;
	
	/** Priority for weakest match.
	 */
	static const priority_t PRIO0=0;

	/** Priority for type only match.
	 */
	static const priority_t PRIO1=1;

	/** Priority for name only match.
	 */
	static const priority_t PRIO2=2;
	
	/** Priority for full match.
	 */
	static const priority_t PRIO3=3;

	/** Determines whether given rule matches given original.
	 * @param original Original rule.
	 * @param rule Rule to check whether matches.
	 * @param priority Place where to store priority of match (only if non NULL
	 * and matches).
	 *
	 * <b>Rules</b>:
	 * <ul>
	 * <li>original={"", ""} - rule allways matches with PRIO0 priority.
	 * <li>original={type, ""} - rule matches if rule.type==original.type with
	 * PRIO1 priority.
	 * <li>original={"", name} - rule matches if rule.name==original.name with
	 * PRIO2 priority.
	 * <li>original={type, name} - rule matches if original==rule with
	 * PRIO3 priority.
	 * <li> PRIO0 &lt; PRIO1 &lt; PRIO2 &lt; PRIO3
	 * </ul>
	 *
	 * @return true if given rule matches original, false otherwise.
	 */
	virtual bool operator()(const ModeRule & original, const ModeRule & rule,  priority_t * priority)const;
};

/** Typedef to RulesManager for ModeController.
 */
typedef rulesmanager::RulesManager<ModeRule, PropertyMode> ModeRulesManager;

/** Decorator for IConfigurationParser with ModeRule and PropertyMode
 * specialization.
 *
 * This class doesn't provide any stream reading. Instead it delegates all
 * reading work to low-level baseParser (provided in constructor) which provides
 * string representation of Rule and mode. Responisbility of this class is to
 * parse those values to correct ModeRule and PropertyMode types.
 * <br>
 * ModeRule is supposed to have format:
 * <pre>
 * Type[.Value] : stringmode
 * </pre>
 * All other IConfigurationParser operations are delegated to the baseParser.
 */
class ModeConfigurationParser: public IConfigurationParser<ModeRule, PropertyMode>
{
public:
	typedef IConfigurationParser<std::string, std::string> StringParser;
private:
	/** Low level string parser.
	 * This parser is initialized in constructor and it is used for input data
	 * parsing.
	 */
	StringParser & baseParser;
public:
	/** Initialization constructor.
	 * Initializes baseParser field.
	 */
	ModeConfigurationParser(StringParser & parser):baseParser(parser)
	{}

	/** Sets new data stream.
	 * @param str Stream to set.
	 *
	 * Delegates to baseStream (sets stream to low level string parser).
	 *
	 * @return Current stream reference.
	 */
	std::istream * setStream(std::istream * str)
	{
		return baseParser.setStream(str);
	}
	
	/** Skips current key, value pair.
	 * Delegates to baseParser.skip()
	 */
	void skip()
	{
		baseParser.skip();
	}

	/** Checks whether we are on end of data.
	 *
	 * Delegates to baseParser.eod()
	 *
	 * @return true if there is nothing more to read, false otherwise.
	 */
	bool eod()
	{
		return baseParser.eod();
	}

	/** Parses key, value pair and transforms them to rule and mode.
	 * @param rule Reference where to put parsed rule.
	 * @param mode Reference where to put parsed mode.
	 *
	 * Delegates to baseParser.parse and transforms returned strings to rule
	 * and mode.
	 * <p>
	 * Rule string has format:
	 * <pre>
	 * type[.name]
	 * </pre>
	 * If `.' is not found name or there are no characters behind, name is
	 * empty. Everything before `.' is Type. It can also be empty. Both parts
	 * are trimed before set (all leading and trailing blanks re skiped).
	 * <br>
	 * Name string should be same as PropertyMode names. If not recognized,
	 * returns with an error.
	 * <br>
	 * Note that !parse() && eod() means no error but nothing more to parse.
	 *
	 * @return true if parsing was successful, false otherwise.
	 */
	bool parse(ModeRule & rule, PropertyMode & mode);
};

/**
 * Basic mode controller implementation. 
 *
 * Provides mapping from type, name pair to Property mode, where type stands for
 * complex type name (Value of complex - dictionary - Type field value) and name
 * for field name in given complex type.
 * <p>
 * <b>Rules</b>
 * <br>
 * We will write type, name pair in following format:
 * <pre>
 * type[.name]: mode
 * </pre>
 * not specified is represented by empty string or word containing just
 * blanks.
 * <ul>
 * <li>not specified type and name. Each type name combination matches. 
 * <pre>
 * Example:
 * .: mdUnknown
 * is same as
 * : mdUnknown
 *
 * Default mode is mdUnknown
 * </pre>
 *
 * <li>type with name unspecified is same as pure type and specifies all fields
 * from dictionary with given Type field value.
 * <pre>
 * Example:
 * Page. : mdAdvanced
 * is same as
 * Page : mdAdvanced
 *
 * All fields from Page complex value is mdAdvanced
 * </pre>
 *
 * <li>unspecified type and specified name stands for all fields with given name
 * with no respect to complex type. It has higher priority than pure type 
 * specification. 
 * <pre>
 * Example:
 * Page. : mdNormal
 * .Value: mdReadOnly
 *
 * Page.foo is mdNormal
 * Page.Value is mdReadOnly
 * Foo.Value is mdReadOnly
 * </pre>
 *
 * <li>Type, Name specifies concrete field on complex type (Type field value of
 * complex) and field name in this complex. This has the highest priority.
 * <pre>
 * Example:
 * Page. : mdNormal
 * .Value: mdReadOnly
 * Page.Value: mdAdvanced
 *
 * Page.Value is mdAdvanced
 * Page.foo is mdNormal
 * foo.Value is mdReadOnly
 * </pre>
 * 
 * <li>Most specific mapping is always used if multiple rules matches.
 * <li>No matching rule stands for default policy (value returned by
 * getDefaultMode).
 * <li>type, name are case sensitive.
 * </ul>
 * 
 * <p>
 * <b>Usage</b><br>
 * Instance with empty constructor is created with mdUnknown default mode. If
 * you need to specify different one, use setDefaultMode method or constructor
 * with parameter. 
 * <br>
 * Rules for modes can be add either manualy by addRule (inherited from 
 * RulesManager class) method or from file using loadFromFile method.
 * loadFromFile method requires file name and parser parameters. Second one is
 * implementator which implements parsing functionality for given configuration
 * file format.
 * <pre>
 * Example:
 *
 * // Initializes mode controller
 * //============================
 * 
 * // creates ModeController instance
 * ModeController modeControler;
 *
 * // Creates low-level parser for simple configuration files (one line one
 * // setting). This base parser is used by ConfParser which is used by
 * // ModeController in loadFromFile 
 * StringConfigurationParser baseParser(StringConfigurationParser(NULL));
 * ModeController::ConfParser parser(baseParser);
 * 
 * // parser is ready, we can load rules 
 * int result=modeControler.loadFromFile(fileName, parser);
 * if(result==-1)
 *   cerr << "File \""<<fileName<<"\" parsing failed"<<endl;
 * else
 *   cout << result << "new rules added"<<endl;
 * 
 * // use mode controller
 * //====================
 * PropertyMode mode=modeControler.getMode(ParentType, ChildName);
 * 
 * </pre>
 */
class ModeController: public ModeRulesManager
{
	/** Default mode.
	 *
	 * Value is set in constructor.
	 */
	PropertyMode defaultMode;

	/** Matcher for rules.
	 *
	 * This matched for supertype matcher intialization in constructor.
	 */
	ModeMatcher matcher;
public:

	/** Type for configuration parser for loadFromFile method.
	 *
	 * @see ModeConfigurationParser
	 */
	typedef ModeConfigurationParser ConfParser;
	
	/** Constructor.
	 *
	 * Intiailizes defaultMode to mdUnknown.
	 * Sets ModeMatcher instance to ModeRulesManager.
	 */
	ModeController():defaultMode(mdUnknown)
	{ 
		// initializes specialized rules matcher for property modes.
		setRuleMatcher(&matcher);
	}

	/** Constructor with default mode.
	 * @param defMod Default mode to be used.
	 *
	 * Initializes defaultMode with given one.
	 * Sets ModeMatcher instance to ModeRulesManager.
	 */
	ModeController(PropertyMode defMod):defaultMode(defMod)
	{
		// initializes specialized rules matcher for property modes.
		setRuleMatcher(&matcher);
	}

	/** Virtual destructor.
	 * This method is empty, because no special treatment is required.
	 */
	virtual ~ModeController () {}

	/** 
	 * Get default mode.
	 * 
	 * Default policy for non matching rules.
	 * @return defaultMode field value.
	 */
	virtual PropertyMode getDefaultMode() const
	{
		return defaultMode;
	}

	/** Sets default mode.
	 * @param mode New default mode.
	 *
	 * @return Previous value of default mode.
	 */
	virtual PropertyMode setDefaultMode(PropertyMode mode)
	{
		PropertyMode old=defaultMode;

		defaultMode=mode;

		return old;
	}

	/**
	 * Get mode for value with given type and name.
	 * @param type Type field value of complex type.
	 * @param name Name if the field in complex type.
	 * 
	 * Given parameters may be empty what means not specified.
	 * <br>
	 * If no rule matches given pair, getDefaultMode is returned.
	 * 
	 * @see RulesManager::findMatching
	 * @return Mode which is most specific for given type, name pair.
	 */
	virtual PropertyMode getMode (const std::string& type, const std::string& name) const
	{
		ModeRule rule={type, name};
		PropertyMode mode;

		// delegates to ModeRulesManager and uses returned mode
		if(findMatching(rule, &mode))
			return mode;

		// didn't match - defaultMode is used
		return defaultMode;
	}
};

} // namespace configuration

#endif  //_MODECONTROLLER_H_
