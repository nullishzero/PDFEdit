// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$ 
 *
 * $Log$
 * Revision 1.2  2006/05/11 21:05:48  hockm0bm
 * first scratch of CAnnotation class
 *         - still not in kernel.pro
 *
 *
 */

#ifndef __CANNOTATION_H__
#define __CANNOTATION_H__

#include "static.h"
#include "cobject.h"

using namespace pdfobjects;

/** High level object for annotations.
 *
 * This class wrapps pdf annotation dictionary and provides simple interface
 * for manipulation with this dictionary. Also implements logic needed for
 * proper annotation handling.
 * <br>
 * Instance can be created only from existing annotation dictionary or using
 * some of factory methods (each for specific annotation style). If any of
 * required entries is not specified in given dictionary, adds them with default
 * values. This means that empty dictionary is initialized with all required
 * entries with default values.
 * <br>
 * Each CAnnotation instance is valid while it is accessible from some page
 * dictionary. In moment when reference to it is removed from page (more
 * precisely from Annots array in page dictionary), instance is invalidated and
 * so no changes can be done to it anymore. All annotation's properties are
 * available also in invalidated instance.
 * <br>
 * Maintained dictionary is returned by getDictionary method. This can be used
 * to perform unsupported or unusual operations.
 */
class CAnnotation 
{
	/** Private constructor.
	 */
	CAnnotation ();

	/** Annotation dictionary.
	 *
	 * Pdf annotation dictionary maintained by this instance.
	 */
	boost::shared_ptr<CDict> annotDictionary;

	/** Validity flag.
	 */
	bool valid;
public:
	/** Initialization constructor.
	 * @param annotDict Annotation dictionary.
	 * @param page Page dictionary where this annotation is/should be
	 *
	 * Initializes annotDictionary field and set valid flag to true.
	 * Checks whether given dictionary is available from given page dictionary.
	 * If not adds it to the page's Annots array.
	 * <br>
	 * valid field is allways set to true.
	 */
	CAnnotation(boost::shared_ptr<CDict> annotDict, boost::shared_ptr<CDict> page);

	/** Destructor.
	 *
	 * Deallocates all additional data needed for work. Doesn't do anything with
	 * annotDictionary field.
	 */
	~CAnnotation ()
	{
	}

	/** Returns valid flag value.
	 *
	 * If this method returns false, all methods which would produce changes 
	 * fails with exception.
	 *
	 * @return true if instance is valid, false otherwise.
	 */
	bool isValid()const
	{
		return valid;
	}

	
};



#endif // __CANNOTATION_H__

