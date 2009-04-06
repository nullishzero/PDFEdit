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

#ifndef _PDFOPERATORSBASE_H_
#define _PDFOPERATORSBASE_H_

// static includes
#include "kernel/static.h"
#include "kernel/iproperty.h"
#include "utils/iterator.h"
#include "utils/listitem.h"

//==========================================================
namespace pdfobjects {
//==========================================================


//
// Forward declaration
//
class CContentStream;
class CPdf;

							 
//==========================================================
// PdfOperator
//==========================================================
	
/**
 * This class is the base class for COMPOSITE design pattern and also DECORATOR design pattern.
 * 
 * This is a special implementation, a hybrid between implementation of stl containers and GoF
 * example.
 *
 * Content stream consists of operators and their operands. Operators can be
 * composites. Each operator alters graphical state in a predefined way.
 * Operators have to be processed sequentially.
 * 
 * This is the building  block of almost all visible objects on a page. Every
 * operation on a text, graphics etc.. means adding/deleting/changing operators. 
 *
 * Operators themselves build a double linked list queue. We use iterators to
 * iterate over them sequentially.
 *
 * They also form a tree like structure and that is the reason why Composite
 * design pattern is used here. Decorator and composite together allow a very
 * easy way to change these operators. (e.g. changing font means 
 * wrapping the text operator to a composite, which includes also the font
 * change operator.) 
 *
 * This approach allows us to represent operators in a human readable way
 * (tree like structure) and to have the power of double linked list with
 * iterators.
 *
 * We can not simply represent these two queues in one. If we want to have all
 * info only in interator queue, items can not be shared pointers because of the
 * cyclic shared pointer problem (no item would get ever deallocated). If we
 * want to have all the info in tree like queue, we would have to know the
 * parents and it would not be easy to write an iterator. It would not be a clean
 * straight forward solution. Another possibility is to have the queue outside
 * (e.g. in content stream) but then we would not be able to simply create
 * composites.
 *
 * Another advantage of the composite design pattern is the single unified interface.
 * In most cases we do not need to distinguish between a composite and a leaf.
 *
 * Iterators are crucial when selecting operators. 
 */
class PdfOperator : public ListItem<PdfOperator>
{	
		
	// Typedefs
public:
	// containers
	typedef std::deque<boost::shared_ptr<IProperty> > 	Operands;
	typedef std::list<boost::shared_ptr<PdfOperator> > 	PdfOperators;
	// iterators
	typedef iterator::DoubleListIterator<PdfOperator>	Iterator;
	typedef Iterator::ListItem							ListItem;
	// bbox
	typedef libs::Rectangle								BBox;

	// Friends
public:
	friend class iterator::DoubleListIterator<PdfOperator>;
	friend class CContentStream;

	// Variables
private:
	/** This enables mapping between pdfoperator and contentstream. */
	CContentStream* _contentstream;
	
	// Ctor & Dtor
protected:
	/** Constructor. */
	PdfOperator () : _contentstream (NULL) {}

	// Destructor
public:
	/** Destructor. */
	virtual ~PdfOperator ()	{}

	
	//
	// Pdf operator interface
	//
public:
	
	/**
	 * Return the number of parameters.
	 *
	 * @return Number of parameters.
	 */
	virtual size_t getParametersCount () const = 0;

	/**
	 * Get the parameters used with this operator.
	 *
	 * @param container Will be used to store parameters.
	 */
	virtual void getParameters (Operands& container) const = 0;

	/**
	 * Get the string representation of this operator.
	 *
	 * @param str String that will hold the representation of this operator.
	 */
	virtual void getStringRepresentation (std::string& str) const = 0;

	/**
	 * Get the string operator name.
	 *
	 * @param first String that will hold operator name.
	 */
	virtual void getOperatorName (std::string& first) const = 0;

	
	//
	// Composite interface
	//
public:
	
	/**
	 * Add an operator to the end of composite. One problem is when the
	 * composite is empty, we have to specify prev variable because we can not
	 * get shared pointer from this.
	 * 
	 * @param oper Operator to be added.
	 * @param prev Operator, after which we should place the added one in
	 * iterator list. If not specified, it is assumed that children are not
	 * empty and that the last item is not a composite.
	 */
	virtual void push_back (UNUSED_PARAM const boost::shared_ptr<PdfOperator> oper, 
							UNUSED_PARAM boost::shared_ptr<PdfOperator> _prev = boost::shared_ptr<PdfOperator> ())
		{ throw NotImplementedException ("PdfOperator::push_back ()"); }

protected:
	/**
	 * Insert an operator after an item.
	 * 
	 * @param oper Operator after which an oper will be inserted.
	 * @param newOper Operator to be inserted.
	 */
	virtual void insert_after (const boost::shared_ptr<PdfOperator>, boost::shared_ptr<PdfOperator>)
		{ throw NotImplementedException ("PdfOperator::insert_after ()"); }

	/**
	 * Remove an operator from the composite interface.
	 *
	 * REMARK: This will not remove it from the Iterator list.
	 *
	 * @param op to be erased.
	 */
	virtual void remove (boost::shared_ptr<PdfOperator>)
		{ throw NotImplementedException ("PdfOperator::remove ()"); }

protected:	
	/**
	 * Put behind.
	 * 
	 * REMARK: We can't simply make putBehind a method, because we have to
	 * know the smart pointer behind the object. Member variable "this" is not
	 * sufficient.
	 *
	 * @param behindWhich Behind which elemnt.
	 * @param which 		Which element.
	 */
	static void putBehind (	boost::shared_ptr<PdfOperator> behindWhich, 
							boost::shared_ptr<PdfOperator> which);

	//
	// Composite interface
	//
public:
	/**
	 * Get children count.
	 *
	 * @return Number of children.
	 */
	virtual size_t getChildrenCount () const
		{ return 0; }
	
	/**
	 * Get childrens.
	 *
	 * @param opers Children container.
	 */
	virtual void getChildren (PdfOperators& opers) const
		{ opers.clear(); }

	
	//
	// Iterator interface
	//
public:
	/**
	 * Get iterator.
	 * 
	 * REMARK: We can't simply make getIterator a method, because we have to
	 * know the smart pointer behind the object. Member variable "this" is not
	 * sufficient.
	 *
	 * @param op Operator from which we want the iterator.
	 * @param forward Direction of traversing when looking for the first valid
	 * item.
	 * 
	 * @return Iterator.
	 */
	template<typename ITER>
	static ITER getIterator (boost::shared_ptr<PdfOperator> op, bool forward) 
		{ return ITER (ListItem (op),forward); }

	/** \copydoc getIterator. */
	template<typename ITER>
	static ITER getIterator (boost::shared_ptr<PdfOperator> op) 
		{ return ITER (ListItem (op)); }

	/** Create simple iterator. */
	static Iterator getIterator (boost::shared_ptr<PdfOperator> op) 
		{ return Iterator (ListItem (op)); }

	//
	// Clone interface
	//
public:
	/**
	 * Clone this object.
	 *
	 * @return New clone.
	 */
	virtual boost::shared_ptr<PdfOperator> clone () = 0;

	//
	// CContentStream interface
	//
public:
	/**
	 * Set content stream.
	 *
	 * Every pdf operator needs to know into which content stream it belongs.
	 * This is due to the fact, that some functions just return container of 
	 * pdfoperators, and if we want to work with them, we have to know the
	 * contentstream.
	 *
	 * We can find out the CStream but there is no mapping between CStream and
	 * content stream.
	 *
	 * @param cs Content stream.
	 */
	void setContentStream (CContentStream& cs)
		{ _contentstream = &cs; }
	
	
	/**
	 * Get content stream.
	 * @return Content stream that this pdfoperator belongs to.
	 */
	boost::shared_ptr<CContentStream> getContentStream () const;

	//
	// BBox
	//
private:
	/** Bounding box of this operator. */
	BBox _bbox;

public:
	/** 
	 * Set bounding box.
	 *
	 * @param rc Bounding box.
	 */
	void setBBox (const BBox& rc)
		{ _bbox = rc; }

	/**
	 * Get bounding box of this operator.
	 *
	 * @return Bounding box.
	 */
	BBox getBBox () const
		{ assert (BBox::isInitialized(_bbox)); return _bbox; }
	

	//
	// Observer interface
	//
public:
	/**
	 * Init operands and store observer for later unregistering.
	 */
	virtual void init_operands (boost::shared_ptr<observer::IObserver<IProperty> >, boost::weak_ptr<CPdf>, IndiRef*) = 0;

};	// class PdfOperator



//==========================================================
// CompositePdfOperator
//==========================================================

/**
 * Composite object.
 *
 * This is an implementation of Composite design pattern where leaves and
 * composites share the same interface.
 *
 * \see PdfOperator
 */
class CompositePdfOperator : public PdfOperator
{
	// Variables
protected:
	/** Child operator, where all calls are redirected. */
	PdfOperators _children;

	//
	// Ctor & Dtor
	//
protected:
	/** Constructor. */
	CompositePdfOperator () {}

	/** Destructor. */
	virtual ~CompositePdfOperator() {}

	//
	// Composite interface
	//
public:
	virtual size_t getChildrenCount () const {return _children.size ();}
	virtual void push_back (const boost::shared_ptr<PdfOperator> oper, 
							boost::shared_ptr<PdfOperator> _prev = boost::shared_ptr<PdfOperator> ());
	virtual void getChildren (PdfOperators& container) const;
protected:
	virtual void remove (boost::shared_ptr<PdfOperator> op);
	virtual void insert_after (const boost::shared_ptr<PdfOperator> oper, boost::shared_ptr<PdfOperator> newOper);

	//
	// PdfOperator interface
	//
public:
	virtual size_t getParametersCount () const {return 0;}
	virtual void getParameters (Operands&) const {}
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first) const = 0;
	virtual void init_operands (boost::shared_ptr<observer::IObserver<IProperty> > observer, boost::weak_ptr<CPdf> pdf, IndiRef* rf);

	//
	// Clone interface
	//
protected:
	virtual boost::shared_ptr<PdfOperator> clone () = 0;

}; // class CompositePdfOperator


//==========================================================
// Helper functions - is*
//==========================================================

//
// Specific operator
//
inline bool
isPdfOp (const PdfOperator& op, const std::string& opn)
{
	std::string tmp;
	op.getOperatorName (tmp);
	return opn == tmp;
}
inline bool
isPdfOp (const PdfOperator& op, 
		 const std::string& opn1, 
		 const std::string& opn2)
{ 
	std::string tmp;
	op.getOperatorName (tmp);
	return (tmp == opn1) || (tmp == opn2); 
}
inline bool
isPdfOp (const PdfOperator& op, 
		 const std::string& opn1, 
		 const std::string& opn2, 
		 const std::string& opn3)
{
	std::string tmp;
	op.getOperatorName (tmp);
	return (tmp == opn1) || (tmp == opn2) || (tmp == opn3); 
}
inline bool
isPdfOp (const PdfOperator& op, 
		 const std::string& opn1, 
		 const std::string& opn2, 
		 const std::string& opn3, 
		 const std::string& opn4)
{
	std::string tmp;
	op.getOperatorName (tmp);
	return (tmp == opn1) || (tmp == opn2) || (tmp == opn3) || (tmp == opn4);
}

/** Is specific operator. */
template<typename T>
inline bool
isPdfOp (const T& op, const std::string& opn)
	{ return isPdfOp (*op, opn); }
template<typename T>
inline bool
isPdfOp (const T& op, const std::string& opn1, const std::string& opn2)
	{ return isPdfOp (*op, opn1, opn2); }
template<typename T>
inline bool
isPdfOp (const T& op, const std::string& opn1, const std::string& opn2, const std::string& opn3)
	{ return isPdfOp (*op, opn1, opn2, opn3); }
template<typename T>
inline bool
isPdfOp (const T& op, const std::string& opn1, const std::string& opn2, const std::string& opn3, const std::string& opn4)
	{ return isPdfOp (*op, opn1, opn2, opn3, opn4); }



//==========================================================
// Helper functions -  other
//==========================================================

/**
 * Get last operator if a composite.
 *
 * @param oper Operator.
 *
 * @return Last operator of a composite.
 */
boost::shared_ptr<PdfOperator> getLastOperator (boost::shared_ptr<PdfOperator> oper);

/**
 * Get last operator if a composite.
 *
 * @param it Iterator.
 * 
 * @return Iterator over last operator of a composite.
 */
inline boost::shared_ptr<PdfOperator> getLastOperator (PdfOperator::Iterator it)
	{ return getLastOperator (it.getCurrent()); }


//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _PDFOPERATORSBASE_H_

