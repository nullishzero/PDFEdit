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

#ifndef _PDFOPERATORS_H_
#define _PDFOPERATORS_H_

// static includes
#include "kernel/pdfoperatorsbase.h"

//==========================================================
namespace pdfobjects {
//==========================================================


//==========================================================
// Concrete implementations of PdfOperator
//==========================================================

/**
 * Almost all simple (non composite) operators will be constructed by specifying number of operands, operands and the
 * text representation of the operator.
 *
 * This is an implementation of Composite design pattern where leaves and
 * composites share the same interface.
 *
 * \see PdfOperator

 */
class SimpleGenericOperator : public PdfOperator
{
private:
	/** Operands. */
	Operands _operands;
	/** Text representing the operator. */
	const std::string _opText;
	/** Operand observers registered on its operands. */
	boost::shared_ptr<observer::IObserver<IProperty> > _operandobserver;
	
public:

	/** 
	 * Constructor. 
	 * Create it as a standalone object. Prev and Next are not valid.
	 *
	 * @param opTxt Operator name text representation.
	 * @param numOper (Maximum) Number of operands.
	 * @param opers This is a stack of operands from which we take number specified
	 * 				by numOper or while any operand left.
	 */
	SimpleGenericOperator (const char* opTxt, const size_t numOper, Operands& opers);
	SimpleGenericOperator (const std::string& opTxt, Operands& opers);

	
	//
	// PdfOperator interface
	//
public:

	virtual size_t getParametersCount () const
		{ return _operands.size (); }

	virtual void getParameters (Operands& container) const
		{ copy (_operands.begin(), _operands.end(), back_inserter(container)); }

	virtual void getOperatorName (std::string& first) const
		{ first = _opText;}
	
	virtual void getStringRepresentation (std::string& str) const;

	//
	// Clone interface
	//
protected:
	virtual boost::shared_ptr<PdfOperator> clone ();

	
	//
	// Observer interface
	//
public:
	void init_operands (boost::shared_ptr<observer::IObserver<IProperty> > observer, boost::weak_ptr<CPdf> pdf, IndiRef* rf);

	//
	// Destructor
	//
public:
	
	/**
	 * Destructor.
	 */
	virtual ~SimpleGenericOperator();

}; // class SimpleGenericOperator


/** Text dedicated operator class.
 * This class represents those text operators which contains text to be 
 * displayed. This is necessary, because text string stored in operator's
 * operands is not the same as the displayed one in general and may be 
 * affected by font encoding. 
 * <br>
 * Use getFontText method to retreive text from text operator filtered 
 * through its font code maps.
 */
class TextSimpleOperator: public SimpleGenericOperator
{
	// forward declaration
	class FontData;

	/** Font data for later identification of associated font
	 */
	FontData* fontData;
protected:
	/** Finds current font for operator from fontName.
	 * Uses resources from content stream to retriev font by name.
	 * Returned instance must not be deallocated by caller.
	 * @return Font instance for this operator.
	 */
	GfxFont* getCurrentFont()const;
public:
	TextSimpleOperator (const char* opTxt, const size_t numOper, Operands& opers)
		:SimpleGenericOperator(opTxt, numOper, opers), fontData(NULL) {}
	TextSimpleOperator(const std::string& opTxt, Operands& opers)
		:SimpleGenericOperator(opTxt, opers), fontData(NULL) {}

	virtual ~TextSimpleOperator();
	
	/** Returns string represented by this text operator in raw format.
	 * Raw format doesn't take care about font used for this operator.
	 * @param str String to be set.
	 */
	virtual void getRawText(std::string& str)const;

	/** 
	 * Sets string which should be displayed according to the used font.
	 *
	 * Method will do the lowlevel transformation to the proper characters
	 * codes according to font and its encoding.
	 */
	void setFontText (const std::string& str);

	/** Returns string represented by this text operator converted 
	 * according the font encoding.
	 * @param str String to be set.
	 */
	virtual void getFontText(std::string& str)const;

	/** Sets font specific stuff.
	 * This method should be called from StateUpdater when we do know the 
	 * current font for this operator.
	 * <br>
	 * This method doesn't influence operator itself (or its operands).
	 * @param gfxFont Xpdf GfxFont instance.
	 */
	void setFontData(GfxFont* gfxFont);

	/** Returns font name for this operator.
	 * May return null if setFontData hasn't been called yet.
	 * @return Font name or NULL if not initialized yet.
	 */
	const char* getFontName()const;

}; // class TextSimpleOperator

//==========================================================
// Concrete implementations of CompositePdfOperator
//==========================================================


/**
 * Unknown composite operator which is constructed from a begin and end tag.
 *
 * This is an implementation of Composite design pattern where leaves and
 * composites share the same interface.
 * 
 * \see PdfOperator, CompositePdfOperator
 */
class UnknownCompositePdfOperator : public CompositePdfOperator
{
private:
		
	/** Text representing the beginning operator. */
	const char* _opBegin;
	/** Text representing the ending operator. */
	const char* _opEnd;

public:
	
	/** 
	 * Constructor. 
	 * Create it as a standalone object. Prev and Next are not valid.
	 *
	 * @param opBegin_ Start operator name text representation.
	 * @param opEnd_ End operator name text representation.
	 */
	UnknownCompositePdfOperator (const char* opBegin, const char* opEnd);

public:
	// End operator is added to composite as normal operator so just prepand start operator
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first) const {first = _opBegin;}

	//
	// Clone interface
	//
protected:
	virtual boost::shared_ptr<PdfOperator> clone ();

};


//
// Forward declaration
//
class CInlineImage;

/**
 * Inline image pdf operator. This is not a real composite but we can represent
 * it as one. This object is special. It is different than
 * every other pdf operator.
 *
 * This is an implementation of the Composite design pattern where leaves and
 * composites share the same interface.
 * 
 * \see PdfOperator, CompositePdfOperator
 */
class InlineImageCompositePdfOperator : public CompositePdfOperator
{
private:
		
	/** Text representing the beginning operator. */
	const char* _opBegin;
	/** Text representing the ending operator. */
	const char* _opEnd;
	/** Stream representing inline image. */
	boost::shared_ptr<CInlineImage> _inlineimage;

public:
	
	/** 
	 * Constructor. 
	 * Create it as a standalone object. Prev and Next are not valid.
	 *
	 * @param opBegin_ Start operator name text representation.
	 * @param opEnd_ End operator name text representation.
	 * @param im_ Stream representing inline image.
	 */
	InlineImageCompositePdfOperator (boost::shared_ptr<CInlineImage> im, const char* opBegin = "BI", const char* opEnd = "EI");

	
	//
	// PdfOperator interface
	//
public:
	virtual size_t getParametersCount () const {return 1;}
	virtual void getParameters (Operands& opers) const;
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first) const {first = _opBegin;}

	//
	//
	//
	size_t getWidth() const;
	size_t getHeight() const;

	//
	// Clone interface
	//
protected:
	virtual boost::shared_ptr<PdfOperator> clone ();

};


//==========================================================
// Helper funcions - general
//==========================================================

/** Factory function for operators creation.
 * Creates instance depending on type of the operator.
 * <br>
 * Note that this function doesn't cover inline images (BI operator).
 * @param name Opertor name.
 * @param operands Operands for operator.
 * @return Valid pdfoperator object.
 * @throw ElementBadTypeException if operator or its operands are not valid.
 * @throw NotImplementedException if given operator is inline image (BI).
 */
boost::shared_ptr<PdfOperator> createOperator(const std::string& name, PdfOperator::Operands& operands);

/** Factory function for operators creation.
 * Transforms const char parameter to the string and delegates to 
 * createOperator(std::string&, PdfOperator::Operands&)
 * @param name Opertor name.
 * @param operands Operands for operator.
 * @return Valid pdfoperator object.
 * @throw ElementBadTypeException if operator or its operands are not valid.
 * @throw NotImplementedException if given operator is inline image (BI).
 */
boost::shared_ptr<PdfOperator> createOperator(const char *name, PdfOperator::Operands& operands);

/** 
 * Create translation operator.
 */
boost::shared_ptr<PdfOperator> createOperatorTranslation (double x, double y);

/** 
 * Create scaling operator.
 */
boost::shared_ptr<PdfOperator> createOperatorScale (double width, double height);

/** Create a text operator with the font encoding translation.
 * @param cc Content stream wehere the operator will be added (this is not done by this 
 *           function - we just reuse resources from it).
 * @param fontName Font for the text.
 * @param op Text operator to use (tj, ' or TJ).
 * @param text Text to be used for the operator (unicode encoded).
 * @throw ElementBadTypeException if an unknown text operator is provided.
 * @throw NotImplementedException for " text operator.
 */
boost::shared_ptr<PdfOperator> createOperatorText (boost::shared_ptr<CContentStream>& cc,
                const std::string &fontName, const std::string &op, const std::string &text);

/** Is an operator a composite. */
inline bool 
isCompositeOp (const PdfOperator* oper)
{
	const CompositePdfOperator* compo = dynamic_cast<const CompositePdfOperator*> (oper);
	return (NULL == compo) ? false : true;
}

/** Is an operator a composite. */
inline bool 
isCompositeOp (PdfOperator::Iterator it)
	{ return isCompositeOp (it.getCurrent().get()); }

/** Is an operator a composite. */
inline bool 
isCompositeOp (boost::shared_ptr<PdfOperator> oper)
	{ return isCompositeOp (oper.get()); }


/** Is an operator an inline image. */
inline bool 
isInlineImageOp (const PdfOperator* oper)
{
	const InlineImageCompositePdfOperator* img = dynamic_cast<const InlineImageCompositePdfOperator*> (oper);
	return (NULL == img) ? false : true;
}
/** Is an operator an inline image. */
inline bool 
isInlineImageOp (boost::shared_ptr<PdfOperator> oper)
	{ return isInlineImageOp (oper.get()); }


/**
 * Find composite operator into which an operator belongs.
 *
 * @param begin Start iterator.
 * @param oper Object we want to find the composite in which it resides.
 *
 * @return Composite operator.
 */
boost::shared_ptr<CompositePdfOperator> findCompositeOfPdfOperator (PdfOperator::Iterator begin, 
									boost::shared_ptr<PdfOperator> oper);


//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _PDFOPERATORS_H_
