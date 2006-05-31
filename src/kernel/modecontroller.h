// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 *
 * $RCSfile$
 *
 * $Log$
 * Revision 1.6  2006/05/31 07:06:24  hockm0bm
 * doc update and prepared to be implemented
 *         - just for sync
 *
 */

#ifndef _MODECONTROLLER_H_
#define _MODECONTROLLER_H_

#include "static.h"

//=====================================================================================

/** 
 * Enum describing property mode. 
 *
 */
enum PropertyMode
{
	mdUnknown, mdNormal, mdHidden, mdReadOnly, mdAdvanced
};


/**
 * Interface for mode controller. 
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
 * This interface doesn't specify how to set rules, because it is designed for
 * consumers.
 */
class IModeController
{
public:
	/** 
	 * Get default mode.
	 * 
	 * Default policy for non matching rules.
	 * @return Default mode.
	 */
	virtual PropertyMode getDefaultMode() const = 0;

	/** Sets default mode.
	 * @param mode New default mode.
	 *
	 * @return Previous value of default mode.
	 */
	//virtual PropertyMode setDefaultMode(PropertyMode mode)=0;

	/**
	 * Get mode for value with given type and name.
	 * @param type Type field value of complex type.
	 * @param name Name if the field in complex type.
	 * 
	 * Given parameters may be empty what means not specified.
	 * <br>
	 * If no rule matches given pair, getDefaultMode is returned.
	 * 
	 * @return Mode which is most specific for given type, name pair.
	 */
	virtual PropertyMode getMode (const std::string& type, const std::string& name) const = 0;
protected:
	/** Destructor. */
	virtual ~IModeController () {};
};

/**
 * Default mode controller provided by kernel.
 *
 * It implements singleton idiom. (Meyers singleton)
 * 
 */
class ModeController : public IModeController
{
typedef std::map<std::string, PropertyMode> Modes;

private:
	Modes modes;

public:
	/**
	 * Create the object.
	 */
	static ModeController& Instance (const std::string& fileName);

	// Get default mode
	virtual PropertyMode getDefaultMode () const {return mdUnknown;}
	
	// Get mode
	virtual PropertyMode getMode (const std::string& type, 
								  const std::string& name) const;
	
	//
	// Singleton
	//
private:
	
	/**
	 * Parse the file for known properties.
	 *
	 * @param fileName File name.
	 */
	ModeController (const std::string& fileName);
	
private:
	ModeController (const ModeController&);
	ModeController& operator= (const ModeController&);
	~ModeController () {utilsPrintDbg (debug::DBG_INFO, "Mode controller destroyed.");};
};



#endif  //_MODECONTROLLER_H_
