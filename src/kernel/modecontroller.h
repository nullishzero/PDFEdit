/*
 * =====================================================================================
 *        Filename:  iproperty.h
 *     Description:  IProperty class 
 *         Created:  01/19/2006 11:23:44  CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _MODECONTROLLER_H_
#define _MODECONTROLLER_H_

#include "static.h"

//=====================================================================================

/** 
 * Enum describing property mode. 
 *
 *
 */
enum PropertyMode
{
		mdUnknown, raz,dva,tri
};


/**
 * Interface for mode controller. Others like gui/cui can provide special mode controllers.
 */
class IModeController
{
public:
		/**
		 * Get default mode, when we do not know the mode.
		 *
		 * @return Default mode.
		 */
		virtual PropertyMode getDefaultMode () const = 0;
		
		/**
		 * Get mode of the property if we know it.
		 *
		 * @return Mode of the property. mdUnknown if we do not recognize the property.
		 */
		virtual PropertyMode getMode (const std::string& type, 
									  const std::string& name) const = 0;
protected:
		/** Destructor. */
		virtual ~IModeController () {};
};






/**
 * Default mode controller provided by kernel.
 *
 * It implements singleton idiom. (Meyers singleton)
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
	~ModeController () {printDbg (debug::DBG_INFO, "Mode controller destroyed.");};
};



#endif  //_MODECONTROLLER_H_
