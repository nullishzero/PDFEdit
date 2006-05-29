// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  ccontentstream.cc
 *         Created:  03/24/2006 10:33:34 PM CET
 *          Author:  jmisutka, mjahoda
 * =====================================================================================
 */

// static
#include "static.h"
#include "ccontentstream.h"

//
#include "pdfoperators.h"
#include "cobjecthelpers.h"
#include "cobject.h"
#include "cinlineimage.h"

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
namespace {
//==========================================================

	
	//==========================================================
	// Actual state (position) updaters
	//==========================================================
	
	//
	void
	printTextUpdate (GfxState& state, const std::string& txt, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

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
		char *p;
		int len = 0, n = 0, uLen = 0, nChars = 0, nSpaces = 0, i = 0;
		GString s (txt.c_str());

		font = state.getFont();
		wMode = font->getWMode();
		
		// handle a Type 3 char
		if (font->getType() == fontType3 /* && out->interpretType3Chars() */) 
		{
			Rectangle h_rc();

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
			p = s.getCString();
			len = s.getLength();
			while (len > 0) 
			{
			  n = font->getNextChar(p, len, &code,
									u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
									&dx, &dy, &originX, &originY);
			  
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
		} else /* if (out->useDrawChar()) */ {
			state.textTransformDelta(0, state.getRise(), &riseX, &riseY);
			p = s.getCString();
			len = s.getLength();
			while (len > 0) {
				n = font->getNextChar(p, len, &code,
									  u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
									  &dx, &dy, &originX, &originY);
				if (wMode) {
					dx *= state.getFontSize();
					dy = dy * state.getFontSize() + state.getCharSpace();
					if (n == 1 && *p == ' ') {
						dy += state.getWordSpace();
					}
				} else {
					dx = dx * state.getFontSize() + state.getCharSpace();
					if (n == 1 && *p == ' ') {
						dx += state.getWordSpace();
					}
					dx *= state.getHorizScaling();
					dy *= state.getFontSize();
				}
				state.textTransformDelta(dx, dy, &tdx, &tdy);
				originX *= state.getFontSize();
				originY *= state.getFontSize();
				state.textTransformDelta(originX, originY, &tOriginX, &tOriginY);
				state.shift(tdx, tdy);
				p += n;
				len -= n;
			}
		}
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY() +state.getFontSize(), & rc->xright, & rc->yright);
	}	
	//
	void 
	unknownUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;
		/* utilsPrintDbg (debug::DBG_DBG, "Unknown updater.");*/
	}

	// "m"
	void
	opmUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (2 <= args.size());
		utilsPrintDbg (debug::DBG_DBG, "");
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		state.moveTo (getDoubleFromIProperty (args[0]), getDoubleFromIProperty (args[1]));
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "Td"
	void
	opTdUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (2 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		double tx = state.getLineX() + getDoubleFromIProperty (args[0]);
		double ty = state.getLineY() + getDoubleFromIProperty (args[1]);
		state.textMoveTo(tx, ty);
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "Tm"
	void
	opTmUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (6 <= args.size ());
		utilsPrintDbg (debug::DBG_DBG, "");
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		state.setTextMat (	getDoubleFromIProperty (args[0]), 
							getDoubleFromIProperty (args[1]),
							getDoubleFromIProperty (args[2]), 
							getDoubleFromIProperty (args[3]),
							getDoubleFromIProperty (args[4]), 
							getDoubleFromIProperty (args[5])
						  );
		state.textMoveTo(0, 0);

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "BT"
	void
	opBTUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		utilsPrintDbg (debug::DBG_DBG, "");
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		state.setTextMat (1, 0, 0, 1, 0, 0);
		state.textMoveTo (0, 0);

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "l"
	void
	oplUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (2 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);
		
		state.lineTo (getDoubleFromIProperty(args[0]), getDoubleFromIProperty (args[1]));

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "c"
	void
	opcUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (6 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		state.curveTo ( getDoubleFromIProperty(args[0]), getDoubleFromIProperty(args[1]),
						getDoubleFromIProperty(args[2]), getDoubleFromIProperty(args[3]),
						getDoubleFromIProperty(args[4]), getDoubleFromIProperty(args[5]));

		// simple calculate boundingbox of curve (inexact)
		Point h_width;
		state.transformDelta( state.getLineWidth()/2, state.getLineWidth()/2, & h_width.x, & h_width.y );
		rc->xleft -= h_width.x;
		rc->yleft -= h_width.y;
		rc->xright += h_width.x;
		rc->yright += h_width.y;
		Point h_pt;
		for (int i=0; i<6 ;i+=2) {
			state.transform(getDoubleFromIProperty(args[i]), getDoubleFromIProperty(args[i+1]), & h_pt.x, & h_pt.y);
			rc->xleft = min( rc->xleft, h_pt.x - h_width.x );
			rc->xright = max( rc->xright, h_pt.x + h_width.x );
			rc->yleft = min( rc->yleft, h_pt.y - h_width.y );
			rc->yright = max( rc->yright, h_pt.y + h_width.y );
		}
	}
	// "v"
	void
	opvUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (4 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		state.curveTo ( state.getCurX(), state.getCurY(), 
						getDoubleFromIProperty(args[0]),
						getDoubleFromIProperty(args[1]),
						getDoubleFromIProperty(args[2]),
						getDoubleFromIProperty(args[3])
					  );

		// simple calculate boundingbox of curve (inexact)
		Point h_width;
		state.transformDelta( state.getLineWidth()/2, state.getLineWidth()/2, & h_width.x, & h_width.y );
		rc->xleft -= h_width.x;
		rc->yleft -= h_width.y;
		rc->xright += h_width.x;
		rc->yright += h_width.y;
		Point h_pt;
		for (int i=0; i<4 ;i+=2) {
			state.transform(getDoubleFromIProperty(args[i]), getDoubleFromIProperty(args[i+1]), & h_pt.x, & h_pt.y);
			rc->xleft = min( rc->xleft, h_pt.x - h_width.x );
			rc->xright = max( rc->xright, h_pt.x + h_width.x );
			rc->yleft = min( rc->yleft, h_pt.y - h_width.y );
			rc->yright = max( rc->yright, h_pt.y + h_width.y );
		}
	}
	// "y"
	void
	opyUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (4 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		state.curveTo ( getDoubleFromIProperty(args[0]),
						getDoubleFromIProperty(args[1]),
						getDoubleFromIProperty(args[2]),
						getDoubleFromIProperty(args[3]),
						getDoubleFromIProperty(args[2]),
						getDoubleFromIProperty(args[3])
					  );

		// simple calculate boundingbox of curve (inexact)
		Point h_width;
		state.transformDelta( state.getLineWidth()/2, state.getLineWidth()/2, & h_width.x, & h_width.y );
		rc->xleft -= h_width.x;
		rc->yleft -= h_width.y;
		rc->xright += h_width.x;
		rc->yright += h_width.y;
		Point h_pt;
		for (int i=0; i<6 ;i+=2) {
			state.transform(getDoubleFromIProperty(args[i]), getDoubleFromIProperty(args[i+1]), & h_pt.x, & h_pt.y);
			rc->xleft = min( rc->xleft, h_pt.x - h_width.x );
			rc->xright = max( rc->xright, h_pt.x + h_width.x );
			rc->yleft = min( rc->yleft, h_pt.y - h_width.y );
			rc->yright = max( rc->yright, h_pt.y + h_width.y );
		}
	}
	// "re"
	void
	opreUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
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

		// Set rectangle from operands position on output devices
		state.transform(x - state.getLineWidth()/2, y - state.getLineWidth()/2, & rc->xleft, & rc->yleft);
		state.transform(x+w+ state.getLineWidth()/2, y+h+ state.getLineWidth()/2, & rc->xright, & rc->yright);
		Point h_pt;
		state.transform(x - state.getLineWidth()/2, y+h+ state.getLineWidth()/2, & h_pt.x, & h_pt.y);
			rc->xleft = min( rc->xleft, h_pt.x );
			rc->xright = max( rc->xright, h_pt.x );
			rc->yleft = min( rc->yleft, h_pt.y );
			rc->yright = max( rc->yright, h_pt.y );
		state.transform(x+w+ state.getLineWidth()/2, y - state.getLineWidth()/2, & h_pt.x, & h_pt.y);
			rc->xleft = min( rc->xleft, h_pt.x );
			rc->xright = max( rc->xright, h_pt.x );
			rc->yleft = min( rc->yleft, h_pt.y );
			rc->yright = max( rc->yright, h_pt.y );
		
	}
	// "h"
	void
	ophUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		state.closePath ();

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "Tc"
	void
	opTcUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

  		state.setCharSpace (getDoubleFromIProperty(args[0]));

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "Tf"
	void
	opTfUpdate (GfxState& state, boost::shared_ptr<GfxResources> res, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (2 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		assert( isName( args[0] ) );
		std::string val;
		IProperty::getSmartCObjectPtr<CName> (args[0]) ->getPropertyValue (val);

		GfxFont *font = NULL;

		if (!(font = res.get()->lookupFont (val.c_str())))
			return;		// same as displaing with xpdf/Gfx
		
		state.setFont (font, getDoubleFromIProperty (args[1]));
	}
	// "Ts"
	void
	opTsUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		state.setRise (getDoubleFromIProperty (args[0])); 

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "Tw"
	void
	opTwUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

  		state.setWordSpace (getDoubleFromIProperty (args[0]));

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "Tz"
	void
	opTzUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		state.setHorizScaling (getDoubleFromIProperty (args[0]));

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "TD"
	void
	opTDUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (2 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		double tx = state.getLineX() + getDoubleFromIProperty (args[0]);
		double ty = getDoubleFromIProperty (args[1]);
		state.setLeading(-ty);
		ty += state.getLineY();
		state.textMoveTo(tx, ty);

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "T*"
	void
	opTstarUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		double tx = state.getLineX();
		double ty = state.getLineY() - state.getLeading();
		state.textMoveTo (tx, ty);

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "'"
	void
	opApoUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		double tx = state.getLineX();
		double ty = state.getLineY() - state.getLeading();
		state.textMoveTo(tx, ty);
		//printTextUpdate (state, getStringFromIProperty (args[0]));

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "TL"
	void
	opTLUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

  		state.setLeading ( getDoubleFromIProperty (args[0]));

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "\"
	void
	opSlashUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (3 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		state.setWordSpace (getDoubleFromIProperty (args[0]));
		state.setCharSpace (getDoubleFromIProperty (args[1]));
		double tx = state.getLineX();
		double ty = state.getLineY() - state.getLeading();
		state.textMoveTo(tx, ty);
		//printTextUpdate (state, getStringFromIProperty (args[2]));

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
	// "TJ"
	void
	opTJUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		if (!state.getFont()) 
		{
			assert (!"Could not get font.");
			throw ElementBadTypeException ("opTJUpdate: State in bad state.");
		}
		if (pArray != args[0]->getType())
		{
			assert (!"opTJUpdate: Invalid first argument.");
			throw ElementBadTypeException ("opTJUpdate: Object in bad state.");
		}

		int wMode;
		wMode = state.getFont()->getWMode();

		Rectangle h_rc;
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
					// Set edge of rectangle from actual position on output devices
					state.transform(state.getCurX (), state.getCurY(), & h_rc.xleft, & h_rc.yleft);

					double dd = -getDoubleFromIProperty (item);
					if (wMode) {
						state.textShift(dd * 0.001 * fabs(state.getFontSize()), 0);
					} else {
						state.textShift(0, dd * 0.001 * fabs(state.getFontSize()));
					}

					// Set edge of rectangle from actual position on output devices
					state.transform(state.getCurX (), state.getCurY(), & h_rc.xright, & h_rc.yright);
					}

					break;
		    	case pString:
		      		printTextUpdate (state, getStringFromIProperty (item), & h_rc);
				  break;

				default:
					assert (!"opTJUpdate: Bad object type.");
					throw ElementBadTypeException ("opTJUpdate: Bad object type.");
			}// switch
			rc->xleft = min( rc->xleft, min( h_rc.xleft, h_rc.xright ) );
			rc->xright = max( rc->xright, max( h_rc.xleft, h_rc.xright ) );
			rc->yleft = min( rc->yleft, min( h_rc.yleft, h_rc.yright ) );
			rc->yright = max( rc->yright, max( h_rc.yleft, h_rc.yright ) );
		}// for
	}
	// "Tj"
	void
	opTjUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size ());
		assert( state.getFont() != NULL );	// No font for text

 		printTextUpdate (state, getStringFromIProperty (args[0]), rc);
	}
	// "cm"
	void
	opcmUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (6 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		state.concatCTM (
				getDoubleFromIProperty(args[0]),
				getDoubleFromIProperty(args[1]),
				getDoubleFromIProperty(args[2]),
				getDoubleFromIProperty(args[3]),
				getDoubleFromIProperty(args[4]),
				getDoubleFromIProperty(args[5]));

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}

	// "Q"
	void
	opQUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		state.restore();

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}

	// "q"
	void
	opqUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		state.save();

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}

	// "Tr"
	void
	opTrUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		state.setRender( getIntFromIProperty(args[0]) );

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}

	// "w"
	void
	opwUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		state.setLineWidth( getDoubleFromIProperty(args[0]) );
	}
	
	// "BI"
	void
	opBIUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator> op, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		utilsPrintDbg (DBG_DBG, "NOT IMPLEMENTED");
		utilsPrintDbg (DBG_DBG, " " << op->getParametersCount() << " , " << op->getChildrenCount());

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}

	// "ID"
	void
	opIDUpdate (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator> op, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);

		utilsPrintDbg (DBG_DBG, "NOT IMPLEMENTED");
		utilsPrintDbg (DBG_DBG, " " << op->getParametersCount() << " , " << op->getChildrenCount());

		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}

/*	// ""
	void
	op (GfxState& state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xleft, & rc->yleft);


		// Set edge of rectangle from actual position on output devices
		state.transform(state.getCurX (), state.getCurY(), & rc->xright, & rc->yright);
	}
*/
	//==========================================================
	// End of actual state (position) updaters
	//==========================================================


	//==========================================================
	
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


	/** Maximum operator arguments. From pdf spec.*/
	const size_t MAX_OPERANDS = 6;

	/** Maximum operator name length. From pdf spec.*/
	static const size_t MAX_OPERATOR_NAMELEN = 4;

	/**
	 * Known operators, it is copied from pdf BECAUSE it is
	 * a private member variable of a class and we do not have
	 * access to it
	 */
	typedef struct
	{
		char name[MAX_OPERATOR_NAMELEN];	/**< Operator name. */
		int argNum;						 	/**< Number of arguments operator should get. */
		unsigned short types[MAX_OPERANDS];	/**< Bits are representing what it should be. */
		
		/** Function to execute when updating position. */
		void (*update) (GfxState&, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc);
		
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
			{"BI",  -1, {setNoneBitsShort ()}, 
					opBIUpdate, "EI" },	
			{"BMC", 1, {setNthBitsShort (pName)}, 
					unknownUpdate, "" },	
			{"BT",  0, {setNoneBitsShort ()}, 
					opBTUpdate, "ET" },	
			{"BX",  0, {setNoneBitsShort ()}, 
					unknownUpdate, "EX" },	
			{"CS",  1, {setNthBitsShort (pName)}, 
					unknownUpdate, "" },	
			{"DP",  2,	{setNthBitsShort (pName), setNthBitsShort (pDict, pName)}, 
					unknownUpdate, "" },	
			{"Do",  1, {setNthBitsShort (pName)}, 
				   	unknownUpdate, "" },	
			{"EI",  -10, {setNoneBitsShort ()}, 
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
					opIDUpdate, "" },	
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
					opQUpdate, "" },	
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
					opTrUpdate, "" },	
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
					opcmUpdate, "" },	
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
					opqUpdate, "Q" },	
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
					opwUpdate, "" },	
			{"y",   4, 	{setNthBitsShort (pInt, pReal), setNthBitsShort (pInt, pReal),    
						setNthBitsShort (pInt, pReal),  setNthBitsShort (pInt, pReal)}, 
					opyUpdate, "" },	

		};
	
	/** Is it a simple or complex operator. */
	bool isSimpleOperator (const CheckTypes& chck)
		{ return ('\0' == chck.endTag[0]); }

	/**
	 * Set pdf to operands.
	 *
	 * This is vital when changing those operands.
	 *
	 * @param first First operator to set pdf to.
	 * @param pdf Pdf where operand will belong.
	 * @param rf  Indiref of content's stream parent.
	 * @param cs Content stream.
	 */
	void
	opsSetPdfRefCs (shared_ptr<PdfOperator> first, CPdf& pdf, IndiRef rf, CContentStream& cs)
	{
		utilsPrintDbg (DBG_DBG, "");
		CContentStream::OperatorIterator it = PdfOperator::getIterator (first);
		while (!it.isEnd())
		{
			// Set cs
			it.getCurrent()->setContentStream (cs);
			
			// Get operands
			PdfOperator::Operands operands;
			it.getCurrent()->getParameters (operands);
			
			// Set pdf and ref
			PdfOperator::Operands::iterator oper = operands.begin ();
			for (; oper != operands.end (); ++oper)
			{
				(*oper)->setPdf (&pdf);
				(*oper)->setIndiRef (rf);
			}
			
			it = it.next ();
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
		// \TODO DEBUG
		string str;
		for (PdfOperator::Operands::iterator it = operands.begin(); it != operands.end(); ++it)
		{
			string tmp;
			(*it)->getStringRepresentation (tmp);
			str += " " + tmp;
		}
		utilsPrintDbg (DBG_DBG, "Operands: " << str);
		/////
		
		size_t argNum = static_cast<size_t> ((ops.argNum > 0) ? ops.argNum : -ops.argNum);
			
		//
		// Check operator size if > 0 than it is the exact size, maximum
		// otherwise
		//
		if (((ops.argNum >= 0) && (operands.size() != argNum)) 
			 || ((ops.argNum <  0) && (operands.size() > argNum)) )
		{
			utilsPrintDbg (DBG_ERR, "Number of operands mismatch.. expected " << ops.argNum << " got: " << operands.size());
			return false;
		}
		
		//
		// Check arguments
		//
		PdfOperator::Operands::reverse_iterator it = operands.rbegin ();
		// Be carefull of buffer overflow
		argNum = std::min (argNum, operands.size());
		advance (it, argNum);
		for (int pos = 0; it != operands.rend (); ++it, ++pos)
		{			
  			if (!isBitSet(ops.types[pos], (*it)->getType()))
			{
				utilsPrintDbg (DBG_ERR, "Bad " << pos << "-th operand type [" << (*it)->getType() << "] " << hex << " 0x" << ops.types[pos]);
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
	 * Parse inline image. Inline image is a stream withing e.g. a text stream.
	 * Binary data can make text parser to behave incorrectly.
	 *
	 * @param stream Actual parser.
	 *
	 * @return CStream representing inline image
	 */
	CInlineImage*
	getInlineImage (CStreamXpdfReader<CContentStream::CStreams>& streamreader) 
	{
		kernelPrintDbg (DBG_DBG, "");
		xpdf::XpdfObject dict;
		dict->initDict ((XRef*)NULL); // We do not have (need) valid xref, but be CAREFUL

		// Get first object
		Object o;
		streamreader.getXpdfObject (o);

		//
		// Get the inline image dictionary
		// 
		while (!streamreader.eof() && !o.isCmd("ID")) 
		{
			if (o.isName())
			{
				char* key = ::copyString (o.getName());
				streamreader.getXpdfObject (o);
				if (streamreader.eof()) 
				{
					gfree (key);
					assert (!"Bad inline image.");
					throw CObjInvalidObject ();
				}
				dict->dictAdd (key, &o);
			
			}
			
			streamreader.getXpdfObject (o);
		}
		// Free ID
		o.free ();

		// Bad content stream
		if (streamreader.eof())
		{
			utilsPrintDbg (debug::DBG_CRIT, "Content stream is damaged...");
			return NULL;
		}
	
		// 
		// Make stream
		// 
		boost::scoped_ptr<Stream> str (new ::EmbedStream (streamreader.getXpdfStream (), dict.get(), gFalse, 0));
		str->reset();
		if (str)
		{
			// Copy chars to buf and with this buffer initialize CInlineImage
			CStream::Buffer buf;
			
			int c1, c2;
			c1 = str->getBaseStream()->getChar();
			c2 = str->getBaseStream()->getChar();
			buf.push_back (c1);
			buf.push_back (c2);
			while (!('E' == c1 && 'I' == c2) && EOF != c2) 
			{
				c1 = c2;
				c2 = str->getBaseStream()->getChar();
				buf.push_back (c2);
			}
			// Pop EI
			buf.pop_back ();
			buf.pop_back ();
			return new CInlineImage (*dict, buf);
				
		}else
		{
			assert (!"Bad embedded stream.");
			throw CObjInvalidObject ();
		}
	}

	
	/**
	 * Find the operator and create it. 
	 *
	 * Here is decided which implementation of a pdf operator is used.
	 *
	 * @param stream Parser.
	 * @param operands Operands.
	 * @param last Last.
	 *
	 * @return Created pdf operator.
	 */
	shared_ptr<PdfOperator>
	createOp (CStreamXpdfReader<CContentStream::CStreams>& streamreader, PdfOperator::Operands& operands, shared_ptr<PdfOperator>& last)
	{
		// This is ugly but needed because of memory leaks
		shared_ptr<PdfOperator> result;
		
		// Get first object
		Object o;
		streamreader.getXpdfObject (o);

		//
		// Loop through all object, if it is an operator create pdfoperator else assume it is an operand
		//
		while (!streamreader.eof()) 
		{
			if (o.isCmd ())
			{// We have an OPERATOR

				//
				// Try to find the op by its name
				// 
				const CheckTypes* chcktp = findOp (o.getCmd());
				// Operator not found, create unknown operator
				if (NULL == chcktp)
				{
					result = shared_ptr<PdfOperator> (new UnknownPdfOperator (operands, string (o.getCmd())));
					break;
				}
				
				assert (chcktp);
				utilsPrintDbg (DBG_DBG, "Operator found. " << chcktp->name);

				//
				// SPECIAL CASE for inline image (stream within a text stream)
				//
				if ( 0 == strncmp (chcktp->name, "BI", 2))
				{
					utilsPrintDbg (debug::DBG_DBG, "");
					
					shared_ptr<CInlineImage> inimg (getInlineImage (streamreader));
					result = shared_ptr<PdfOperator> (new InlineImageCompositePdfOperator (chcktp->name, chcktp->endTag, inimg));
					break;
				}
				
				//
				// Check the type against specification
				// 
				if (!check (*chcktp, operands))
				{
					o.free ();
					assert (!"Content stream bad operator type.");
					throw MalformedFormatExeption ("Content stream bad operator type.");
				}
				
				// Get operands count
				size_t argNum = static_cast<size_t> ((chcktp->argNum > 0) ? chcktp->argNum : -chcktp->argNum);

				//
				// If endTag is "" it is a simple operator, composite otherwise
				// 
				if (isSimpleOperator(*chcktp))
				{// Simple operator
					
					last = result = shared_ptr<PdfOperator> (new SimpleGenericOperator (chcktp->name, argNum, operands));
					break;
					
				}else
				{// Composite operator
					
					bool foundEndTag = false;
					result = shared_ptr<PdfOperator> (new UnknownCompositePdfOperator (chcktp->name, chcktp->endTag));	
					// The same as in reparseContentStream
					shared_ptr<PdfOperator> _last = result;
					shared_ptr<PdfOperator> _prevLast =result;
					
					// While not the end
					while (last = createOp (streamreader, operands, _last))
					{
						result->push_back (last, _prevLast);
							
						// Is it the end tag?
						string tag;
						last->getOperatorName (tag);
						if (tag == chcktp->endTag)
						{
							foundEndTag = true;
							break;
						}
						
						// Save last as previous
						_prevLast = _last;
					}

					if (!foundEndTag)
					{
						assert (!"Bad content stream while reparsing. End tag was not found.");
						throw CObjInvalidObject ();
					}
					
					// Return composite in result
					break;
				}

			}else // if (o.isCmd ())
			{// We have an OPERAND
				
				shared_ptr<IProperty> pIp (createObjFromXpdfObj (o));
				operands.push_back (pIp);
			}

			o.free ();
			// Grab the next object
			streamreader.getXpdfObject (o);

		} // while (!obj.isEOF())
		
		o.free ();
		return result;
	}

	/**
	 * Parse the stream into small objects. 
	 *
	 * This function must NOT be called during initialization of contentstream.
	 * Use ** instead.
	 * 
	 * It assumes that supplied streams build a valid content stream that is not
	 * breaked neither in a composite operator nor at any other crazy point that the pdf
	 * specification allows.
	 *
	 * @param operators Operator stack.
	 * @param streams 	Streams to be parsed.
	 * @param cs 		Content stream in which operators belong
	 */
	void
	reparseContentStream (CContentStream::Operators& operators, CContentStream::CStreams& streams, CContentStream& cs)
	{
		PdfOperator::Operands operands;
		shared_ptr<PdfOperator> last, previousLast;
		shared_ptr<PdfOperator> actual;

		// Clear operators
		operators.clear ();
	
		// Check if streams are in a valid pdf
		for (CContentStream::CStreams::const_iterator it = streams.begin(); it != streams.end(); ++it)
		{
			assert (hasValidPdf (*it) && hasValidRef (*it));
			if (!hasValidPdf (*it) || !hasValidRef (*it))
				throw CObjInvalidObject ();
		}

		assert (!streams.empty());
		CStreamXpdfReader<CContentStream::CStreams> streamreader (streams);
		streamreader.open ();
	
		//
		// actual is the top most last one, e.g. a composite
		// last is the last operator, e.g. the last item of an composite
		// we want to add new (actual) after the last one of LAST call to
		// 	createOp
		//
		while (actual=createOp (streamreader, operands, last))
		{
			if (previousLast)
			{
				// Insert it in to the pdfoperator chain
				PdfOperator::putBehind (previousLast, actual);
			}
			// Save it into our "top level" container
			operators.push_back (actual);
			previousLast = last;
		}

		// Close actual stream
		streamreader.close ();

		assert (operands.empty());
		// Set pdf ref and cs
		if (!operators.empty())
			opsSetPdfRefCs (operators.front(), *(streams.front()->getPdf()), streams.front()->getIndiRef(), cs);
	}

	/**
	 * Parse the stream for the first time into small objects. 
	 *
	 * Problem with content stream is, that it can be splitted in many streams
	 * and the split points are really insane. And moreover some pdf creators 
	 * produce even more insane split points.
	 * 
	 * @param operators Operator stack.
	 * @param streams 	Streams to be parsed.
	 * @param cs 		Content stream in which operators belong
	 * @param parsedstreams Streams that have been really parsed.
	 */
	void
	parseContentStream (CContentStream::Operators& operators, 
						CContentStream::CStreams& streams, 
						CContentStream& cs,
						CContentStream::CStreams& parsedstreams)
	{
		PdfOperator::Operands operands;
		shared_ptr<PdfOperator> last, previousLast;
		shared_ptr<PdfOperator> actual;

		// Clear operators
		operators.clear ();
	
		// Check if streams are in a valid pdf
		for (CContentStream::CStreams::const_iterator it = streams.begin(); it != streams.end(); ++it)
		{
			assert (hasValidPdf (*it) && hasValidRef (*it));
			if (!hasValidPdf (*it) || !hasValidRef (*it))
				throw CObjInvalidObject ();
		}
		CPdf* pdf = streams.front()->getPdf ();
		assert (pdf);
		IndiRef rf = streams.front()->getIndiRef ();

		assert (!streams.empty());
		CStreamXpdfReader<CContentStream::CStreams> streamreader (streams);
		streamreader.open ();
	
		//
		// actual is the top most last one, e.g. a composite
		// last is the last operator, e.g. the last item of an composite
		// we want to add new (actual) after the last one of LAST call to
		// 	createOp
		//
		while (actual=createOp (streamreader, operands, last))
		{
			if (previousLast)
			{
				// Insert it in to the pdfoperator chain
				PdfOperator::putBehind (previousLast, actual);
			}
			// Save it into our "top level" container
			operators.push_back (actual);
			previousLast = last;

			// We have found a correct content stream.
			if (streamreader.eofOfActualStream())
				break;				
		}

		// Save which streams were parsed and close
		streamreader.close (parsedstreams);

		// Check if they match the input streams and delete them
		CContentStream::CStreams::iterator itparsed = parsedstreams.begin (); 
		for (; itparsed != parsedstreams.end(); ++itparsed)
		{			
			assert (*(streams.begin()) == *itparsed);
			//streams.erase (streams.begin());
			streams.pop_front();
		}

		assert (operands.empty());
		// Set pdf ref and cs
		if (!operators.empty())
			opsSetPdfRefCs (operators.front(), *pdf, rf, cs);
	}


	/**
	 * Saves bounding box of all operators.
	 *
	 * @param it Iterator that will be used to traverse all operators.
	 * @param res Resources.
	 * @param state Gfx state.
	 */
	void
	setOperatorBBox (PdfOperator::Iterator it, boost::shared_ptr<GfxResources> res, GfxState& state)
	{
		utilsPrintDbg (debug::DBG_DBG, "");
		boost::shared_ptr<PdfOperator> op;
		Rectangle rc;

		// Init global variables
		xpdf::openXpdfMess ();
		
		while (!it.isEnd ())
		{
			op = it.getCurrent();
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
				// Check arguments
				assert ( (chcktp->argNum >= 0) || (ops.size () <= (size_t)-chcktp->argNum));
				assert ( (chcktp->argNum < 0) || (ops.size () == (size_t)chcktp->argNum));
				if ( ((chcktp->argNum >= 0) && (ops.size () != (size_t)chcktp->argNum)) ||
				      ((chcktp->argNum < 0) && (ops.size () > (size_t)-chcktp->argNum)) )
				{
					kernelPrintDbg (debug::DBG_CRIT, "Bad content stream. Incorrect parameters.");
					throw CObjInvalidObject ();
				}

				// Update the state
				(chcktp->update) (state, res, op, ops, &rc);
				
			}else
			{
				// Update the state
				unknownUpdate (state, res, op, ops, &rc);
			}

			op->setBBox (rc);
			it = it.next ();
		
		} // while

		// Close xpdf mess
		xpdf::closeXpdfMess ();
	}

//==========================================================
} // namespace
//==========================================================

//==========================================================
// CContentStream observer
//==========================================================

//
// Observer interface
//

//
//
//
void 
CContentStream::CStreamObserver::notify (boost::shared_ptr<IProperty> newValue, 
									boost::shared_ptr<const IProperty::ObserverContext>) const 
throw ()
{
	try {

	utilsPrintDbg (debug::DBG_DBG, "");
	assert (hasValidPdf (newValue));
	assert (hasValidRef (newValue));

	// Stream has changed, reparse it
	contentstream->reparse ();
	
	}catch (...)
	{
		assert (!"This is very very bad because this function can't throw according to the interface.");
	}
}

//==========================================================
// CContentStream
//==========================================================

//
// Constructors
//
CContentStream::CContentStream (CStreams& strs, 
								boost::shared_ptr<GfxState> state, 
								boost::shared_ptr<GfxResources> res) 
	: gfxstate (state), gfxres (res)
{
	kernelPrintDbg (DBG_DBG, "");
	
	// If streams are empty return
	if (strs.empty())
		return;

	// Check if streams are in a valid pdf
	for (CStreams::const_iterator it = strs.begin(); it != strs.end(); ++it)
	{
		assert (hasValidPdf(*it) && hasValidRef(*it));
		if (!hasValidPdf(*it) || !hasValidRef(*it))
			throw CObjInvalidObject ();
	}
	
	// Parse it into small objects
	assert (gfxres);
	assert (gfxstate);
	
	// Parse it, move parsed straems from strs to cstreams
	parseContentStream (operators, strs, *this, cstreams);
	
	// Save bounding boxes
	if (!operators.empty())
		setOperatorBBox (PdfOperator::getIterator (operators.front()), gfxres, *gfxstate);

	// Create and register observer
	observer = boost::shared_ptr<CStreamObserver> (new CStreamObserver (this));
	registerObservers ();
}


//
// Helper methods
//

//
//
//
void
CContentStream::reparse (boost::shared_ptr<GfxState> state, boost::shared_ptr<GfxResources> res)
{
	// Save resources if new
	if (state)
		gfxstate = state;
	if (res)
		gfxres = res;
	
	assert (gfxres);
	assert (gfxstate);
	
	// Clear operators	
	operators.clear ();
	
	// Parse it
	reparseContentStream (operators, cstreams, *this);
	
	// Save bounding boxes
	if (!operators.empty())
		setOperatorBBox (PdfOperator::getIterator (operators.front()), gfxres, *gfxstate);
}

//
//
//
void
CContentStream::_objectChanged ()
{
	assert (!cstreams.empty());
	// Do not notify anything if we are not in a valid pdf
	if (!hasValidPdf (cstreams.front()))
		return;
	assert (hasValidRef (cstreams.front()));

	//
	// Make the change
	//  -- unregister OBSERVERS, because when saving to ccs which consists of
	//  more cstreams, after first save an error occurs, because the stream is
	//  no a valid content stream (it parses stream in which first stream is the
	//  complete stream and the next stream contains a part of the previous
	//  stream and it is an error)
	//	-- put everything into FIRST content stream
	//
	
	//
	// Unregister observer
	// 
	unregisterObservers ();

	// Save it
	string tmp;
	getStringRepresentation (tmp);
	assert (!cstreams.empty());
	CStreams::iterator it = cstreams.begin();
	assert (it != cstreams.end());
	// Put it to the first cstream
	(*it)->setBuffer (tmp);
	++it;
	// Erase all others
	for (;it != cstreams.end();++it)
		(*it)->setBuffer (string(""));

	//
	// Register observers again
	// 
	registerObservers ();
	
	// Notify observers
	this->notifyObservers ( shared_ptr<CContentStream> (),shared_ptr<const ObserverContext> ());
}


//
// If an operator is in CContentStream::operators it is
// 	* not in a composite
// if it is not there it is
//  * in a composite, so we have to remove it from the composite
//  
// It an operator is a composite
//  * we have to carefully adjust iterator
// 
void
CContentStream::deleteOperator (OperatorIterator it, bool indicateChange)
{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (!operators.empty());

	// Be sure that the operator won't get deallocated along the way
	boost::shared_ptr<PdfOperator> toDel = it.getCurrent ();
	
	//
	// Remove it from operators or composite
	// 
	
	Operators::iterator operIt = std::find (operators.begin(), operators.end(), toDel);
	if (operIt == operators.end())
	{
		// Find the composite in which the operator resides
		OperatorIterator begin = PdfOperator::getIterator (operators.front());
		boost::shared_ptr<PdfOperator> composite = findCompositeOfPdfOperator (begin, toDel);
		assert (composite);
		// Remove it from composite
		if (composite)
			composite->remove (toDel);
		else
		{
			//assert ("Want to delete a not existing operator.");
			throw CObjInvalidObject ();
		}
	
	}else
	{
		// Remove it from operators
		operators.erase (operIt);
	}

	
	//
	// Remove it from iterator list
	//
	
	// Get "real" next
	OperatorIterator itNxt = PdfOperator::getIterator (getLastOperator (toDel)).next();
	// Get the prev of operator that should be deleted
	OperatorIterator itPrv = it; itPrv.prev ();
	// Set iterators, in other words remove operIt from iterator list
	if (!itNxt.isEnd())
		itNxt.getCurrent()->setPrev (itPrv.getCurrent());
	if (!itPrv.isEnd())
		itPrv.getCurrent()->setNext (itNxt.getCurrent());

	//
	// To be sure
	//
	toDel->setPrev (PdfOperator::ListItem());
	toDel->setNext (PdfOperator::ListItem());
	
	// If indicateChange is true, pdf&rf&contenstream is set when reparsing
	if (indicateChange)
		_objectChanged ();
}


//
//
//
void
CContentStream::insertOperator (OperatorIterator it, boost::shared_ptr<PdfOperator> newOper, bool indicateChange)
{
	kernelPrintDbg (debug::DBG_DBG, "");

	// Insert into empty contentstream
	if (operators.empty ())
	{
		assert (it.isEnd());
		operators.push_back (newOper);
		return;
	}
	assert (!it.isEnd());

	//
	// Insert into operators or composite
	// 
	
	Operators::iterator operIt = std::find (operators.begin(), operators.end(), it.getCurrent());
	if (operIt == operators.end())
	{
		// Find the composite in which the operator resides
		OperatorIterator begin = PdfOperator::getIterator (operators.front());
		boost::shared_ptr<PdfOperator> composite = findCompositeOfPdfOperator (begin, it.getCurrent());
		assert (composite);
		// Insert it into composite
		if (composite)
			composite->insert_after (it.getCurrent(), newOper);
		else
		{
			//assert ("Want to insert after not existing operator.");
			throw CObjInvalidObject ();
		}
	
	}else
	{
		// Insert it into operators
		++operIt;
		operators.insert (operIt, newOper);
	}

	//
	// Insert it in the iterator list
	//
	
	// Get "real" next
	assert (!it.isEnd());
	OperatorIterator itCur = PdfOperator::getIterator(getLastOperator (it));
	OperatorIterator itNxt = itCur; itNxt.next();
	assert (!itCur.isEnd());
	// Set iterators, in other words insert newVal to iterator list
	itCur.getCurrent()->setNext (newOper);
	newOper->setPrev (itCur.getCurrent());
	
	if (!itNxt.isEnd())
	{
		itNxt.getCurrent()->setPrev (newOper);
		newOper->setNext (itNxt.getCurrent());
	}


	// If indicateChange is true, pdf&rf&contenstream is set when reparsing
	if (indicateChange)
		_objectChanged ();
}


//
// We can not simply use add and delete function because we would mess up
// iterator list in both cases
//
void
CContentStream::replaceOperator (OperatorIterator it, 
								 boost::shared_ptr<PdfOperator> newOper, 
								 OperatorIterator itPrv,
								 OperatorIterator itNxt,
								 bool indicateChange)
{


	kernelPrintDbg (debug::DBG_DBG, "");
	assert (!operators.empty());

	// Be sure that the operator won't get deallocated along the way
	boost::shared_ptr<PdfOperator> toReplace = it.getCurrent ();
	
	//
	// Replace in operators or composite
	// 
	
	Operators::iterator operIt = std::find (operators.begin(), operators.end(), toReplace);
	if (operIt == operators.end())
	{
		// Find the composite in which the operator resides
		OperatorIterator begin = PdfOperator::getIterator (operators.front());
		boost::shared_ptr<PdfOperator> composite = findCompositeOfPdfOperator (begin, toReplace);
		assert (composite);
		// Replace it from composite
		if (composite)
		{
			composite->insert_after (toReplace, newOper);
			composite->remove (toReplace);
		
		}else
		{
			//assert ("Want to insert after not existing operator.");
			throw CObjInvalidObject ();
		}
	
	}else
	{
		// Replace it from operators
		std::replace (operators.begin(), operators.end(), *operIt, newOper);
	}

	
	//
	// Remove it from iterator list
	//
	
	// First and last of new operator
	OperatorIterator itCur = PdfOperator::getIterator (newOper);
	assert (!itCur.isEnd());
	OperatorIterator itCurLast = PdfOperator::getIterator(getLastOperator (itCur));
	assert (!itCurLast.isEnd());

	if (!itNxt.isEnd())
	{
		itNxt.getCurrent()->setPrev (itCurLast.getCurrent());
		itCurLast.getCurrent()->setNext (itNxt.getCurrent());
	}
	if (!itPrv.isEnd())
	{
		itPrv.getCurrent()->setNext (itCur.getCurrent());
		itCur.getCurrent()->setPrev (itPrv.getCurrent());
	}
	
	// If indicateChange is true, pdf&rf&contenstream is set when reparsing
	if (indicateChange)
		_objectChanged ();
}

//
// Observer interface
//

//
//
//
void
CContentStream::registerObservers () const
{
	if (observer)
	{
		// Register observer
		for (CStreams::const_iterator it = cstreams.begin(); it != cstreams.end(); ++it)
			(*it)->registerObserver (observer);
	}else
	{
		assert (!"Observer is not initialized.");
		throw CObjInvalidOperation ();
	}
}

//
//
//
void
CContentStream::unregisterObservers () const
{
	if (observer)
	{
		// Unregister observer
		for (CStreams::const_iterator it = cstreams.begin(); it != cstreams.end(); ++it)
			(*it)->unregisterObserver (observer);
	}else
	{
		assert (!"Observer is not initialized.");
		throw CObjInvalidOperation ();
	}
}


//==========================================================
// Operator changing functions
//==========================================================


//
//
//
bool 
containsNonStrokingOperator (boost::shared_ptr<PdfOperator> oper)
{
	NonStrokingOperatorIterator it = PdfOperator::getIterator<NonStrokingOperatorIterator> (oper);
	if (it.isEnd())
		return false;
	else
		return true;
}


//
//
//
bool 
containsStrokingOperator (boost::shared_ptr<PdfOperator> oper)
{
	StrokingOperatorIterator it = PdfOperator::getIterator<StrokingOperatorIterator> (oper);
	if (it.isEnd())
		return false;
	else
		return true;
}


//
// This will be the result:
// q 		-- save graphical state
// r g b rg/RG	-- change color
// operator	-- changed operator
// Q		-- restore graphical state
//
boost::shared_ptr<PdfOperator>
operatorSetColor (boost::shared_ptr<PdfOperator> oper, double r, double g, double b)
{
	//
	// Change prev and next to NULL of oper,this will be needed in
	// containsNonStrokingOperator and containsStrokingOperator functions
	//
	// Anyway these will change in push_back function
	//
	oper->setPrev (PdfOperator::ListItem());
	getLastOperator(oper)->setNext (PdfOperator::ListItem());
	
	// Create empty composite
	boost::shared_ptr<CompositePdfOperator> composite (new UnknownCompositePdfOperator	 ("",""));

	PdfOperator::Operands operands;

	// q
	operands.clear();
	// This is the first operator to be inserted, so we HAVE TO specify the second paramater.
	composite->push_back (shared_ptr<PdfOperator> (new SimpleGenericOperator ("q", 0, operands)), composite);
	
	// r g b rg
	operands.clear ();
	operands.push_back (shared_ptr<IProperty> (new CReal (r)));
	operands.push_back (shared_ptr<IProperty> (new CReal (g)));
	operands.push_back (shared_ptr<IProperty> (new CReal (b)));

	if (containsNonStrokingOperator(oper))
		composite->push_back (shared_ptr<PdfOperator> (new SimpleGenericOperator ("rg", 3, operands)));
	if (containsStrokingOperator(oper))
		composite->push_back (shared_ptr<PdfOperator> (new SimpleGenericOperator ("RG", 3, operands)));
	// DEBUG
	if (!containsStrokingOperator(oper) && !containsNonStrokingOperator(oper))
	{
		string opstr;
		oper->getStringRepresentation (opstr);
		kernelPrintDbg (debug::DBG_CRIT, opstr);
		assert (!"Function does not have desired effect, because stroking/nonstroking operator was not found...\n"
				"Please give one of the operators (printed in debug mode)"
				" to either Stroking or Nonstroking iterator and recompile...");
	}
	
	// operator
	composite->push_back (oper);
	
	// Q
	operands.clear();
	composite->push_back (shared_ptr<PdfOperator> (new SimpleGenericOperator ("Q", 0, operands)));

	return composite;
}



//==========================================================
} // namespace pdfobjects
//==========================================================
