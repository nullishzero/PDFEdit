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

#ifndef _CPAGEANNOTS_H_
#define _CPAGEANNOTS_H_

// static includes
#include "kernel/static.h"
#include "kernel/cpagemodule.h"
#include "kernel/iproperty.h"


//==========================================================
namespace pdfobjects {
//==========================================================

// Forward declaration
class CPage;
class CAnnotation;


//==========================================================
// CPageAnnots
//==========================================================

/**
 * Class representing annotations.
 * Provides convinient access and modify operations on "Annots" entry of a page dictionary.
 */
class CPageAnnots : public ICPageModule
{

	//==========================================================
	// Annots observers
	//==========================================================
private:
	/** 
	 * Observer for Annots property.
	 * This observer is registered on page dictionary and if Annots property is
	 * a reference also to this property. Any change which leads to change of 
	 * Annots array (either add, remove or change) is handled here.  Note that
	 * it doesn't handle array content change.
	 */
	class PropWatchDog: public IPropertyObserver
	{
		CPageAnnots* _annots;

	public:
		/** Initialization constructor.
		 * Sets annots field according parameter.
		 *
		 * @param annots CPageAnnots instance.
		 */
		PropWatchDog(CPageAnnots* annots):_annots(annots)
		{
			// given parameter must be non NULL
			// this is used only internaly by CPageAnnots, so assert is enough for
			// checking
			assert(_annots);
		}

		/** Empty destructor. */
		virtual ~PropWatchDog() throw(){}
		
		/** Observer handler.
		 * 
		 * Checks given context type:
		 * <ul>
		 * <li>BasicChangeContext means that Annots property in page dictionary
		 * is reference and its value has changed.
		 * <li>ComplexChangeContext means that page dictionary has changed. So
		 * checks property id and if it not Annots, immediatelly returns,
		 * because this change doesn't affect annotations. Otherwise checks
		 * original value type. If it is reference, unregisters this observer
		 * from it. If newValue is reference, registers observer to it.
		 * </ul>
		 * In any case:
		 * <ul>
		 * <li>Tries to get array from oldValue and unregister observers from
		 * it (uses page-&gt;unregisterAnnotsObservers).
		 * <li>Invalidates and removes all annotations from 
		 * page-&gt;annotStorage.
		 * <li>collects all current annotations (uses collectAnnotations).
		 * <li>Tries to get current Annots array and registers observers to it
		 * (uses page-&gt;registerAnnotsObservers)
		 * </ul>
		 *
		 * @param newValue New value of changed property.
		 * @param context Context of the change.
		 */
		virtual void notify (boost::shared_ptr<IProperty> newValue, 
							 boost::shared_ptr<const IProperty::ObserverContext> context) const throw();

		/** Returns observer priority. */
		virtual priority_t getPriority() const throw()
			{ /** TODO some constant */ return 0; }
	
	}; // class AnnotsPropWatchDog

	/** 
	 * Observer for Annots array synchronization.
	 * This observer is registered on Annots array property and all its
	 * reference typed elements. It handles change in Annots array content -
	 * this means either element is added, removed or replaced, or any of its
	 * reference elements changes its value.
	 */
	class ArrayWatchDog: public IPropertyObserver
	{
		/** Page owner of this observer.*/
		CPageAnnots* _annots;

	public:
		typedef observer::BasicChangeContext<IProperty> BasicObserverContext;
			
		/** Initialization constructor.
		 * Sets page field according parameter.
		 *
		 * @param _page CPage instance.
		 */
		ArrayWatchDog(CPageAnnots* annots):_annots(annots)
		{
			// given parameter must be non NULL
			// this is used only internaly by CPage, so assert is enough for
			// checking
			assert(_annots);
		}

		/** Empty destructor. */
		virtual ~ArrayWatchDog() throw(){}
		
		/** Observer handler.
		 * 
		 * Checks given context type:
		 * <ul>
		 * <li>BasicObserverContext means that Annots array reference element 
		 * has changed its value.
		 * <li>ComplexChangeContext means that Annots array content has changed.
		 * If original value is reference, then unregisters this obsever from
		 * it. If newValue is reference registers this observer to it.
		 * <li>Different context is not supported and so method immediatelly
		 * returns.
		 * </ul>
		 * In both situations calls consolidateAnnotsStorage with original and
		 * new value parameters.
		 *
		 * @param newValue New value of changed property.
		 * @param context Context of the change.
		 */
		virtual void notify (boost::shared_ptr<IProperty> newValue, 
							 boost::shared_ptr<const IProperty::ObserverContext> context) const throw();

		/** Returns observer priority. */
		virtual priority_t getPriority()const throw()
			{ /* TODO some constant */ return 0; }
	};

	//==========================================================

	// Typedefs
public:
	/** Type for annotation storage. */
	typedef std::vector<boost::shared_ptr<CAnnotation> > Annotations;

	
	// Variables
private:
	CPage* _page;	// pages
	/** Annotations on this page. */
	Annotations _annotations;
	/** Watchdog for Annots property. @see AnnotsPropWatchDog */
	boost::shared_ptr<PropWatchDog> _prop_wd;
	/** Watchdog for Annotation array. @see AnnotsArrayWatchDog */
	boost::shared_ptr<ArrayWatchDog> _array_wd;


	// Ctor & Dtor
public:
	CPageAnnots (CPage* page);
	~CPageAnnots ();

	//
	// ICPageModule interface
	//
public:
	/** @see ICPageModule::reset */
	virtual void reset ();

	//
	// Methods
	//
public:

	/** 
	 * Fills given container with all page's annotations.
	 * 
	 * Copies _annotations content to given container (which is cleared at
	 * first).
	 * <br>
	 * Given container must support clear and insert operations and store
	 * shared_ptr&lt;CAnnotation$gt; elements. 
	 * 
	 * @param container Container which is filled in.
	 */
	template<typename T>
	void getAll (T& container)const
	{
		container.clear();	
		container.insert(container.begin(), _annotations.begin(), _annotations.end());
	}


	/** 
	 * Adds new annotation to this page.
	 * 
	 * Inserts deep copy of given annotation and stores its reference to Annots
	 * array in page dictionary (if this doesn't exist, it is created). 
	 * User has to call getAllAnnotations to get current annotations state (we 
	 * don't have identifier for annotations - there are some mechanisms how to 
	 * do it according pdf specification, but there is no explicit identifier).
	 * <br>
	 * Given annotation may come from different CPdf or may belong to nowhere.
	 * <br>
	 * As a result _annotations is updated. New indirect object representing
	 * annotation dictionary is added to same pdf (dictionary is same as given
	 * one except P field is updated to contain correct reference to this page).
	 * <br>
	 * Note that this page must belong to pdf and has to have valid indirect
	 * reference. This is neccessary, because annotation is indirect object page
	 * keeps reference to it. Reference without pdf doesn't make sense.
	 *
	 * @param annot Annotation to add.
	 * @throw CObjInvalidObject if this page doesn't have valid pdf or indirect
	 * reference.
	 * @throw ElementBadTypeException if Annots field from page dictionary is
	 * not an array (or reference with array indirect target).
	 */ 
	void add(boost::shared_ptr<CAnnotation> annot);

	/** Removes given annotation from page.
	 * @param annot Annotation to remove.
	 *
	 * Tries to find given annotation in _annotations and if found, removes
	 * reference from Annots array.
	 * <br>
	 * As a result, removed annotation is invalidated and not accessible. User 
	 * has to call getAllAnnotations method to get current state (same way as 
	 * in addAnnotation case).
	 *
	 * @return true if annotation was removed.
	 */
	bool del(boost::shared_ptr<CAnnotation> annot);


	//
	// Helper methods
	//
private:

	/** 
	 * Registers observers for annotation synchronization.
	 * Checks type of given property and if it is reference, registers
	 * AnnotsPropWatchDog observer to it and dereferences indirect object. If 
	 * annots or dereferenced object is array, registers _annots_array_wd 
	 * observer to it and all its reference type elements.
	 *
	 * By default, it registers annotation observer for page dictionary.
	 *
	 * @param annots Annots property.
	 */
	void reg_observers(boost::shared_ptr<IProperty> annots = boost::shared_ptr<IProperty>());

	/** 
	 * Unregisters obsevers from given Annots property.
	 * This method works reversely to registerAnnotsObservers (observers are
	 * unregistered but rest is same).
	 * 
	 * By default, it registers annotation observer for page dictionary.
	 *
	 * @param annots Annots property.
	 */
	void unreg_observers(boost::shared_ptr<IProperty> annots = boost::shared_ptr<IProperty>());


	/** 
	 * Consolidates _annotations field according given change.
	 * Works in two steps. First handles oldValue and second newValue. At first
	 * checkes oldValue type and if it is reference, dereference indirect
	 * objects and if it is annotation dictionary, it will invalidate 
	 * associated CAnnotation and removes it from _annotations. 
	 * <br>
	 * In second step, checks newValue type and if it is reference to
	 * dictionary, it will create new CAnnotation instance and adds it to
	 * _annotations. 
	 * 
	 * @param oldValue Removed reference from _annotations.
	 * @param newValue Added reference to the _annotations.
	 */
	void consolidate(boost::shared_ptr<IProperty>& oldValue, boost::shared_ptr<IProperty>& newValue);

}; // class CPageAnnots



//==========================================================
} // namespace pdfobjects
//==========================================================

#endif // _CPAGEANNOTS_H_

