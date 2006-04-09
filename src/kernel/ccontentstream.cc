// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  ccontentstream.cc
 *         Created:  03/24/2006 10:33:34 PM CET
 *          Author:  jmisutka (), 
 * =====================================================================================
 *
 * \TODO: complex operators
 */

// static
#include "static.h"
#include "ccontentstream.h"

//
#include "pdfoperators.h"
#include "cobject.h"

//==========================================================
namespace pdfobjects {
//==========================================================

using namespace std;
using namespace boost;
using namespace debug;
using namespace utils;


//==========================================================
// Specialized classes representing operators
//==========================================================

namespace
{

	//
	// Actual state (position) updaters
	//
	void 
	unknownUpdate (GfxState&, const PdfOperator::Operands&)
	{
		printDbg (debug::DBG_DBG, "!");
	}
	
	//
	// Maximum operator arguments
	//
	const size_t MAX_OPERANDS = 6;
			
	//
	// Bit helper function
	//
	template<class T, class U>
	inline bool isBitSet(T value, U mask)
		{ 	// compare number of bits of where we want to store bit with the position 
			assert ((int)(sizeof(short) * 8) > mask);
			return (value & ((unsigned short)0x1 << mask)) != 0;}

	inline unsigned short setNoneBitsShort()
		{ return 0x0;}

	template<class U>
	inline unsigned short setNthBitsShort(U mask)
		{ 	// compare number of bits of where we want to store bit with the position 
			assert ((int)(sizeof(short) * 8) > mask);
			return ((unsigned short) 0x1 << mask);}

	template<class U>
	inline unsigned short setNthBitsShort(U mask, U mask1)
		{ return setNthBitsShort (mask) | setNthBitsShort (mask1); }
	template<class U>
	inline unsigned short setNthBitsShort(U mask, U mask1, U mask2)
		{ return setNthBitsShort (mask,mask1) | setNthBitsShort (mask2); }
	template<class U>
	inline unsigned short setNthBitsShort(U mask, U mask1, U mask2, U mask3)
		{ return setNthBitsShort (mask,mask1,mask2) | setNthBitsShort (mask3); }
	template<class U>
	inline unsigned short setNthBitsShort(U mask, U mask1, U mask2, U mask3, U mask4)
		{ return setNthBitsShort (mask,mask1,mask2,mask3) | setNthBitsShort (mask4); }


	//
	// Known operators, it is copied from pdf BECAUSE it is
	// a private member variable of a class and we do not have
	// access to it
	//
	typedef struct
	{
		const char 				name[4];				/** Operator name. */
		const size_t			argNum;					/** Number of arguments operator should get. */
		const unsigned short	types[MAX_OPERANDS];	/** Bits are representing what it should be. */
		void (*update) (GfxState& state, const PdfOperator::Operands& ops);/** Function to execute when updating position. */
		
	} CheckTypes;

	/**
	 * All known operators.
	 * 
	 */
	const CheckTypes KNOWN_OPERATORS[] = {
			{"",  3, 	{setNthBitsShort (pInt, pReal),
					 	setNthBitsShort (pInt, pReal),    
					 	setNthBitsShort (pString)}, unknownUpdate},	
/*			{"'", 1, 	setNthBitsShort (pString), unknownUpdate},	
			{"B", 0, 	setNoneBitsShort (), unknownUpdate},	
			{"B*", 0, 	setNoneBitsShort (), unknownUpdate},	
			{"BDC", 2, 	setNthBitsShort (pName),   
						setNthBitsShort (pDict, pName), unknownUpdate},	
			{"BI", 0, 	setNoneBitsShort (), unknownUpdate},	
			{"BMC", 1, 	setNthBitsShort (pName), unknownUpdate},	
			{"BT",  0, 	setNoneBitsShort (), unknownUpdate},	
			{"BX",  0, 	setNoneBitsShort (), unknownUpdate},	
			{"CS",  1, 	setNthBitsShort (pName), unknownUpdate},	
			{"DP",  2,	setNthBitsShort (pName),   
						setNthBitsShort (pDict, pName), unknownUpdate},	
			{"Do",  1, 	setNthBitsShort (pName), unknownUpdate},	
			{"EI",  0, 	setNoneBitsShort (), unknownUpdate},	
			{"EMC", 0, 	setNoneBitsShort (), unknownUpdate},	
			{"ET",  0, 	setNoneBitsShort (), unknownUpdate},	
			{"EX",  0, 	setNoneBitsShort (), unknownUpdate},	
			{"F",   0, 	setNoneBitsShort (), unknownUpdate},	
			{"G",   1, 	setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"ID",  0, 	setNoneBitsShort (), unknownUpdate},	
			{"J",   1, 	setNthBitsShort (pInt), unknownUpdate},	
			{"K",   4, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),
						setNthBitsShort (pInt, pReal),
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"M",   1, 	setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"MP",  1, 	setNthBitsShort (pName), unknownUpdate},	
			{"Q",   0, 	setNoneBitsShort (), unknownUpdate},	
			{"RG",  3, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"S",   0, 	setNoneBitsShort (), unknownUpdate},	
			{"SC",  4, setNthBitsShort (pInt, pReal),   
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
  			{"SCN", 5, setNthBitsShort (pInt, pReal, pName),   
						setNthBitsShort (pInt, pReal, pName),    
						setNthBitsShort (pInt, pReal, pName),    
						setNthBitsShort (pInt, pReal, pName),
						setNthBitsShort (pInt, pReal, pName), unknownUpdate},	
			{"T*",  0, 	setNoneBitsShort (), unknownUpdate},	
			{"TD",  2, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"TJ",  1, 	setNthBitsShort (pArray), unknownUpdate},	
			{"TL",  1, 	setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"Tc",  1, 	setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"Td",  2, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"Tf",  2, 	setNthBitsShort (pName),   
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"Tj",  1, 	setNthBitsShort (pString), unknownUpdate},	
			{"Tm",  6, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),
						setNthBitsShort (pInt, pReal),
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"Tr",  1, 	setNthBitsShort (pInt), unknownUpdate},	
			{"Ts",  1, 	setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"Tw",  1, 	setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"Tz",  1, 	setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"W",   0, 	setNoneBitsShort (), unknownUpdate},	
			{"W*",  0, 	setNoneBitsShort (), unknownUpdate},	
			{"b",   0, 	setNoneBitsShort (), unknownUpdate},	
			{"b*",  0, 	setNoneBitsShort (), unknownUpdate},	
  			{"c",   6, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),
						setNthBitsShort (pInt, pReal),
						setNthBitsShort (pInt, pReal), unknownUpdate},	
  			{"cm",  6, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),
						setNthBitsShort (pInt, pReal),
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"cs",  1, 	setNthBitsShort (pName), unknownUpdate},	
			{"d",   2, 	setNthBitsShort (pArray),  
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"d0",  2, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
  			{"d1",  6, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"f",   0, 	setNoneBitsShort (), unknownUpdate},	
			{"f*",  0, 	setNoneBitsShort (), unknownUpdate},	
			{"g",   1, 	setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"gs",  1, 	setNthBitsShort (pName), unknownUpdate},	
			{"h",   0, 	setNoneBitsShort (), unknownUpdate},	
			{"i",   1, 	setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"j",   1, 	setNthBitsShort (pInt), unknownUpdate},	
			{"k",   4, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"l",   2, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"m",   2, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"n",   0, 	setNoneBitsShort (), unknownUpdate},	
			{"q",   0, 	setNoneBitsShort (), unknownUpdate},	
			{"re",  4, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"rg",  3, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"ri",  1, 	setNthBitsShort (pName), unknownUpdate},	
			{"s",   0, 	setNoneBitsShort (), unknownUpdate},	
			{"sc",  4, setNthBitsShort (pInt, pReal),   
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
  			{"scn", 5, setNthBitsShort (pInt, pReal, pName),   
						setNthBitsShort (pInt, pReal, pName),    
						setNthBitsShort (pInt, pReal, pName),    
						setNthBitsShort (pInt, pReal, pName),    
						setNthBitsShort (pInt, pReal, pName), unknownUpdate},	
			{"sh",  1, 	setNthBitsShort (pName), unknownUpdate},	
			{"v",   4, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"w",   1, 	setNthBitsShort (pInt, pReal), unknownUpdate},	
			{"y",   4, 	setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), unknownUpdate},	
*/
		};

	/**
	 * Set pdf to operands.
	 *
	 * This is vital when changing those operands.
	 *
	 * @param pdf Pdf where operand will belong.
	 * @param rf  Indiref of content's stream parent.
	 * @param operands Operand stack.
	 * @param count Count of objects to bind.
	 */
	void
	operandsSetPdf (CPdf& pdf, IndiRef rf, PdfOperator::Operands& operands, int count = std::numeric_limits<int>::max())
	{
		PdfOperator::Operands::reverse_iterator it = operands.rbegin ();
		for (; (it != operands.rend ()) && (0 <= count); ++it, --count)
		{
			(*it)->setPdf (&pdf);
			(*it)->setIndiRef (rf);
		}
	}
	
	/**
	 * Check if the operands match the specification.
	 *
	 * @param ops Operator specification
	 * @param operands Operand stack.
	 *
	 * @return True if OK, false otherwise.
	 */
	bool check (const CheckTypes& ops, PdfOperator::Operands& operands)
	{
		string str;
		for (PdfOperator::Operands::iterator it = operands.begin();it != operands.end(); ++it)
		{
			string tmp;
			(*it)->getStringRepresentation (tmp);
			str += " " + tmp;
		}
		printDbg (DBG_DBG, "Operands: " << str);
		
		PdfOperator::Operands::reverse_iterator it = operands.rbegin ();
		//
		// Check arguments in reverse order
		//
		int j = ops.argNum - 1;
		for (size_t i = 0; i < ops.argNum; ++i, --j)
		{			
			assert (j >= 0);
  			if (!isBitSet(ops.types[j], (*it)->getType()))
			{
				printDbg (DBG_ERR, "Bad " << i << "-th operand type [" << (*it)->getType() << "] " << hex << " 0x" << ops.types[i]);
				return false;
			}
			// Next element
			++it;
		}

		return true;
	}

	/**
	 * Find the operator by its name.
	 *
	 * @param opName Name of the operator.
	 *
	 * @return Structure defining the operator.
	 */
	const CheckTypes*
	findOp (const string& opName)
	{
		int lo, hi, med, cmp;
		
		cmp = std::numeric_limits<int>::max ();
		lo = -1;
		hi = sizeof (KNOWN_OPERATORS) / sizeof (CheckTypes);
		
		printDbg (DBG_DBG, "size: " << hi );
		
		// 
		// dividing of interval
		// 
		while (hi - lo > 1) 
		{
			med = (lo + hi) / 2;
			cmp = opName.compare (KNOWN_OPERATORS[med].name);
			if (cmp > 0)
				lo = med;
			else if (cmp < 0)
				hi = med;
			else
				lo = hi = med;
		}

		if (0 == cmp)
			return &(KNOWN_OPERATORS[lo]);
		else
			return NULL;
	}
		
	/**
	 * Find the operator and create it. 
	 *
	 * Here is decided which implementation of a pdf operator is used.
	 *
	 * @param op String representation of the operator. It is used to find the operator and
	 * 			 sometimes to initialize it.
	 * @param operands Operand stack.
	 * @param pdf Pdf where it will belong
	 * @param rf  Id of the parent object.
	 * @param
	 *
	 * @return Created pdf operator.
	 */
	PdfOperator*
	createOp  ( const string& op, 
				PdfOperator::Operands& operands, 
				CPdf& pdf, 
				IndiRef rf, 
				boost::shared_ptr<PdfOperator>& )
	{
		printDbg (DBG_DBG, "Finding operator: " << op);

		// Try to find the op by its name
		const CheckTypes* chcktp = findOp (op);
		if (NULL == chcktp)
		{
			printDbg (DBG_DBG, "Operator not found.");

			// Set pdf to all operands
			operandsSetPdf (pdf, rf, operands);
			return new UnknownPdfOperator (operands, op);
		}
				
		assert (NULL != chcktp);
		printDbg (DBG_DBG, "Operator found. " << chcktp->name);

		if (!check (*chcktp, operands))
		{
			throw MalformedFormatExeption ("Content stream bad operator type. ");
		}
		else
		{
			// Set pdf to all operands
			operandsSetPdf (pdf, rf, operands, chcktp->argNum);
			// Result in lo
			return new SimpleGenericOperator (chcktp->name, chcktp->argNum, operands);
		}
		//
		// \TODO complex types
		//
	}	

	/**
	 * Parse the stream into small objects.
	 *
	 * @param operators Operator stack.
	 * @param obj 	Xpdf content stream.
	 * @param pdf 	Pdf where this content stream belongs (parent object)
	 * @param rf 	Id of parent object.
	 *
	 *
	 * \TODO 
	 */
	void
	parseContentStream (CContentStream::Operators& operators, 
						Object& obj, 		
						CPdf& pdf, 
						IndiRef rf)
	{
		assert (obj.isStream() || obj.isArray());
		
		//
		// Create the parser and lexer and get objects from it
		//
		scoped_ptr<Parser> parser (new Parser (NULL, new Lexer(NULL, &obj)));

		PdfOperator::Operands operands;
			
		Object o;
		parser->getObj(&o);

		boost::shared_ptr<PdfOperator> cmplex;
		//
		// Loop through all object, if it is an operator create pdfoperator else assume it is an operand
		//
		while (!o.isEOF()) 
		{
			if (o.isCmd ())
			{
				// Create operator
				boost::shared_ptr<PdfOperator> op  (createOp (string (o.getCmd ()), 
										operands, 
										pdf, rf,
										cmplex));
				//
				// Put it either to operators when the operator itself is a complex type
				// or if it is not a complex type
				// Put it behind complex type when the operand itself is not complex but we are in a
				// complex type (that is indicated by cmplex variable)
				//
				if (cmplex && cmplex != op)
				{
					PdfOperator::putBehind (cmplex, op);
				
				}else
				{
					if (!operators.empty())
					{
						operators.back ()->setNext (op);
						op->setPrev (operators.back ());
					}
					// Make it the last one
					operators.push_back (op);
				}
				
				assert (operands.empty());
				if (!operands.empty ())
					throw MalformedFormatExeption ( "CContentStream::CContentStream() "
									"Operands left on stack in pdf content stream after operator.");
					
			}else // if (o.isCmd ())
			{
				shared_ptr<IProperty> pIp (createObjFromXpdfObj (o));
				operands.push_back (pIp);
			}

			// free it else memory leak
			o.free ();
			// grab the next object
			parser->getObj(&o);
		}
	}
	

//==========================================================
} // namespace
//==========================================================


//==========================================================
namespace operatorparser {
//==========================================================

//
//
//
void 
adjustActualPosition (boost::shared_ptr<PdfOperator> op, GfxState& state)
{
	if (op)
	{
		// Get operator name
		string frst, lst;
		op->getOperatorName(frst, lst);
		// Get operator specification
		const CheckTypes* chcktp = findOp (frst);
		// Get operands
		PdfOperator::Operands ops;
		op->getParameters (ops);
		// If operator found use the function else use default
		if (NULL != chcktp)
		{
			assert (ops.size () == chcktp->argNum);
			(chcktp->update) (state, ops);
			
		}else
		{
			unknownUpdate (state, ops);
		}

	}else
		throw ElementBadTypeException ("adjustActualPosition()");
	
}
/*
	"m"	
	state->moveTo(args[0].getNum(), args[1].getNum());
	"l"
	state->lineTo(args[0].getNum(), args[1].getNum());
	"c"
	state->curveTo(x1, y1, x2, y2, x3, y3);

	"v"
	  x1 = state->getCurX();
  y1 = state->getCurY();
  x2 = args[0].getNum();
  y2 = args[1].getNum();
  x3 = args[2].getNum();
  y3 = args[3].getNum();
  state->curveTo(x1, y1, x2, y2, x3, y3);
  
  "y"
  x1 = args[0].getNum();
  y1 = args[1].getNum();
  x2 = args[2].getNum();
  y2 = args[3].getNum();
  x3 = x2;
  y3 = y2;
  state->curveTo(x1, y1, x2, y2, x3, y3);
  
  "re"
  state->moveTo(x, y);
  state->lineTo(x + w, y);
  state->lineTo(x + w, y + h);
  state->lineTo(x, y + h);
  state->closePath();
  
  "h"
 state->closePath();
	

Gfx.cc::#1478
  
  "BT"
  state->setTextMat(1, 0, 0, 1, 0, 0);
  state->textMoveTo(0, 0);

  "Tc"
  state->setCharSpace(args[0].getNum());

  "Tf"
  state->setFont(font, args[1].getNum());

  "Ts"
  text rise
  
  
  "Tw"
  state->setWordSpace(args[0].getNum());

  "Tz"
state->setHorizScaling(args[0].getNum());

  "Td"
  tx = state->getLineX() + args[0].getNum();
  ty = state->getLineY() + args[1].getNum();
  state->textMoveTo(tx, ty);

  "TD"
  tx = state->getLineX() + args[0].getNum();
  ty = args[1].getNum();
  state->setLeading(-ty);
  ty += state->getLineY();
  state->textMoveTo(tx, ty);
 


  "Tm"
 state->setTextMat(args[0].getNum(), args[1].getNum(),
		    args[2].getNum(), args[3].getNum(),
		    args[4].getNum(), args[5].getNum());
  state->textMoveTo(0, 0);
  out->updateTextMat(state);
  out->updateTextPos(state);
  
  "T*"
  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty)

  "'"
  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
  doShowText

  "TL"
  state->setLeading(args[0].getNum());

  "\"
  state->setWordSpace(args[0].getNum());
  state->setCharSpace(args[1].getNum());
  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
  doShowText
		  
  "TJ"
void Gfx::opShowSpaceText(Object args[], int numArgs) {
  Array *a;
  Object obj;
  int wMode;
  int i;

  if (!state->getFont()) {
    error(getPos(), "No font in show/space");
    return;
  }
  if (fontChanged) {
    out->updateFont(state);
    fontChanged = gFalse;
  }
  out->beginStringOp(state);
  wMode = state->getFont()->getWMode();
  a = args[0].getArray();
  for (i = 0; i < a->getLength(); ++i) {
    a->get(i, &obj);
    if (obj.isNum()) {
      // this uses the absolute value of the font size to match
      // Acrobat's behavior
      if (wMode) {
	state->textShift(0, -obj.getNum() * 0.001 *
			    fabs(state->getFontSize()));
      } else {
	state->textShift(-obj.getNum() * 0.001 *
			 fabs(state->getFontSize()), 0);
      }
      out->updateTextShift(state, obj.getNum());
    } else if (obj.isString()) {
      doShowText(obj.getString());
    } else {
      error(getPos(), "Element of show/space array must be number or string");
    }
    obj.free();
  }
  out->endStringOp(state);
}



"!!!"
void Gfx::doShowText(GString *s) {
  GfxFont *font;
  int wMode;
  double riseX, riseY;
  CharCode code;
  Unicode u[8];
  double x, y, dx, dy, dx2, dy2, curX, curY, tdx, tdy, lineX, lineY;
  double originX, originY, tOriginX, tOriginY;
  double oldCTM[6], newCTM[6];
  double *mat;
  Object charProc;
  Dict *resDict;
  Parser *oldParser;
  char *p;
  int len, n, uLen, nChars, nSpaces, i;

  font = state->getFont();
  wMode = font->getWMode();

  if (out->useDrawChar()) {
    out->beginString(state, s);
  }

  // handle a Type 3 char
  if (font->getType() == fontType3 && out->interpretType3Chars()) {
    mat = state->getCTM();
    for (i = 0; i < 6; ++i) {
      oldCTM[i] = mat[i];
    }
    mat = state->getTextMat();
    newCTM[0] = mat[0] * oldCTM[0] + mat[1] * oldCTM[2];
    newCTM[1] = mat[0] * oldCTM[1] + mat[1] * oldCTM[3];
    newCTM[2] = mat[2] * oldCTM[0] + mat[3] * oldCTM[2];
    newCTM[3] = mat[2] * oldCTM[1] + mat[3] * oldCTM[3];
    mat = font->getFontMatrix();
    newCTM[0] = mat[0] * newCTM[0] + mat[1] * newCTM[2];
    newCTM[1] = mat[0] * newCTM[1] + mat[1] * newCTM[3];
    newCTM[2] = mat[2] * newCTM[0] + mat[3] * newCTM[2];
    newCTM[3] = mat[2] * newCTM[1] + mat[3] * newCTM[3];
    newCTM[0] *= state->getFontSize();
    newCTM[1] *= state->getFontSize();
    newCTM[2] *= state->getFontSize();
    newCTM[3] *= state->getFontSize();
    newCTM[0] *= state->getHorizScaling();
    newCTM[2] *= state->getHorizScaling();
    state->textTransformDelta(0, state->getRise(), &riseX, &riseY);
    curX = state->getCurX();
    curY = state->getCurY();
    lineX = state->getLineX();
    lineY = state->getLineY();
    oldParser = parser;
    p = s->getCString();
    len = s->getLength();
    while (len > 0) {
      n = font->getNextChar(p, len, &code,
			    u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
			    &dx, &dy, &originX, &originY);
      dx = dx * state->getFontSize() + state->getCharSpace();
      if (n == 1 && *p == ' ') {
	dx += state->getWordSpace();
      }
      dx *= state->getHorizScaling();
      dy *= state->getFontSize();
      state->textTransformDelta(dx, dy, &tdx, &tdy);
      state->transform(curX + riseX, curY + riseY, &x, &y);
      saveState();
      state->setCTM(newCTM[0], newCTM[1], newCTM[2], newCTM[3], x, y);
      //~ out->updateCTM(???)
      if (!out->beginType3Char(state, curX + riseX, curY + riseY, tdx, tdy,
			       code, u, uLen)) {
	((Gfx8BitFont *)font)->getCharProc(code, &charProc);
	if ((resDict = ((Gfx8BitFont *)font)->getResources())) {
	  pushResources(resDict);
	}
	if (charProc.isStream()) {
	  display(&charProc, gFalse);
	} else {
	  error(getPos(), "Missing or bad Type3 CharProc entry");
	}
	out->endType3Char(state);
	if (resDict) {
	  popResources();
	}
	charProc.free();
      }
      restoreState();
      // GfxState::restore() does *not* restore the current position,
      // so we deal with it here using (curX, curY) and (lineX, lineY)
      curX += tdx;
      curY += tdy;
      state->moveTo(curX, curY);
      state->textSetPos(lineX, lineY);
      p += n;
      len -= n;
    }
    parser = oldParser;

  } else if (out->useDrawChar()) {
    state->textTransformDelta(0, state->getRise(), &riseX, &riseY);
    p = s->getCString();
    len = s->getLength();
    while (len > 0) {
      n = font->getNextChar(p, len, &code,
			    u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
			    &dx, &dy, &originX, &originY);
      if (wMode) {
	dx *= state->getFontSize();
	dy = dy * state->getFontSize() + state->getCharSpace();
	if (n == 1 && *p == ' ') {
	  dy += state->getWordSpace();
	}
      } else {
	dx = dx * state->getFontSize() + state->getCharSpace();
	if (n == 1 && *p == ' ') {
	  dx += state->getWordSpace();
	}
	dx *= state->getHorizScaling();
	dy *= state->getFontSize();
      }
      state->textTransformDelta(dx, dy, &tdx, &tdy);
      originX *= state->getFontSize();
      originY *= state->getFontSize();
      state->textTransformDelta(originX, originY, &tOriginX, &tOriginY);
      out->drawChar(state, state->getCurX() + riseX, state->getCurY() + riseY,
		    tdx, tdy, tOriginX, tOriginY, code, n, u, uLen);
      state->shift(tdx, tdy);
      p += n;
      len -= n;
    }

  } else {
    dx = dy = 0;
    p = s->getCString();
    len = s->getLength();
    nChars = nSpaces = 0;
    while (len > 0) {
      n = font->getNextChar(p, len, &code,
			    u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
			    &dx2, &dy2, &originX, &originY);
      dx += dx2;
      dy += dy2;
      if (n == 1 && *p == ' ') {
	++nSpaces;
      }
      ++nChars;
      p += n;
      len -= n;
    }
    if (wMode) {
      dx *= state->getFontSize();
      dy = dy * state->getFontSize()
	   + nChars * state->getCharSpace()
	   + nSpaces * state->getWordSpace();
    } else {
      dx = dx * state->getFontSize()
	   + nChars * state->getCharSpace()
	   + nSpaces * state->getWordSpace();
      dx *= state->getHorizScaling();
      dy *= state->getFontSize();
    }
    state->textTransformDelta(dx, dy, &tdx, &tdy);
    out->drawString(state, s);
    state->shift(tdx, tdy);
  }

  if (out->useDrawChar()) {
    out->endString(state);
  }

  updateLevel += 10 * s->getLength();
}

  "Tj"
 doShowText(args[0].getString());

 
 \TODO 
 XOBJECTS
*/

//==========================================================
} // namespace operatorparser
//==========================================================

//
//
//
CContentStream::CContentStream (shared_ptr<IProperty> stream, Object* obj)
{
	// not implemented yet
	assert (obj != NULL);
	if (pStream != stream->getType() || NULL == obj)
		throw CObjInvalidObject (); 
	printDbg (DBG_DBG, "Creating content stream.");

	// Check if stream is in a pdf, if not is NOT IMPLEMENTED
	// the problem is with parsed pdfoperators
	assert (NULL != stream->getPdf ());
	
	// Save stream
	contentstreams.push_back (stream);

	// Parse it into small objects
	parseContentStream (operators, *obj, *(stream->getPdf ()), stream->getIndiRef());
}

//
// Parse the xpdf object, representing the content stream
//
CContentStream::CContentStream (ContentStreams& streams, Object* obj)
{
	// not implemented yet
	assert (obj != NULL);
	if (NULL == obj)
		throw CObjInvalidObject (); 
	for (ContentStreams::iterator it = streams.begin(); it != streams.end (); ++it)
	{
		if (pStream != (*it)->getType())
			throw CObjInvalidObject (); 
		// Check if stream is in a pdf, if not is NOT IMPLEMENTED
		// the problem is with parsed pdfoperators
		assert (NULL != (*it)->getPdf());
	}
	printDbg (DBG_DBG, "Creating content stream.");
	
	// Save content streams
	copy (streams.begin(), streams.end(), back_inserter (contentstreams));

	// Parse it into small objects
	parseContentStream (operators, *obj, *(streams.front()->getPdf ()), streams.front()->getIndiRef ());
}

//
// 
//
void
CContentStream::getStringRepresentation (string& str) const
{
	printDbg (DBG_DBG, " ()");
	string frst, lst, tmp;

	str.clear ();
	for (Operators::const_iterator it = operators.begin (); it != operators.end(); ++it)
	{
			
		(*it)->getOperatorName (frst, lst);
		printDbg (DBG_DBG, "Operator name: " << frst << " " << lst << " param count: " << (*it)->getParametersCount() );
		
		(*it)->getStringRepresentation (tmp);
		str += tmp + "\n";
		tmp.clear ();
	}
}


//==========================================================
} // namespace pdfobjects
//==========================================================
