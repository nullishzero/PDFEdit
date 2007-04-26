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

/*
 * $RCSfile$ 
 *
 * $Log$
 * Revision 1.14  2007/04/26 09:31:52  bilboq
 *
 * gcc-4.3 compilation fixes
 *
 * Revision 1.13  2007/02/04 20:17:01  mstsxfx
 * Common Licence comment for all cc and h files available in doc/licence_header
 * file and its content to all cc and h files in src/{gui,kernel,utils}
 * directories.
 * Xpdf code, QSA and QOutputDevice code are not touched
 *
 * Revision 1.12  2006/08/29 17:21:25  petrm1am
 *
 * const -> neconst in annota<tion initializers
 *
 * Revision 1.11  2006/08/29 16:28:00  hockm0bm
 * LinkAnnotInitializer imlpemented
 *
 * Revision 1.10  2006/08/27 21:34:12  hockm0bm
 * typo
 *
 * Revision 1.9  2006/06/29 20:00:07  hockm0bm
 * doc updated
 *
 * Revision 1.8  2006/06/18 12:04:42  hockm0bm
 * obsevers code clean up and consolidation
 *
 * Revision 1.7  2006/06/17 18:34:53  hockm0bm
 * Refactoring changes
 *
 * * IObserverHandler
 *         -renamed to ObserverHandler - it is not interface
 *         - BasicObserverContext, CDictComplexObserverContext,
 *           CArrayComplexObserverContext removed - it doesn't have to know
 *           anything about existing context types
 * * CObjectSimple, CDict, CArray, CStream, CPage, CAnnotation, CContentStream
 *         - each provides typedef with produced change context
 *
 * Revision 1.6  2006/06/03 20:08:18  hockm0bm
 * == operator added
 *
 * Revision 1.5  2006/06/02 16:54:06  hockm0bm
 * * checkAndReplace removed (and placed to cobjecthelpers.h)
 * * CAnnotation constructor with page, rect, annotType parameters removed
 * * createAnnotation static added
 * * getDictionary method added
 *
 * Revision 1.4  2006/05/30 21:00:25  hockm0bm
 * * annotTypeMapping utils method added
 * * default static values for TextAnnotInitializer
 * * TextAnnotInitializer functor
 *         - almost complete:
 *           Just some not very offten used fields are not done (but they
 *           have default value according pdf specification).
 *           It is not very important to add them
 * * CAnnotation initialization constructor
 *         - signature changed - uses CPage instead of page dictionary,
 *           annotDict parameter removed (one is created in constructor),
 *           rect parameter added (rectangle for annotation),
 *           adding to the page Annots array is done by CPage interface
 *         - initializes Type, Rect, P, M annotation dictionary fields
 * * CAnnotation class extends IObserverHandler
 *         - because of invalidation
 * * CAnnotation::invalidate method added
 *         - notifies all observers and sets valid flag to false
 * * CAnnotation::isValid method added
 * * CAnnotation::AnnotType enmumeration for known types
 * * CAnnotation::getType method added
 *
 * Revision 1.3  2006/05/23 19:12:03  hockm0bm
 * * IAnnotInitializator added
 * * UniversalAnnotInitializer implemented
 * * TextAnnotInitializer
 *         - skeleton implementation
 * * CAnnotation
 *         - static initializer
 *         - constructor with initializer
 *         - constructor with initialized dictionary
 *
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

namespace pdfobjects
{

namespace utils
{

/** Annotation initializator interface.
 *
 * Provides interface for initializators of annotation dictionaries.
 * Implementator should return all supported annotation types by
 * getSupportedList method. Function operator initializes given annotation
 * dictionary.
 * <br>
 * This is implementation of stateless builder design pattern (stateless means
 * that result is constructed in one step and returned directly from construct
 * function operator).
 */
class IAnnotInitializator
{
public:
	/** Type for supported initializer names.
	 */
	typedef std::vector<std::string> SupportedList;

	virtual ~IAnnotInitializator(){}
	
	/** Returns list of supported annotation types.
	 *
	 * Returned container contains all supprted annotation types, which can be
	 * used in function operator to make some initialization.
	 * 
	 * @return SupportedList container with all supprted types.
	 */
	virtual SupportedList getSupportedList() const =0;

	/** Initialization (builder) function.
	 * @param annotDict Annotation dictionary to initialize.
	 * @param annotType Name of the annotation type to initialize.
	 *
	 * Initializes given dictionary to given annotType annotation type. If
	 * doesn't support such type, doesn't do any special initialization and
	 * returns with false.
	 * <br>
	 * In any case checks whether given annotDict contains Type field with /Annot
	 * value and if not sets this value.
	 * <br>
	 * Implemntator may do some checking and fail if given annotType can't be
	 * initialized (for example if it is initialized as different type which is
	 * in conflict with desired one).
	 
	 * @return if given annotType special initialization was done for annotDict
	 */
	virtual bool operator()(boost::shared_ptr<CDict> & annotDict, std::string annotType)=0;
};

/** Composite for annotation intializers.
 *
 * Collects implementators of IAnnotInitializator interfaces. Each implementator
 * is associated with annotation type(s) which is supported. Function operator
 * delegates work to registered implementator or just adds Type field with Annot
 * value.
 */
class UniversalAnnotInitializer: public IAnnotInitializator
{
public:
	/** Type for implementators mapping.
	 * Maps annotation intializator name to implementation.
	 */
	typedef std::map<std::string, boost::shared_ptr<IAnnotInitializator> > AnnotList;
private:
	/** Registered initializators.
	 */
	AnnotList implList;
public:
	/** Empty constructor.
	 *
	 * Doesn't do any special initialization. implList is empty and so no
	 * initializators are supported by default.
	 */
	UniversalAnnotInitializer(){}
	
	/** Initialization constructor from existing mapping.
	 * @param impls Existing mapping.
	 *
	 * Use given mapping for implList initialization.
	 */
	UniversalAnnotInitializer(AnnotList impls):implList(impls){}

	virtual ~UniversalAnnotInitializer(){}

	/** Returns list of supported initializators.
	 * 
	 * @return list of all names from implList.
	 */
	SupportedList getSupportedList()const;
		
	/** Registers new implementator for given annotation type.
	 * @param annotType Type of the annotation.
	 * @param impl Implementator for initializator.
	 * @param forceNew Flag controling overwriting.
	 *
	 * Checks whether given type is already registered (mapping for this key is
	 * in implList) and if it is checks forceNew flag. If it is true, replaces
	 * current implementator (in implList) with given one, otherwise doesn't do
	 * anything. If it is not registered, registers new with annotType as key
	 * and impl pair to implList.
	 *
	 * @return true if implementation was changed/added, false otherwise.
	 */
	bool registerInitializer(std::string annotType, boost::shared_ptr<IAnnotInitializator> impl, bool forceNew=false);

	/** Initialization function.
	 * @param annotDict Annotation dictionary to initialize.
	 * @param annotType Name of the annotation type to initialize.
	 *
	 * Sets Type field value to Annot, if it as different or missing value and
	 * searches initializator for given annotType. If found, delegates to the
	 * implementation and returns same as implementator functor. Otherwise
	 * returns false.
	 * @return if given annotType special initialization was done for annotDict.
	 */
	virtual bool operator()(boost::shared_ptr<CDict> & annotDict, std::string annotType);

};

/** Initializator for Text annotation.
 *
 * Initializes text annotation. Default values for text specific annotation
 * dictionary fields are stored in public static class fields and so can be
 * changed be class user.
 */
class TextAnnotInitializer: public IAnnotInitializator
{
public:
	/** Default value for Open entry in Annotation dictionary.
	 * Value is true by default.
	 */
	static bool OPEN;

	/** Default value for Contents entry in Annotation dictionary.
	 * Value is empty string by default.
	 */
	static std::string CONTENTS;

	/** Default value for Name entry in Annotation dictionary.
	 * Value is Comment by default.
	 */
	static std::string NAME;

	/** Default value for State entry in Annotation dictionary.
	 * Value is Unmarked by default.
	 */
	static std::string STATE;

	/** Default value for StateModel entry in Annotation dictionary.
	 * Value is Marked by default.
	 */
	static std::string STATEMODEL;

	/** Default value for F entry in Annotation dictionary.
	 */
	static int FLAGS;
	
	/** Returns supported type.
	 *
	 * @return list with one element with Text type.
	 */
	IAnnotInitializator::SupportedList getSupportedList()const;

	/** Initializes text annotation.
	 * @param annotDict Annotation dictionary to initialize.
	 * @param annotType Name of the annotation type to initialize (must be Text).
	 *
	 * Checks if given annotType is Text and if yes, initializes given
	 * dictionary as Text annotation. Otherwise immediately returns with false.
	 * <br>
	 * Assumes that following entries are initialized: Type, P, Rect, M.
	 * <br>
	 * Initialization doesn't do any checking and so annotation dictioanries
	 * initialized to different type shouldn't be used here. Result may be
	 * incorrect annotation dictionary in such situation.
	 *
	 * @return true if initialization is successfull, false otherwise.
	 */
	virtual bool operator()(boost::shared_ptr<CDict> & annotDict, std::string annotType);
};

/** Initializator for Link annotation.
 *
 * Initializes link annotation. Default values for link specific annotation
 * dictionary fields are stored in public static class fields and so can be
 * changed be class user.
 */
class LinkAnnotInitializer: public IAnnotInitializator
{
public:
	/** Default value for Contents entry in Annotation dictionary.
	 * Value is empty string by default.
	 */
	static std::string CONTENTS;

	/** Default value for Dest entry in Annotation dictionary.
	 */
	static std::string DEST;

	/** Default value for H (highlight mode) entry in Annotation dictionary.
	 */
	static std::string H;

	/** Returns supported type.
	 *
	 * @return list with one element with Link type.
	 */
	IAnnotInitializator::SupportedList getSupportedList()const;

	/** Initializes Link annotation.
	 * @param annotDict Annotation dictionary to initialize.
	 * @param annotType Name of the annotation type to initialize (must be Link).
	 *
	 * Checks if given annotType is Link and if yes, initializes given
	 * dictionary as Link annotation. Otherwise immediately returns with false.
	 * <br>
	 * Assumes that following entries are initialized: Type, P, Rect, M.
	 * <br>
	 * Initialization doesn't do any checking and so annotation dictioanries
	 * initialized to different type shouldn't be used here. Result may be
	 * incorrect annotation dictionary in such situation.
	 *
	 * @return true if initialization is successfull, false otherwise.
	 */
	virtual bool operator()(boost::shared_ptr<CDict> & annotDict, std::string annotType);
};
} // namespace utils

/** High level object for annotations.
 *
 * This class wrapps pdf annotation dictionary and provides simple interface
 * for manipulation with this dictionary. Also implements logic needed for
 * proper annotation handling.
 * <br>
 * Instance can be created only from existing annotation dictionary or using
 * factory method createAnnotation.
 * <br>
 * Each CAnnotation instance is valid while it is accessible from some page
 * dictionary. In moment when reference to it is removed from page (more
 * precisely from Annots array in page dictionary), instance should be
 * invalidated. This is done by annotation maintainer (CPage instance) using
 * invalidate method. Note that all methods are available also when instance
 * is invalidated. Annotation keeper should register observer which handles
 * situation that instance is not valid anymore. Other possibility is to call
 * isValid method.
 * <br>
 * Maintained dictionary is returned by getDictionary method. This can be used
 * to perform unsupported or unusual operations.
 */
class CAnnotation: public observer::ObserverHandler<CAnnotation>
{
	/** Private constructor.
	 */
	CAnnotation ();

	/** Annotation dictionary.
	 *
	 * Pdf annotation dictionary maintained by this instance.
	 */
	boost::shared_ptr<CDict> annotDictionary;

	/** Initializator for annotations.
	 * This implementator is used when annotation dictionary should be
	 * initialized.
	 * Concrete implementation can be set by setAnnotInitializator method.
	 */
	static boost::shared_ptr<utils::IAnnotInitializator> annotInit;

	/** Validity flag.
	 *
	 * Value is initialized to true in constructor and can be changed by
	 * invalidate method to false.
	 */
	bool valid;
public:
	/** Type for observer change context.
	 * This class produces BasicChangeContext.
	 */
	typedef observer::BasicChangeContext<CAnnotation> ChangeContext;
	
	/** Annotation type enumeration.
	 * List of all known types according pdf specification. See PDF
	 * specification chapter 8.4.5. Annotation Types.
	 * <br>
	 * CAnnotation doesn't have to support initialization of all listed types
	 * (it depends on annotation initializator).
	 */
	enum AnnotType 
	{
	Text, Link, FreeText, Line, Square, Circle, Polygon, PolyLine, Highlight, 
	Underline, Squiggly, StrikeOut, Stamp, Caret, Ink, Popup, Fileattachement, 
	Sound, Movie, Widget, Screen, Printermark, Trapnet, Watermark, _3D, Unknown
	};
	
	/** Initialization constructor.
	 * @param annotDict Annotation dictionary.
	 *
	 * Initializes annotDictionary from given one and sets valid flag to true.
	 * Doesn't perform any checking. Given dictionary has to be in pdf and its
	 * reference in Annots array.
	 * <br>
	 * This should be used when caller is sure that given annotDict is ok (e. g.
	 * one returned from createAnnotation method). 
	 */
	CAnnotation(boost::shared_ptr<CDict> annotDict):annotDictionary(annotDict), valid(true){}
	
	/** Static factory method for annotation creation.
	 * @param rect Rectangle for annotation (location on the screen in default 
	 * user space units).
	 * @param annotType Type of the annotation.
	 *
	 * Creates new annotation dictionary and do some initialization according
	 * given parameters.
	 * <br>
	 * At first fills maintaining information requiered by pdf specification to 
	 * the dictionary, such as Type, M and Rect fields.
	 * Then uses annotInit initializator for type specific initialization with 
	 * given annotType (it depends on annotInit static initializator what it is 
	 * done). 
	 * <br>
	 * Created annotation can be used in CPage::addAnnotation method.
	 *
	 * @return CAnnotation instance wrapped by shared pointer.
	 */
	static boost::shared_ptr<CAnnotation> 
		createAnnotation(Rectangle rect, std::string annotType);

	/** Destructor.
	 *
	 * Deallocates all additional data needed for work. Doesn't do anything with
	 * annotDictionary field.
	 */
	~CAnnotation ()
	{
	}

	/** Sets new annotation initializator.
	 * @param init Initializator to be used.
	 *
	 * Static method which sets new implementator for annotation initializator.
	 * If given init holds non NULL initializator, sets it to the static
	 * annotInit field. 
	 *
	 * @return previous implementation.
	 */
	static boost::shared_ptr<utils::IAnnotInitializator> 
		setAnnotInitializator(boost::shared_ptr<utils::IAnnotInitializator> init)
	{
		boost::shared_ptr<utils::IAnnotInitializator> old=annotInit;

		// if given is non NULL, sets new value
		if(init.get())
			annotInit=init;

		return old;
	}
	
	/** Retuns validity status.
	 *
	 * @return value of valid flag.
	 */
	bool isValid()const
	{
		return valid;
	}
	
	/** Invalidates this annotation.
	 * 
	 * This method should be called when annotation maintainer (there should be
	 * only one according pdf specification) releases reference to annotation
	 * dictionary. All registered observers are notified - newValue parameter
	 * stands for this annotation instance and context is empty (shared_ptr
	 * contains NULL).
	 * <br>
	 * If valid flag is already false, nothing is done.
	 */
	void invalidate();

	/** Returns type of annotation.
	 *
	 * Gets Subtype field value from maintained annotation dictionary and maps
	 * it to enumenration AnnotType value. If this type is not one from pdf
	 * specification (valid in time of program writting) or Subtype field is 
	 * not present, Unknown value is returned.
	 *
	 * @return AnnotType value of current annotation type.
	 */
	AnnotType getType()const;

	/** Returns maintained annotation dictionary.
	 *
	 * This can be used if CAnnotation interface doesn't provide required
	 * functionality. User can perform changes directly on annotation
	 * dictionary. Nevertheless this can be rather dangerous becuse nonsese
	 * information may be provided.
	 */
	boost::shared_ptr<CDict> getDictionary()const
	{
		return annotDictionary;
	}

	/** Equality operator.
	 * @param annot Annotation to compare with this.
	 *
	 * Two annotations are same if they maintain same annotation dictionaries.
	 */
	bool operator ==(const CAnnotation & annot)
	{
		return getDictionary()==annot.getDictionary();
	}
};

} // namespace pdfobjects


#endif // __CANNOTATION_H__

