// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$ 
 *
 * $Log$
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
	typedef std::vector<std::string> SupportedList;

	virtual ~IAnnotInitializator(){};
	
	/** Returns list of supported annotation types.
	 *
	 * Returned container contains all supprted annotation types, which can be
	 * used in function operator to make some initialization.
	 * 
	 * @return SupportedList container with all supprted types.
	 */
	virtual SupportedList getSupportedList()const =0;

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
	typedef std::map<std::string, boost::shared_ptr<IAnnotInitializator> > AnnotList;
private:
	AnnotList implList;
public:
	/** Empty constructor.
	 *
	 * Doesn't do any special initialization. implList is empty and so no
	 * initializators are supported by default.
	 */
	UniversalAnnotInitializer(){}
	
	/** Initialization constructor from exiting mapping.
	 * @param impls Existing mapping.
	 *
	 * Use given mapping for implList initialization.
	 */
	UniversalAnnotInitializer(AnnotList impls):implList(impls){}

	virtual ~UniversalAnnotInitializer(){};

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
 */
class TextAnnotInitializer: public IAnnotInitializator
{
public:
	/** Default value for Open entry.
	 * Value is true by default.
	 */
	static bool OPEN;
	
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
	 * Initialization doesn't do any checking and so annotation dictioanries
	 * initialized to different type shouldn't be used here. Result may be
	 * incorrect annotation dictionary in such situation.
	 *
	 * @return true if initialization is successfull, false otherwise.
	 */
	virtual bool operator()(boost::shared_ptr<CDict> & annotDict, std::string annotType)const;
};

} // namespace utils

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

	/** Initializator for annotations.
	 * This implementator is used when annotation dictionary should be
	 * initialized.
	 * Concrete implementation can be set by setAnnotInitializator method.
	 */
	static boost::shared_ptr<utils::IAnnotInitializator> annotInit;
public:
	/** Initialization constructor.
	 * @param annotDict Annotation dictionary.
	 *
	 * Initializes annotDictionary from given one and sets valid flag to true.
	 * Doesn't perform any checking.
	 * <br>
	 * This should be used when caller is sure that given annotDict is ok (e. g.
	 * when dictionary is created from template)
	 */
	CAnnotation(boost::shared_ptr<CDict> annotDict):annotDictionary(annotDict), valid(true){}
	
	/** Initialization constructor.
	 */
	CAnnotation(boost::shared_ptr<CDict> annotDict, 
			boost::shared_ptr<CDict> page, 
			std::string annotType);

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

		// if given is un NULL, sets new value
		if(init.get())
			annotInit=init;

		return old;
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

} // namespace pdfobjects


#endif // __CANNOTATION_H__

