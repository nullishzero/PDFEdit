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

// static
#include "kernel/static.h"
#include "kernel/pdfoperators.h"
//
#include "kernel/iproperty.h"
#include "kernel/cinlineimage.h"

#include "kernel/ccontentstream.h"
#include "kernel/stateupdater.h"

//==========================================================
namespace pdfobjects {
//==========================================================

using namespace std;
using namespace boost;
using namespace debug;
using namespace utils;


//==========================================================
// Concrete implementations of PdfOperator
//==========================================================

//
//
//
SimpleGenericOperator::SimpleGenericOperator (const char* opTxt, 
											  const size_t numOper, 
											  Operands& opers) : _opText (opTxt)
{
		//utilsPrintDbg (debug::DBG_DBG, "Operator [" << opTxt << "] Operand size: " << numOper << " got " << opers.size());
		assert (numOper >= opers.size());
		if (numOper < opers.size())
			throw MalformedFormatExeption ("Operator operand size mismatch.");

	//
	// Store the operands and remove it from the stack
	// REMARK: the op count can vary ("scn" operator takes arbitrary number of
	// parameters)
	//
	for (size_t i = 0; (i < numOper) && !opers.empty(); ++i)
	{
		Operands::value_type val = opers.back ();
		// Store the last element of input parameter
		_operands.push_front (val);
		// Remove the element from input parameter
		opers.pop_back ();
	}
}

//
//
//
SimpleGenericOperator::SimpleGenericOperator (const std::string& opTxt, 
											  Operands& opers): _opText (opTxt)
{
		utilsPrintDbg (debug::DBG_DBG, opTxt);
	//
	// Store the operands and remove it from opers
	//
	while (!opers.empty())
	{
		// Store the last element of input parameter
		_operands.push_front ( opers.back() );
		// Remove the element from input parameter
		opers.pop_back ();
	}
}

//
//
//
SimpleGenericOperator::~SimpleGenericOperator() 
{
		// can happen when used as a temporary object
		if (0 < _operands.size() && !(_operandobserver)) 
			return;
	for (Operands::iterator it = _operands.begin(); it != _operands.end(); ++it) {
		UNREGISTER_SHAREDPTR_OBSERVER ((*it), _operandobserver);
	}
}


//
//
//
void 
SimpleGenericOperator::getStringRepresentation (std::string& str) const
{
	std::string tmp;
	for (Operands::const_iterator it = _operands.begin(); it != _operands.end (); ++it)
	{
		tmp.clear ();
		(*it)->getStringRepresentation (tmp);
		str += tmp + " ";
	}

	// Add operator string
	str += _opText;
}
	

//
//
//
shared_ptr<PdfOperator> 
SimpleGenericOperator::clone ()
{
	// Clone operands
	Operands ops;
	for (Operands::iterator it = _operands.begin (); it != _operands.end(); ++it)
		ops.push_back ((*it)->clone());
	assert (ops.size () == _operands.size());

	// Create clone
	return createOperator (_opText,ops);
}


void 
SimpleGenericOperator::init_operands (shared_ptr<observer::IObserver<IProperty> > observer, 
									  boost::weak_ptr<CPdf> pdf, 
									  IndiRef* rf)
{ 
	// store observer
	_operandobserver = observer; 
	//
	for (Operands::iterator oper = _operands.begin (); oper != _operands.end (); ++oper)
	{
		if (hasValidPdf(*oper))
		{ // We do not support adding operators from another stream
			if ( ((*oper)->getPdf().lock() != pdf.lock()) || !((*oper)->getIndiRef() == *rf) )
			{
				kernelPrintDbg (debug::DBG_ERR, "Pdf or indiref do not match: want " << *rf <<  " op has" <<(*oper)->getIndiRef());
				throw CObjInvalidObject ();
			}
			
		}else
		{
			(*oper)->setPdf (pdf);
			(*oper)->setIndiRef (*rf);
			REGISTER_SHAREDPTR_OBSERVER((*oper), observer);
			(*oper)->lockChange ();
		}
	} // for
}


void TextSimpleOperator::getRawText(std::string& str)const
{
using namespace utils;
	utilsPrintDbg(debug::DBG_DBG, "");
	std::string name, rawStr;
	getOperatorName(name);
	Operands ops;
	getParameters(ops);
	if(name == "'" || name == "Tj")
	{
		if(ops.size() != 1 || !isString(ops[0]))
		{
			utilsPrintDbg(debug::DBG_WARN, "Bad operands for operator "
					<<name<<" count="<<ops.size()
					<<" ops[0] type="<< ops[0]->getType());
			return;
		}
		rawStr = getStringFromIProperty(ops[0]);
	}
	else if (name == "\"")
	{
		if(ops.size() != 3 || !isArray(ops[2]))
		{
			utilsPrintDbg(debug::DBG_WARN, "Bad operands for operator "
					<<name<<" count="<<ops.size()
					<<" ops[2] type="<< ops[2]->getType());
			return;
		}
		rawStr = getStringFromIProperty(ops[2]);
	}
	else if (name == "TJ")
	{
		shared_ptr<IProperty> op = ops[0];
		if (!isArray(op) || ops.size() != 1)
		{
			utilsPrintDbg(debug::DBG_WARN, "Bad operands for TJ operator: ops[type="
					<< op->getType() <<" size="<<ops.size()<<"]");
			return;
		}
		shared_ptr<CArray> opArray = IProperty::getSmartCObjectPtr<CArray>(op);
		std::vector<shared_ptr<IProperty> > props;
		opArray->_getAllChildObjects(props);
		std::vector<shared_ptr<IProperty> >::iterator i;
		for(i=props.begin(); i!=props.end(); ++i)
		{
			shared_ptr<IProperty> p = *i;

			// TODO consider spacing coming from values
			if(!(isString(p)))
				continue;
			rawStr += getStringFromIProperty(p);
		}

	}else
	{
		utilsPrintDbg(debug::DBG_WARN, "Bad operator name="<<name);
		return;
	}

	str = rawStr;
}

void 
TextSimpleOperator::setRawText (std::string& str)
{
		utilsPrintDbg(debug::DBG_DBG, "");
	
	std::string name;
	getOperatorName(name);

	Operands ops;
	getParameters(ops);
	if(name == "'" || name == "Tj")
	{
			if(ops.size() != 1 || !isString(ops[0]))
			{
				utilsPrintDbg(debug::DBG_WARN, "Bad operands for operator " <<name<<" count="<<ops.size()<<" ops[0] type="<< ops[0]->getType());
				return;
			}
		setValueToSimple<CString, pString>(ops[0], str);
	}
	else if (name == "\"")
	{
			if(ops.size() != 3 || !isArray(ops[2]))
			{
				utilsPrintDbg(debug::DBG_WARN, "Bad operands for operator "<<name<<" count="<<ops.size()<<" ops[2] type="<< ops[2]->getType());
				return;
			}
		setValueToSimple<CString, pString>(ops[2], str);
	}
	else if (name == "TJ")
	{
		shared_ptr<IProperty> op = ops[0];
			if (!isArray(op) || ops.size() != 1)
			{
				utilsPrintDbg(debug::DBG_WARN, "Bad operands for TJ operator: ops[type="<< op->getType() <<" size="<<ops.size()<<"]");
				return;
			}
		// am to lazy to do it
		utilsPrintDbg(debug::DBG_WARN, "todo!");

	}else
	{
		utilsPrintDbg(debug::DBG_WARN, "Bad operator name="<<name);
		return;
	}
}


/** Simple class for font data encapsulation.
 */
class TextSimpleOperator::FontData 
{
	string fontName;
	string fontTag;
public:
	FontData(GfxFont* font)
	{
		if (font->getName() && font->getName()->getCString())
			fontName = font->getName()->getCString();
		if (font->getTag() && font->getTag()->getCString())
			fontTag = font->getTag()->getCString();
	}

	const char * getFontName()const
	{
		return fontName.c_str();
	}

	const char * getFontTag()const
	{
		return fontTag.c_str();
	}
};

GfxFont* TextSimpleOperator::getCurrentFont()const
{
	assert(fontData);
	const char* tag = fontData->getFontTag();
	shared_ptr<GfxResources> res = getContentStream()->getResources(); 
	GfxFont* font = res->lookupFont(tag);
	if(!font)
		utilsPrintDbg(debug::DBG_ERR, "Unable to get font(name="
				<<fontData->getFontName()
				<<", tag="<<fontData->getFontTag()
				<<") for operator");
	return font;
}

void TextSimpleOperator::getFontText(std::string& str)const
{
 	std::string rawStr;
	getRawText(rawStr);

 	int len = rawStr.size();
 	GString raw(rawStr.c_str(), len);
 	GfxFont* font = getCurrentFont();
	if(!font)
		return;
	utilsPrintDbg(debug::DBG_INFO, "Textoperator uses font="<<fontData->getFontName());
 	CharCode code;
 	Unicode u;
 	int uLen;
 	double dx, dy, originX, originY;
 	char * p=raw.getCString();
 	while(len>0)
 	{
 		int n = font->getNextChar(p, len, &code, &u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
 			    &dx, &dy, &originX, &originY);
 		for (int i=0; i<uLen; ++i)
 			str += (&u)[i];
 		p += n;
 		len -= n;
  	}
}

TextSimpleOperator::~TextSimpleOperator()
{
	if(fontData)
		delete fontData;
}

const char* TextSimpleOperator::getFontName()const
{
	assert(fontData);
	return fontData->getFontName();
}

void TextSimpleOperator::setFontData(GfxFont* gfxFont)
{
	assert(gfxFont);
	if (!gfxFont)
	{
		utilsPrintDbg(debug::DBG_ERR, "Null font encountered");
		return;
	}
	if(fontData)
		delete fontData;
	fontData = new FontData(gfxFont);
}

//==========================================================
// Concrete implementations of CompositePdfOperator
//==========================================================

//
// UnknownCompositePdfOperator
//

//
//
//
UnknownCompositePdfOperator::UnknownCompositePdfOperator 
	(const char* opBegin, const char* opEnd) : CompositePdfOperator (), _opBegin (opBegin), _opEnd (opEnd)
{
	utilsPrintDbg (DBG_DBG, "Unknown composite operator: " << _opBegin << " " << _opEnd);

}

//
//
//
void
UnknownCompositePdfOperator::getStringRepresentation (string& str) const
{
	// Header
	str += _opBegin; str += " ";
	
	// Delegate
	CompositePdfOperator::getStringRepresentation (str);	
}

//
//
//
shared_ptr<PdfOperator> 
UnknownCompositePdfOperator::clone ()
{
	shared_ptr<UnknownCompositePdfOperator> clone (new UnknownCompositePdfOperator(_opBegin,_opEnd));

	for (PdfOperators::iterator it = _children.begin(); it != _children.end(); ++it)
		clone->push_back ((*it)->clone(),getLastOperator(clone));
	
	// Create clone
	return clone;
}

//
// InlineImageCompositePdfOperator
//

//
//
//
InlineImageCompositePdfOperator::InlineImageCompositePdfOperator 
	(boost::shared_ptr<CInlineImage> im, const char* opBegin, const char* opEnd) 
		: CompositePdfOperator (), _opBegin (opBegin), _opEnd (opEnd), _inlineimage (im)
{
	utilsPrintDbg (DBG_DBG, _opBegin << " " << _opEnd);
}

//
//
//
void
InlineImageCompositePdfOperator::getStringRepresentation (string& str) const
{
	// BI % Begin inline image object
	// /W 17 % Width in samples
	// /H 17 % Height in samples
	// /CS /RGB % Color space
	// /BPC 8 % Bits per component
	// /F [/A85 /LZW] % Filters
	// ID % Begin image data
	// J1/gKA>.]AN&J?]-<HW]aRVcg*bb.\eKAdVV%/PcZ
	// …Omitted data…
	// R.s(4KE3&d&7hb*7[%Ct2HCqC~>
	// EI

	// Header
	str += _opBegin; str += "\n";
	// 
	if (_inlineimage)
	{
		std::string tmp;
		_inlineimage->getStringRepresentation (tmp);	
		str += tmp;
	}else
	{
		assert (!"Bad inline image.");
		throw CObjInvalidObject ();
	}
	// Footer
	str += _opEnd; str += "\n";

}

//
//
//
void
InlineImageCompositePdfOperator::getParameters (Operands& opers) const
{
	boost::shared_ptr<IProperty> ip = _inlineimage;
	opers.push_back (ip);
}

//
//
//
shared_ptr<PdfOperator> 
InlineImageCompositePdfOperator::clone ()
{
	// Clone operands
	shared_ptr<CInlineImage> imgclone = IProperty::getSmartCObjectPtr<CInlineImage> (_inlineimage->clone());
	// Create clone
	return shared_ptr<PdfOperator> (new InlineImageCompositePdfOperator (imgclone, _opBegin, _opEnd));
}


//==========================================================
// Helper funcions
//==========================================================

boost::shared_ptr<PdfOperator> createOperator(const std::string& name, PdfOperator::Operands& operands)
{
	if (name == "BI")
		throw NotImplementedException("Inline images not implemented here");

	// Try to find the op by its name
	const StateUpdater::CheckTypes* chcktp = StateUpdater::findOp (name.c_str());
	// Operator not found, create unknown operator
	if (NULL == chcktp)
		return shared_ptr<PdfOperator> (new SimpleGenericOperator (name ,operands));
	
	assert (chcktp);
	utilsPrintDbg (DBG_DBG, "Operator found. " << chcktp->name);
	// Check the type against specification
	// 
	if (!checkAndFixOperator (*chcktp, operands))
	{
		//assert (!"Content stream bad operator type.");
		throw ElementBadTypeException ("Content stream operator has incorrect operand type.");
	}
	
	// Get operands count
	size_t argNum = static_cast<size_t> ((chcktp->argNum > 0) ? chcktp->argNum : -chcktp->argNum);

	//
	// If endTag is "" it is a simple operator, composite otherwise
	// 
	if (isTextOp(*chcktp))
		return shared_ptr<PdfOperator> (new TextSimpleOperator(chcktp->name, argNum, operands));

	if (isSimpleOp(*chcktp))
		return shared_ptr<PdfOperator> (new SimpleGenericOperator (chcktp->name, argNum, operands));
		
	// Composite operator
	return shared_ptr<PdfOperator> (new UnknownCompositePdfOperator (chcktp->name, chcktp->endTag));

}

boost::shared_ptr<PdfOperator> createOperator(const char *name, PdfOperator::Operands& operands)
{
	std::string n = name;
	return createOperator(n, operands);
}

boost::shared_ptr<PdfOperator> createOperatorTranslation (double x, double y) 
{
	PdfOperator::Operands ops;
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (1)));
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (0)));
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (0)));
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (1)));
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (x)));
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (y)));
	return createOperator("cm", ops);
}


boost::shared_ptr<PdfOperator> createOperatorScale (double width, double height) 
{
	PdfOperator::Operands ops;
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (width)));
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (0)));
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (0)));
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (height)));
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (0)));
	ops.push_back (boost::shared_ptr<IProperty>(new CReal (0)));
	return createOperator("cm", ops);
}

//
//\todo improve performance
//
boost::shared_ptr<CompositePdfOperator>
findCompositeOfPdfOperator (PdfOperator::Iterator it, boost::shared_ptr<PdfOperator> oper)
{
	boost::shared_ptr<CompositePdfOperator> composite;
	typedef PdfOperator::PdfOperators Opers;
	Opers opers;


	while (!it.isEnd())
	{
		// Have we found what we were looking for
		if (isCompositeOp(it))
		{
			opers.clear ();
			it.getCurrent()->getChildren (opers);
			if (opers.end() != std::find (opers.begin(), opers.end(), oper))
				return boost::dynamic_pointer_cast<CompositePdfOperator, PdfOperator> (it.getCurrent());
		}else
		{
			// This can happen only in the "first level" but that should be
			// handled in caller
			if (it.getCurrent() == oper)
			{
				assert (!"Found highest level operator, that should be handled in the caller of this function.");
				throw CObjInvalidObject ();
			}
		}

		it.next ();
	}

	//
	// We should have found the operator
	// -- this can happen in an incorrect script
	// 		that remembers reference to a removed object
	// 
	//assert (!"Operator not found...");
	throw CObjInvalidOperation ();
}


	
//==========================================================
} // namespace pdfobjects
//==========================================================
