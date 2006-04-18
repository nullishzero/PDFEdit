// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  ccontentstream.cc
 *         Created:  03/24/2006 10:33:34 PM CET
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

// static
#include "static.h"
#include "ccontentstream.h"

//
#include "pdfoperators.h"
#include "cobjecthelpers.h"
#include "cobject.h"

//fabs
#include <math.h>

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

	
	//==========================================================
	// Actual state (position) updaters
	//==========================================================
	
	//
	void
	printTextUpdate (GfxState& state, const std::string& txt)
	{
		GfxFont *font;
		int wMode;
		double riseX, riseY;
		CharCode code;
		Unicode u[8];
		double x, y, dx, dy, dx2, dy2, curX, curY, tdx, tdy, lineX, lineY;
		double originX = 0, originY, tOriginX, tOriginY;
		double oldCTM[6], newCTM[6];
		double *mat = NULL;
		Dict *resDict = NULL;
		std::string::const_iterator p;
		int len = 0, n = 0, uLen = 0, nChars = 0, nSpaces = 0, i = 0;

		p = txt.begin ();
		len = txt.length ();

		// handle a Type 3 char
/*		if (font->getType() == fontType3 && out->interpretType3Chars()) 
		{
			mat = state.getCTM();
			for (i = 0; i < 6; ++i) {
			  oldCTM[i] = mat[i];
			}
			mat = state.getTextMat();
			newCTM[0] = mat[0] * oldCTM[0] + mat[1] * oldCTM[2];
			newCTM[1] = mat[0] * oldCTM[1] + mat[1] * oldCTM[3];
			newCTM[2] = mat[2] * oldCTM[0] + mat[3] * oldCTM[2];
			newCTM[3] = mat[2] * oldCTM[1] + mat[3] * oldCTM[3];
			mat = font->getFontMatrix();
			newCTM[0] = mat[0] * newCTM[0] + mat[1] * newCTM[2];
			newCTM[1] = mat[0] * newCTM[1] + mat[1] * newCTM[3];
			newCTM[2] = mat[2] * newCTM[0] + mat[3] * newCTM[2];
			newCTM[3] = mat[2] * newCTM[1] + mat[3] * newCTM[3];
			newCTM[0] *= state.getFontSize();
			newCTM[1] *= state.getFontSize();
			newCTM[2] *= state.getFontSize();
			newCTM[3] *= state.getFontSize();
			newCTM[0] *= state.getHorizScaling();
			newCTM[2] *= state.getHorizScaling();
			state.textTransformDelta(0, state.getRise(), &riseX, &riseY);
			curX = state.getCurX();
			curY = state.getCurY();
			lineX = state.getLineX();
			lineY = state.getLineY();
			while (len > 0) 
			{
			//  n = font->getNextChar(p, len, &code,
			//		    u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
			//		    &dx, &dy, &originX, &originY);
			  dx = dx * state.getFontSize() + state.getCharSpace();
			  if (n == 1 && *p == ' ') 
			  {
				  dx += state.getWordSpace();
			  }
			  dx *= state.getHorizScaling();
			  dy *= state.getFontSize();
			  state.textTransformDelta(dx, dy, &tdx, &tdy);
			  state.transform(curX + riseX, curY + riseY, &x, &y);
			  
			  state.setCTM(newCTM[0], newCTM[1], newCTM[2], newCTM[3], x, y);
			  
			  curX += tdx;
			  curY += tdy;
			  state.moveTo(curX, curY);
			  state.textSetPos(lineX, lineY);
			  p += n;
			  len -= n;
			}
		} else if (out->useDrawChar()) 
*/		{
		state.textTransformDelta(0, state.getRise(), &riseX, &riseY);
		while (len > 0) 
		{
		 // n = font->getNextChar(p, len, &code,
		//		    u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
		//		    &dx, &dy, &originX, &originY);
			n = 1; dy = 0;
			dx = dx * state.getFontSize() + state.getCharSpace();
			if (n == 1 && *p == ' ') 
			{
			  dx += state.getWordSpace();
			}
			dx *= state.getHorizScaling();
			dy *= state.getFontSize();
			
			state.textTransformDelta(dx, dy, &tdx, &tdy);
			originX *= state.getFontSize();
			originY *= state.getFontSize();
			state.textTransformDelta(originX, originY, &tOriginX, &tOriginY);
			state.shift(tdx, tdy);
			p += n;
			len -= n;
		}

/*		} else 
		{
		dx = dy = 0;
		nChars = nSpaces = 0;
		while (len > 0) 
		{
		//  n = font->getNextChar(p, len, &code,
		//		    u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
		//		    &dx2, &dy2, &originX, &originY);
		  dx += dx2;
		  dy += dy2;
		  if (n == 1 && *p == ' ') 
		  {
			++nSpaces;
		  }
		  ++nChars;
		  p += n;
		  len -= n;
		}
		if (wMode) 
		{
		  dx *= state.getFontSize();
		  dy = dy * state.getFontSize()
		   + nChars * state.getCharSpace()
		   + nSpaces * state.getWordSpace();
		} else {
		  dx = dx * state.getFontSize()
		   + nChars * state.getCharSpace()
		   + nSpaces * state.getWordSpace();
		  dx *= state.getHorizScaling();
		  dy *= state.getFontSize();
		}
		state.textTransformDelta(dx, dy, &tdx, &tdy);
		state.shift(tdx, tdy);
	*/	}// if ()
	}	
	//
	void 
	unknownUpdate (GfxState&, const PdfOperator::Operands&)
		{ printDbg (debug::DBG_DBG, "Unknown updater."); }

	// "m"
	void
	opmUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (2 <= args.size());
		printDbg (debug::DBG_DBG, "");
		state.moveTo (getDoubleFromIProperty (args[0]), getDoubleFromIProperty (args[1]));
	}
	// "Td"
	void
	opTdUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (2 <= args.size());
		double tx = state.getLineX() + getDoubleFromIProperty (args[0]);
		double ty = state.getLineY() + getDoubleFromIProperty (args[1]);
		state.textMoveTo(tx, ty);
	}
	// "Tm"
	void
	opTmUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (6 <= args.size ());
		printDbg (debug::DBG_DBG, "");
		state.setTextMat (	getDoubleFromIProperty (args[0]), 
							getDoubleFromIProperty (args[1]),
							getDoubleFromIProperty (args[2]), 
							getDoubleFromIProperty (args[3]),
							getDoubleFromIProperty (args[4]), 
							getDoubleFromIProperty (args[5])
						  );
		state.textMoveTo(0, 0);
	}
	// "BT"
	void
	opBTUpdate (GfxState& state, const PdfOperator::Operands&)
	{
		printDbg (debug::DBG_DBG, "");
		state.setTextMat (1, 0, 0, 1, 0, 0);
		state.textMoveTo (0, 0);
	}
	// "l"
	void
	oplUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (2 <= args.size ());
		state.lineTo (getDoubleFromIProperty(args[0]), getDoubleFromIProperty (args[1]));
	}
	// "c"
	void
	opcUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (6 <= args.size ());
		state.curveTo ( getDoubleFromIProperty(args[0]), getDoubleFromIProperty(args[1]),
						getDoubleFromIProperty(args[2]), getDoubleFromIProperty(args[3]),
						getDoubleFromIProperty(args[4]), getDoubleFromIProperty(args[5]));
	}
	// "v"
	void
	opvUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (4 <= args.size ());
		state.curveTo ( state.getCurX(), state.getCurY(), 
						getDoubleFromIProperty(args[0]),
						getDoubleFromIProperty(args[1]),
						getDoubleFromIProperty(args[2]),
						getDoubleFromIProperty(args[3])
					  );

	}
	// "y"
	void
	opyUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (4 <= args.size ());
		state.curveTo ( getDoubleFromIProperty(args[0]),
						getDoubleFromIProperty(args[1]),
						getDoubleFromIProperty(args[2]),
						getDoubleFromIProperty(args[3]),
						getDoubleFromIProperty(args[2]),
						getDoubleFromIProperty(args[3])
					  );
	}
	// "re"
	void
	opreUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (4 <= args.size ());
		double x = getDoubleFromIProperty(args[0]);
		double y = getDoubleFromIProperty(args[1]);
		double w = getDoubleFromIProperty(args[2]);
		double h = getDoubleFromIProperty(args[3]);
		state.moveTo(x, y);
		state.lineTo(x + w, y);
		state.lineTo(x + w, y + h);
		state.lineTo(x, y + h);
		state.closePath();
	}
	// "h"
	void
	ophUpdate (GfxState& state, const PdfOperator::Operands&)
		{state.closePath ();}
	// "Tc"
	void
	opTcUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (1 <= args.size());
  		state.setCharSpace (getDoubleFromIProperty(args[0]));
	}
	// "Tf"
	void
	opTfUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (1 <= args.size ());
		printDbg (debug::DBG_DBG, "NOT IMPLEMENTED YET");
		GfxFont *font = NULL;

		//if (!(font = res->lookupFont(args[0].getName())))
		//	return;
		state.setFont (font, getDoubleFromIProperty (args[1]));
	}
	// "Ts"
	void
	opTsUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (1 <= args.size());
		state.setRise (getDoubleFromIProperty (args[0])); 
	}
	// "Tw"
	void
	opTwUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (1 <= args.size());
  		state.setWordSpace (getDoubleFromIProperty (args[0]));
	}
	// "Tz"
	void
	opTzUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (1 <= args.size());
		state.setHorizScaling (getDoubleFromIProperty (args[0]));
	}
	// "TD"
	void
	opTDUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (2 <= args.size ());
		double tx = state.getLineX() + getDoubleFromIProperty (args[0]);
		double ty = getDoubleFromIProperty (args[1]);
		state.setLeading(-ty);
		ty += state.getLineY();
		state.textMoveTo(tx, ty);
	}
	// "T*"
	void
	opTstarUpdate (GfxState& state, const PdfOperator::Operands&)
	{
		double tx = state.getLineX();
		double ty = state.getLineY() - state.getLeading();
		state.textMoveTo (tx, ty);
	}
	// "'"
	void
	opApoUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (1 <= args.size ());
		double tx = state.getLineX();
		double ty = state.getLineY() - state.getLeading();
		state.textMoveTo(tx, ty);
		//printTextUpdate (state, getStringFromIProperty (args[0]));
	}
	// "TL"
	void
	opTLUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (1 <= args.size ());
  		state.setLeading ( getDoubleFromIProperty (args[0]));
	}
	// "\"
	void
	opSlashUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (3 <= args.size ());
		state.setWordSpace (getDoubleFromIProperty (args[0]));
		state.setCharSpace (getDoubleFromIProperty (args[1]));
		double tx = state.getLineX();
		double ty = state.getLineY() - state.getLeading();
		state.textMoveTo(tx, ty);
		//printTextUpdate (state, getStringFromIProperty (args[2]));
	}
	// "TJ"
	void
	opTJUpdate (GfxState& state, const PdfOperator::Operands& args)
	{
		assert (1 <= args.size ());
		if (!state.getFont()) 
			throw ElementBadTypeException ("opTJUpdate: State in bad state.");
		if (pArray != args[0]->getType())
			throw ElementBadTypeException ("opTJUpdate: Object in bad state.");
			
  		boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (args[0]);
		for (size_t i = 0; i < array->getPropertyCount(); ++i) 
		{
			boost::shared_ptr<IProperty> item = array->getProperty (i);
			PropertyType tp = item->getType ();
			switch (tp) 
			{
		     	case pInt:
				case pReal:
					{
					double dd = -getDoubleFromIProperty (item);
					// Always horizontal writine WMode
					state.textShift(dd * 0.001 * fabs(state.getFontSize()), 0);
					}
					break;
					
		    	case pString:
		      		//printTextUpdate (state, getStringFromIProperty (item));
				  break;

				default:
					throw ElementBadTypeException ("opTJUpdate: Bad object type.");
			}// switch
		}// for
	}
	// "Tj"
	void
	opTjUpdate (GfxState& /*state*/, const PdfOperator::Operands& args)
	{
		assert (1 <= args.size ());
 		//printTextUpdate (state, getStringFromIProperty (args[0]));
	}
	/*// ""
	void
	op (GfxState& state, const PdfOperator::Operands& args)
	{
	}
*/

	//==========================================================
	
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



	/** Maximum operator name length. */
	const static size_t MAX_OPERATOR_NAMELEN = 4;

	/**
	 * Known operators, it is copied from pdf BECAUSE it is
	 * a private member variable of a class and we do not have
	 *  access to it
	 */
	typedef struct
	{
		char name[MAX_OPERATOR_NAMELEN];	/**< Operator name. */
		int argNum;						 	/**< Number of arguments operator should get. */
		unsigned short types[MAX_OPERANDS];	/**< Bits are representing what it should be. */
		
		/** Function to execute when updating position. */
		void (*update) (GfxState&, const PdfOperator::Operands& );
		
		char endTag[MAX_OPERATOR_NAMELEN]; /**< If it is a complex type, its end tag.*/	
		
	} CheckTypes;

	/**
	 * All known operators.
	 * 
	 * Operator number can be either >0, zero, <0. Zero means no operands are
	 * needed. >0 means that exact argNum of operands are needed. <0 means that
	 * at most argNum operands are needed.
	 *
	 * Each item of types indicate which types we accept.
	 */
	const CheckTypes KNOWN_OPERATORS[] = {
			{"\\",    3, {setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), setNthBitsShort (pString)}, 
					opSlashUpdate, "" },	
			{"'",   1, {setNthBitsShort (pString)}, 
					opApoUpdate, "" },	
			{"B",   0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"B*",  0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"BDC", 2, {setNthBitsShort (pName), setNthBitsShort (pDict, pName)}, 
					unknownUpdate, "" },	
			{"BI",  0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"BMC", 1, {setNthBitsShort (pName)}, 
					unknownUpdate, "" },	
			{"BT",  0, {setNoneBitsShort ()}, 
					opBTUpdate, "" },	
			{"BX",  0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"CS",  1, {setNthBitsShort (pName)}, 
					unknownUpdate, "" },	
			{"DP",  2,	{setNthBitsShort (pName), setNthBitsShort (pDict, pName)}, 
					unknownUpdate, "" },	
			{"Do",  1, {setNthBitsShort (pName)}, 
				   	unknownUpdate, "" },	
			{"EI",  0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"EMC", 0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"ET",  0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"EX",  0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"F",   0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"G",   1, {setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"ID",  0, {setNoneBitsShort ()},
					unknownUpdate, "" },	
			{"J",   1, {setNthBitsShort (pInt)}, 
					unknownUpdate, "" },	
			{"K",   4, {setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal),
					    setNthBitsShort (pInt, pReal),	setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"M",   1, {setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"MP",  1, {setNthBitsShort (pName)}, 
					unknownUpdate, "" },	
			{"Q",   0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"RG",  3, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"S",   0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"SC",  -4, {setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),	setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
  			{"SCN", -5,  {setNthBitsShort (pInt, pReal, pName), setNthBitsShort (pInt, pReal, pName), 
						 setNthBitsShort (pInt, pReal, pName), setNthBitsShort (pInt, pReal, pName), 
						 setNthBitsShort (pInt, pReal, pName)}, 
					unknownUpdate, "" },	
			{"T*",  0, {setNoneBitsShort ()}, 
					opTstarUpdate, "" },	
			{"TD",  2, {setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					opTDUpdate, "" },	
			{"TJ",  1, {setNthBitsShort (pArray)}, 
					opTJUpdate, "" },	
			{"TL",  1, {setNthBitsShort (pInt, pReal)}, 
					opTLUpdate, "" },	
			{"Tc",  1, {setNthBitsShort (pInt, pReal)}, 
					opTcUpdate, "" },	
			{"Td",  2, 	{setNthBitsShort (pInt, pReal),	setNthBitsShort (pInt, pReal)}, 
					opTdUpdate, "" },	
			{"Tf",  2, 	{setNthBitsShort (pName), setNthBitsShort (pInt, pReal)}, 
					opTfUpdate, "" },	
			{"Tj",  1, {setNthBitsShort (pString)}, 
					opTjUpdate, "" },	
			{"Tm",  6, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal),
						 setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					opTmUpdate, "" },	
			{"Tr",  1, {setNthBitsShort (pInt)}, 
					unknownUpdate, "" },	
			{"Ts",  1, {setNthBitsShort (pInt, pReal)}, 
					opTsUpdate, "" },	
			{"Tw",  1, {setNthBitsShort (pInt, pReal)}, 
					opTwUpdate, "" },	
			{"Tz",  1, {setNthBitsShort (pInt, pReal)}, 
					opTzUpdate, "" },	
			{"W",   0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"W*",  0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"b",   0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"b*",  0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
  			{"c",   6, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal),    
						 setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					opcUpdate, "" },	
  			{"cm",  6, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal),    
						 setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal),setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"cs",  1, {setNthBitsShort (pName)}, 
					unknownUpdate, "" },	
			{"d",   2, 	{setNthBitsShort (pArray),setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"d0",  2, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
  			{"d1",  6, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal),    
						 setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"f",   0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"f*",  0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"g",   1, {setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"gs",  1, {setNthBitsShort (pName)}, 
					unknownUpdate, "" },	
			{"h",   0, {setNoneBitsShort ()}, 
					ophUpdate, "" },	
			{"i",   1, {setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"j",   1, {setNthBitsShort (pInt)}, 
					unknownUpdate, "" },	
			{"k",   4, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), 
						 setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"l",   2, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					oplUpdate, "" },	
			{"m",   2, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					opmUpdate, "" },	
			{"n",   0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"q",   0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"re",  4, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), 
						 setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					opreUpdate, "" },	
			{"rg",  3, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"ri",  1, {setNthBitsShort (pName)}, 
					unknownUpdate, "" },	
			{"s",   0, {setNoneBitsShort ()}, 
					unknownUpdate, "" },	
			{"sc",  -4, {setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
  			{"scn", -5,  {setNthBitsShort (pInt, pReal, pName), setNthBitsShort (pInt, pReal, pName),    
						 setNthBitsShort (pInt, pReal, pName),  setNthBitsShort (pInt, pReal, pName),    
						setNthBitsShort (pInt, pReal, pName)}, 
					unknownUpdate, "" },	
			{"sh",  1, {setNthBitsShort (pName)}, 
					unknownUpdate, "" },	
			{"v",   4, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal),    
						 setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal)}, 
					opvUpdate, "" },	
			{"w",   1, {setNthBitsShort (pInt, pReal)}, 
					unknownUpdate, "" },	
			{"y",   4, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),  setNthBitsShort (pInt, pReal)}, 
					opyUpdate, "" },	

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
		for (PdfOperator::Operands::iterator it = operands.begin(); it != operands.end(); ++it)
		{
			string tmp;
			(*it)->getStringRepresentation (tmp);
			str += " " + tmp;
		}
		printDbg (DBG_DBG, "Operands: " << str);
		
		//
		// Check operator size if > 0 than it is the exact size, maximum
		// otherwise
		//
		if (   ((ops.argNum >= 0) && (operands.size() != static_cast<size_t> (ops.argNum))) 
			|| ((ops.argNum <  0) && (operands.size() <= static_cast<size_t> (-ops.argNum))) )
		{
			printDbg (DBG_ERR, "Number of operands mismatch..");
			return false;
		}
		
		//
		// Check arguments
		//
		PdfOperator::Operands::reverse_iterator it = operands.rbegin ();
		advance (it, ops.argNum);
		for (int pos = 0; it != operands.rend (); ++it, ++pos)
		{			
  			if (!isBitSet(ops.types[pos], (*it)->getType()))
			{
				printDbg (DBG_ERR, "Bad " << pos << "-th operand type [" << (*it)->getType() << "] " << hex << " 0x" << ops.types[pos]);
				return false;
			}
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
				PdfOperator* composite)
	{
		printDbg (DBG_DBG, "Finding operator: " << op);

		// Set pdf to all operands
		operandsSetPdf (pdf, rf, operands);

		//
		// Try to find the op by its name
		// 
		const CheckTypes* chcktp = findOp (op);
		if (NULL == chcktp)
		{// operator not found
			printDbg (DBG_DBG, "Operator not found.");
			
			// Create unknown operator
			return new UnknownPdfOperator (operands, op);
		}
		printDbg (DBG_DBG, "Operator found. " << chcktp->name);

		// Check the types against specification
		if (!check (*chcktp, operands))
			throw MalformedFormatExeption ("Content stream bad operator type.");
			
		// Get operands count
		size_t argNum = static_cast<size_t> 
						((chcktp->argNum > 0) ? chcktp->argNum : -chcktp->argNum);

		//
		// If endTag is "" it is a simple operator, complex otherwise
		// 
		if ('\0' == chcktp->endTag[0])
		{// Simple operator
			return new SimpleGenericOperator (chcktp->name, argNum, operands);
		}else
		{// Complex operator
			
			return (composite = new UnknownCompositePdfOperator (chcktp->name, chcktp->endTag));	
		}
	}	

	/**
	 * Parse the stream into small objects.
	 *
	 * @param operators Operator stack.
	 * @param obj 	Xpdf content stream.
	 * @param pdf 	Pdf where this content stream belongs (parent object)
	 * @param rf 	Id of parent object.
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

		// If composite we have to add into composite
		PdfOperator* composite = NULL;
		//
		// Loop through all object, if it is an operator create pdfoperator else assume it is an operand
		//
		while (!o.isEOF()) 
		{
			if (o.isCmd ())
			{
				// Create operator
				boost::shared_ptr<PdfOperator> op  
					(createOp (string(o.getCmd ()), operands, pdf, rf, composite));
				//
				// Put it either to operators when the operator itself is a complex type
				// or if it is not a complex type
				// Put it behind complex type when the operand itself is not complex but we are in a
				// complex type (that is indicated by cmplex variable)
				//
				if (composite && composite != op.get())
				{
					//PdfOperator::putBehind (cmplex, op);
				
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
		string frst;
		op->getOperatorName(frst);
		// Get operator specification
		const CheckTypes* chcktp = findOp (frst);
		// Get operands
		PdfOperator::Operands ops;
		op->getParameters (ops);
		// If operator found use the function else use default
		if (NULL != chcktp)
		{
			assert ( (chcktp->argNum >= 0) || (ops.size () <= (size_t)-chcktp->argNum));
			assert ( (chcktp->argNum < 0) || (ops.size () == (size_t)chcktp->argNum));
			(chcktp->update) (state, ops);
			
		}else
		{
			unknownUpdate (state, ops);
		}

	}else
		throw ElementBadTypeException ("adjustActualPosition()");
	
}

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
	string frst, tmp;

	str.clear ();
	for (Operators::const_iterator it = operators.begin (); it != operators.end(); ++it)
	{
			
		(*it)->getOperatorName (frst);
		printDbg (DBG_DBG, "Operator name: " << frst << " param count: " << (*it)->getParametersCount() );
		
		(*it)->getStringRepresentation (tmp);
		str += tmp + "\n";
		tmp.clear ();
	}
}


//==========================================================
} // namespace pdfobjects
//==========================================================
