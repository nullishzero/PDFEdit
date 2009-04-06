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

#ifndef _OBSERVER_H
#define _OBSERVER_H

#include <vector>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <algorithm>
#include "os/compiler.h"

#include "utils/debug.h"

//=============================================================================
namespace observer
{


/**
 * Observer exception.
 */
struct ObserverException : public std::exception
{
};

/** Supported context types.
 */
enum ChangeContextType {BasicChangeContextType, ComplexChangeContextType, ScopedChangeContextType};

/** Operator for human readable ChangeContextType printing.
 * @param str Stream where to print.
 * @param type Change context type.
 */
inline std::ostream & operator<<(std::ostream & str, ChangeContextType type)
{
	switch(type)
	{
		case BasicChangeContextType:
			str << "BasicChangeContextType";
			break;
		case ComplexChangeContextType:
			str << "ComplexChangeContextType";
			break;
		case ScopedChangeContextType:
			str << "ScopedChangeContextType";
			break;
	}
	return str;
}
	
/** Interface (pure abstract class) for change context.
 *
 * This is base class for all change contexts. It contains just information
 * about supported contexts (in enum) and type of context. 
 * <br>
 * User of this change context should check context type at first and than cast
 * to specific context subclass.
 */
template<typename T> class IChangeContext
{
public:
	
	/** Returns context type.
	 *
	 * @return context type.
	 */
	virtual ChangeContextType getType()const =0;

	/** Virtual destructor.
	 *
	 */
	virtual ~IChangeContext(){}
};

/** Basic change context template class.
 * 
 * Simplest subclass from IChangeContext. It holds previous value of changed
 * one. This value is wrapped by smart pointer to keep clear instancion policy.
 * <br>
 * Exception NOTE:
 * No method throws an exception.
 */
template<typename T> class BasicChangeContext:public IChangeContext<T>
{
	/** Original value. */
	boost::shared_ptr<T> originalValue;
public:
	/** Constructor with original value.
	 * @param origVal Original value used for originalValue initialization.
	 */
	BasicChangeContext(boost::shared_ptr<T> origVal)throw() :originalValue(origVal){}

	/** Destructor.
	 * Just to mark all destructors in subtypes as virtual.
	 */
	virtual ~BasicChangeContext()throw() {}

	/** Returns type of context.
	 *
	 * @return BasicChangeContextType value.
	 */
	ChangeContextType getType() const throw()
	{
		return BasicChangeContextType;
	}

	/** Returns original value.
	 *
	 * @return Orignal value wrapped by smart pointer.
	 */
	virtual boost::shared_ptr<T> getOriginalValue() const throw()
	{
		return originalValue;
	}
};

/** Complex change context template class.
 *
 * This context should be used when value with given ValueType template type is
 * changed in context of complex value type (it is its part) and there is unique
 * identification of this value.
 * <br>
 * Observer may use original value which is accessible from
 * BasicChangeContextType supertype and additional value identificator
 * information.
 */
template<typename ValueType, typename ValueIdType>
class ComplexChangeContext: public BasicChangeContext<ValueType>
{
	ValueIdType valueId;
public:
	/** Initialization constructor.
	 * @param origVal Original value.
	 * @param id Identificator of changed value.
	 *
	 * Initializes originalValue (uses BasicChangeContext constructor with
	 * origVal parameter) and valueId from given id.
	 */
	ComplexChangeContext(boost::shared_ptr<ValueType> origValue, ValueIdType id):
		BasicChangeContext<ValueType>(origValue), valueId(id)
	{}

	virtual ~ComplexChangeContext()throw(){}

	/** Returns identificator of changed value.
	 *
	 * @return value identificator value.
	 */
	ValueIdType getValueId()const throw()
	{
		return valueId;
	}
	
	/** Returns type of context.
	 *
	 * @return ComplexChangeContextType value.
	 */
	ChangeContextType getType() const throw()
	{
		return ComplexChangeContextType;
	}
};

/** Change context with scope information.
 *
 * Scope information are specified as template parameter and describes scope of
 * the change. Template T parameter specifies value holder.
 * <br>
 * Scope can be rather complex information but principally it should contain
 * information which can be used to cathegorize value given in notify method as
 * newValue parameter (e. g. Maximum number of value for progress calculation
 * and newValue contains current state).
 * 
 */
template<typename T, typename S> class ScopedChangeContext:public IChangeContext<T>
{
	/** Scope holder.
	 */
	boost::shared_ptr<S> scope; 
public:
	/** Initialize constructor.
	 * @param s Scope used for initialization.
	 *
	 * Initializes scope field.
	 */
	ScopedChangeContext(boost::shared_ptr<S> s):scope(s){}

	/** Returns scope.
	 * @return Scope value wrapped by shared_ptr smart pointer.
	 */
	boost::shared_ptr<S> getScope()const
	{
		return scope;
	}

	/** Returns context type.
	 *
	 * @return Returns ScopedChangeContextType.
	 */
	ChangeContextType getType() const throw()
	{
		return ScopedChangeContextType;
	}

};


/** Observer interface (pure abstract class).
 *
 * Observer is mechanism how to be informed that somethig has changed. 
 * Implementator of this interface produces handler for this change event.
 * <br>
 * This should be used in following way:
 * <ul>
 * <li>value keeper which wants to enable observers has to implement
 * ObserverHandler interface which enables to register and unregister 
 * observers. It guaranties it calls notify on each registered observer after
 * change was registered.
 * <li>implementator of class is responsible for notify method implementation
 * which handles situation.
 * <li>observer must be registered on target value keeper.
 * </ul>
 *
 * Interface is template to enable different value types to be used (type 
 * parameter stands for value type to be managed - informed about change). 
 * This enables only one type to be handled by one implementator (with all 
 * subtypes).
 * <br>
 * Value change handling is done in notify method (see for more details). This
 * method is called after value has been changed. 
 * <br>
 * Each observer implementation has its priority which is used be value keeper
 * to determine order in which to notify obsevers, if there is more then one.
 * <br>
 * Observer can be in active/inactive state depending on the active flag value.
 * See setActive, isActive methods.
 * <br>
 * Exception NOTE:
 * No method throws an exception.
 */
template<typename T> class IObserver
{
	/** Active flag.
	 * Observer is ignored by observer handler if it is false.
	 */
	bool active;
public:
	/** Type for priority.
	 */
	typedef int priority_t;

	/** Default constructor.
	 */
	IObserver():active(true) {}
	
	/** Notify method.
	 * @param newValue New value of changed value or its part.
	 * @param context Value change context.
	 * 
	 * Each time value keeper, which implements ObserverHandler, changes 
	 * value (or its part), all registered observers are notified about that
	 * by this method calling.
	 * <br>
	 * newValue stands for new value or its part (if value is complex). 
	 * <br>
	 * contex contains additional information about change. It depends on
	 * value keeper which information is provided (if any). It may be NULL
	 * (value stored in shared_ptr may be NULL - more precisely - use 
	 * contex.get()==NULL condition for checking) which means that no 
	 * information is provided. Method implementator should check context 
	 * type (using getType() method). Accoring this type cast to correct 
	 * IChangeContext subclass and use information.
	 * <p>
	 * <b>Example</b>:<br>
	 * Lets say that T is IProperty. If observer is registered on simple
	 * type, newValue contains new value of this simple one. If it is
	 * registered on complex type, it contains changed value (item) inside 
	 * this complex type (value keeper part is changed).
	 */
	virtual void notify (boost::shared_ptr<T> newValue, boost::shared_ptr<const IChangeContext<T> > context) const throw() = 0 ;

	/** Returns priority of obsever.
	 *
	 * Lower number means higher priority.
	 * @return Observer priority value.
	 */
	virtual priority_t getPriority()const throw() =0;

	/** Sets active flag value.
	 * @param active Flag value to be set.
	 * @return previous value of the flag.
	 */
	bool setActive(bool active)
	{
		bool oldValue = this->active;
		this->active = active;
		return oldValue;
	}

	/** Returns current value of the active flag.
	 * @return true if observer can be notified, false otherwise.
	 */
	bool isActive()const
	{
		return active;
	}

	/**
	 * Virtual destructor.
	 */
	virtual ~IObserver ()throw() {}
};

/** Priority comparator functor.
 *
 * T template parameter stands for elements which priority should be compared.
 * It has to provide getPriority method and has to be pointer compatible type
 * (pointer or kind of smart pointer).
 */
template <typename T>
struct PriorityComparator
{
	/** Comparator functor.
	 * @param value1 Value to compare.
	 * @param value2 Value to compare.
	 * 
	 * @return value1-&gt;getPriority() &lt; value2-&gt;getPriority()
	 */
	bool operator ()(const T & value1, const T & value2)const
	{
		return value1->getPriority() < value2->getPriority();
	}
};

/** Storage with sorted elements according priority.
 *
 * Template T parameter stands for stored element type, Storage is type of
 * underlying container type which is used for element storing - it has to
 * provide push_back and erase methods, const_iterator and iterator types.
 * Compare is 
 * functor for priority comparing (it should conform stric weak ordering
 * comparision).
 * <br>
 * Given T has to provide getPriority method.
 */
template<typename T, typename Storage=std::vector<T>, typename Compare=PriorityComparator<T> >
class PriorityList
{
	Storage c;
	Compare comp;
public:
	/** Type for constant iterator.
	 */
	typedef typename Storage::const_iterator const_iterator;

	/** Returns iterator for first element in queue.
	 * Iterator points to element with highest priority.
	 */
	const_iterator begin()const 
	{
		return c.begin();
	}

	/** Returns iterator behind last element in queue.
	 */
	const_iterator end()const
	{
		return c.end();
	}

	/** Returns constant itetor to element with same value.
	 * @param value Value to find.
	 *
	 * @return const_iterator for given value.
	 */
	const_iterator find(const T & value)const
	{
		for(const_iterator i=begin(); i!=end(); i++)
		{
			const T & elem=*i;
			if(elem==value)
				return i;
		}

		return end();
	}

	/** Inserts new value to the storage.
	 * @param value Value to insert.
	 *
	 * Inserts new element to the storage and sorts it by std::sort function.
	 */
	void insert(const T & value)
	{
		c.push_back(value);
		sort(c.begin(), c.end(), comp);
	}
	
	/** Removes given value from list.
	 * @param value Value to remove.
	 *
	 * Removes element from container and keeps priority ordering for other
	 * elements.
	 */
	void erase(const T & value)
	{
		for(typename Storage::iterator i=c.begin(); i!=c.end(); i++)
		{
			T & elem=*i;
			if(elem==value)
			{
				// removes iterator - removing from sorted container produces
				// sorted container
				c.erase(i);	
				return;
			}
		}
	}

	/** Returns number of registered elements.
	 * @return number of elements.
	 */
	size_t size()const
	{
		return c.size();
	}
};

#ifdef OBSERVER_DEBUG
/** Helper macro for debug information printing.
 * Note that this will be empty if OBSERVER_DEBUG is not defined.
 */
#define PRINT_DEBUG_INFO(obj, observer, prefix)		\
	do {						\
		std::cerr << prefix <<"("<<(obj) <<","<<(observer)<<") from ";\
		std::cerr <<__FILE__ <<":"<< __FUNCTION__ <<":"<< __LINE__ << std::endl;\
	} while (0)
#else
#define PRINT_DEBUG_INFO(obj, observer, prefix) do {} while(0)
#endif

/** Wrapper for observer registration.
 * @param obj Observer handler (ObserverHandler wrapped by shared_ptr).
 * @param observer Observer to be registered (IObserver wrapped by shared_ptr).
 *
 * Note that this way of observer registration is preffered because
 * of possible debug information.
 * <br>
 * Don't use expression parameters with side effects (e.g. ++i etc.), 
 * because they may be executed more than once (if debuging is used).
 */
#define REGISTER_SHAREDPTR_OBSERVER(obj, observer)	\
	do {					\
		PRINT_DEBUG_INFO((obj).get(), (observer).get(), "registerObserver");\
		obj->registerObserver(observer);\
	}while(0)

/** Wrapper for observer unregistration.
 * @param obj Observer handler (ObserverHandler wrapped by shared_ptr).
 * @param observer Observer to be registered (IObserver wrapped by shared_ptr).
 *
 * Note that this way of observer unregistration is preffered because
 * of possible debug information.
 * <br>
 * Don't use expression parameters with side effects (e.g. ++i etc.), 
 * because they may be executed more than once (if debuging is used).
 */
#define UNREGISTER_SHAREDPTR_OBSERVER(obj, observer)	\
	do {					\
		PRINT_DEBUG_INFO((obj).get(), (observer).get(), "unregisterObserver");\
		obj->unregisterObserver(observer);\
	}while(0)

/** Wrapper for observer registration.
 * @param obj Observer handler (simple pointer to ObserverHandler).
 * @param observer Observer to be registered (simple pointer to IObserver).
 *
 * Note that this way of observer registration is preffered because
 * of possible debug information.
 * <br>
 * Don't use expression parameters with side effects (e.g. ++i etc.), 
 * because they may be executed more than once (if debuging is used).
 */
#define REGISTER_PTR_OBSERVER(obj, observer)	\
	do {					\
		PRINT_DEBUG_INFO((obj), (observer), "registerObserver");\
		obj->registerObserver(observer);\
	}while(0)

/** Wrapper for observer unregistration.
 * @param obj Observer handler (simple pointer to ObserverHandler).
 * @param observer Observer to be unregistered (simple pointer to IObserver).
 *
 * Note that this way of observer unregistration is preffered because
 * of possible debug information.
 * <br>
 * Don't use expression parameters with side effects (e.g. ++i etc.), 
 * because they may be executed more than once (if debuging is used).
 */
#define UNREGISTER_PTR_OBSERVER(obj, observer)	\
	do {					\
		PRINT_DEBUG_INFO((obj), (observer), "unregisterObserver");\
		obj->unregisterObserver(observer);\
	}while(0)

/** Base class for all notifiers.
 a*
 * Each class which want to support observers should inherit from this class. It
 * provides basic implementation for registering, unregistering and notification
 * of observers.
 * Whenever change occures, subclass is responsible to call notifyObservers
 * method and provide it with correct parameters. Rest is done by superclass.
 * <br>
 * T template parameter stands for value type which is observed (same as used
 * for IObserver template type). Observers are IObserver with T type wrapped 
 * by shared_ptr smart pointer.
 * <br>
 * This implementation supports priority ordering of observers.
 *
 */
template<typename T> class ObserverHandler
{
public:
	/** Type for observer.
	 * Alias to IObserver with T type wrapped by shared_ptr smart pointer.
	 */
	typedef boost::shared_ptr<const IObserver<T> > Observer; 

	/** Type for observer list.
	 * Alias to PriorityList with shared_ptr&lt;Observer&gt; value type,
	 * vector underlying container and PriorityComparator comparator.
	 */
	typedef PriorityList<Observer> ObserverList;

	/** Alias for ObserversContext without template parameter.
	 */
	typedef IChangeContext<T>  ObserverContext;
protected:
	/** List of registered observers.
	 * It is iterable priority queue, so observers are ordered according their
	 * priorities (higher priority closer to begin - NOTE that higher priority
	 * means smaller priority value.
	 */
	ObserverList observers;

	/** Prints all registered observers.
	 * Each observer is printed as pointer address. 
	 * Method is empty if OBSERVER_DEBUG is not defined.
	 */
	void dumpObservers()const
	{
#ifdef OBSERVER_DEBUG
		size_t size = observers.size();
		std::cerr << " observer list (size="
				<<size<<"): ";
		typename ObserverList::const_iterator it = observers.begin();
		for(;it!=observers.end(); ++it)
		{
			Observer observer = *it;
			std::cerr << observer.get() << " ";
		}
#endif
	}
public:
	/** Empty destructor.
	 */
	virtual ~ObserverHandler() {}
	
	/** Registers new observer.
	 * @param observer Observer to register (if NULL, nothing is registered).
	 *
	 * Adds given observer to observers list. Ignores if observer is already in
	 * the list.
	 *
	 * @throw ObserverException if given observer is not valid (it is NULL).
	 */
	virtual void registerObserver(const Observer & observer)
	{
		if (observer.get())
		{
			// ignores if it is already in the list
			if(observers.find(observer)==observers.end())
				observers.insert(observer);
		}
		else
			throw ObserverException ();

	}

	/** Unregisters given observer.
	 * @param observer Observer to unregister.
	 *
	 * Removes given observer from observers list. 
	 *
	 * @throw ObserverException if observer is not in observers list.
	 */
	virtual void unregisterObserver(const Observer & observer)
	{
		if (observer.get())
		{
			typename ObserverList::const_iterator it = observers.find(observer);

			if (it != observers.end ())
				observers.erase (observer);
			else
				throw ObserverException ();
		
		}else
			throw ObserverException ();
	}

	/**
	 * Notify all active observers about a change.
	 *
	 * Observers are notified in according their priorities. Higher priority
	 * (smaller priority value) sooner it is called. Observers with same
	 * priorities are called in unspecified order. All inactive observers
	 * are ignored.
	 *
	 * @param newValue Object with new value.
	 * @param context Context in which the change has been made.
	 */
	virtual void notifyObservers (boost::shared_ptr<T> newValue, boost::shared_ptr<const ObserverContext> context)
	{
		// obsrvers list is ordered by priorities, so iteration works correctly
		typename ObserverList::const_iterator it = observers.begin ();
		for (; it != observers.end(); ++it)
		{
			Observer o = (*it);
			if(o->isActive())
				o->notify (newValue, context);
		}
	}
	
};


/**
 * Checks whether the observer list is empty.
 */
template<typename T>
inline static void 
check_observerlist (UNUSED_PARAM const T& list) 
{
#ifdef OBSERVER_DEBUG
	if (list.size()) 
	{
		kernelPrintDbg (debug::DBG_CRIT, "unregistered observer(s): " << list.size());
	}
#endif
}


} // namespace observer




#endif  // _OBSERVER_H
