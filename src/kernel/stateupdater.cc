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

//
#include "stateupdater.h"
#include "pdfoperators.h"
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
namespace {
//==========================================================

	
	//==========================================================
	// Actual state (position) updaters
	//==========================================================
	
	GfxState *
	printTextUpdate (GfxState* state, const std::string& txt, Rectangle* rc)
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
		char *p;
		int len = 0, n = 0, uLen = 0, nChars = 0, nSpaces = 0, i = 0;
		GString s (txt.c_str());

		font = state->getFont();
		wMode = font->getWMode();
		
		// Set edge of rectangle from actual position on output devices
		state->textTransformDelta(0, font->getDescent() * state->getFontSize(), & x, & y);
                state->transform(state->getCurX() + x, state->getCurY() + y, & rc->xleft, & rc->yleft);

		// handle a Type 3 char
		if (font->getType() == fontType3 /* && out->interpretType3Chars() */) 
		{
			Rectangle h_rc();

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
			p = s.getCString();
			len = s.getLength();
			while (len > 0) 
			{
			  n = font->getNextChar(p, len, &code,
									u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
									&dx, &dy, &originX, &originY);
			  
			  dx = dx * state->getFontSize() + state->getCharSpace();
			  if (n == 1 && *p == ' ') 
			  {
				  dx += state->getWordSpace();
			  }
			  dx *= state->getHorizScaling();
			  dy *= state->getFontSize();
			  state->textTransformDelta(dx, dy, &tdx, &tdy);
			  state->transform(curX + riseX, curY + riseY, &x, &y);
			  
			  //state->setCTM(newCTM[0], newCTM[1], newCTM[2], newCTM[3], x, y);
			  
			  curX += tdx;
			  curY += tdy;
			  state->moveTo(curX, curY);
			  state->textSetPos(lineX, lineY);
			  p += n;
			  len -= n;
			}
		} else /* if (out->useDrawChar()) */ {
			state->textTransformDelta(0, state->getRise(), &riseX, &riseY);
			p = s.getCString();
			len = s.getLength();
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
				state->shift(tdx, tdy);
				p += n;
				len -= n;
			}
		}
		// Set edge of rectangle from actual position on output devices
		state->textTransformDelta(0, state->getFontSize() * font->getAscent(), & x, & y);
		state->transform(state->getCurX() + x, state->getCurY() + y, & rc->xright, & rc->yright);

		// return changed state
		return state;
	}	


	// "m"
	GfxState *
	opmUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (2 <= args.size());
		utilsPrintDbg (debug::DBG_DBG, "");
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state->moveTo (getDoubleFromIProperty (args[0]), getDoubleFromIProperty (args[1]));
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);

		// return changed state
		return state;
	}
	// "Td"
	GfxState *
	opTdUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (2 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		double tx = state->getLineX() + getDoubleFromIProperty (args[0]);
		double ty = state->getLineY() + getDoubleFromIProperty (args[1]);
		state->textMoveTo(tx, ty);
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);

		// return changed state
		return state;
	}
	// "Tm"
	GfxState *
	opTmUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (6 <= args.size ());
		utilsPrintDbg (debug::DBG_DBG, "");
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state->setTextMat (	getDoubleFromIProperty (args[0]), 
							getDoubleFromIProperty (args[1]),
							getDoubleFromIProperty (args[2]), 
							getDoubleFromIProperty (args[3]),
							getDoubleFromIProperty (args[4]), 
							getDoubleFromIProperty (args[5])
						  );
		state->textMoveTo(0, 0);

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);

		// return changed state
		return state;
	}
	// "BT"
	GfxState *
	opBTUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		utilsPrintDbg (debug::DBG_DBG, "");
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state->setTextMat (1, 0, 0, 1, 0, 0);
		state->textMoveTo (0, 0);

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);

		// return changed state
		return state;
	}
	// "Do"
	GfxState *
	opDoUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state->transform(0, 0, & rc->xleft, & rc->yleft);
		
		// Set edge of rectangle from actual position + [1,1] on output devices
		state->transform(1, 1, & rc->xright, & rc->yright);

		// return changed state
		return state;
	}
	// "l"
	GfxState *
	oplUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (2 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);
		
		state->lineTo (getDoubleFromIProperty(args[0]), getDoubleFromIProperty (args[1]));

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);

		// return changed state
		return state;
	}
	// "c"
	GfxState *
	opcUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (6 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		state->curveTo ( getDoubleFromIProperty(args[0]), getDoubleFromIProperty(args[1]),
						getDoubleFromIProperty(args[2]), getDoubleFromIProperty(args[3]),
						getDoubleFromIProperty(args[4]), getDoubleFromIProperty(args[5]));

		// simple calculate boundingbox of curve (inexact)
		Point h_width;
		state->transformDelta( state->getLineWidth()/2, state->getLineWidth()/2, & h_width.x, & h_width.y );
		rc->xleft -= h_width.x;
		rc->yleft -= h_width.y;
		rc->xright += h_width.x;
		rc->yright += h_width.y;
		Point h_pt;
		for (int i=0; i<6 ;i+=2) {
			state->transform(getDoubleFromIProperty(args[i]), getDoubleFromIProperty(args[i+1]), & h_pt.x, & h_pt.y);
			rc->xleft = min( rc->xleft, h_pt.x - h_width.x );
			rc->xright = max( rc->xright, h_pt.x + h_width.x );
			rc->yleft = min( rc->yleft, h_pt.y - h_width.y );
			rc->yright = max( rc->yright, h_pt.y + h_width.y );
		}

		// return changed state
		return state;
	}
	// "v"
	GfxState *
	opvUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (4 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		state->curveTo ( state->getCurX(), state->getCurY(), 
						getDoubleFromIProperty(args[0]),
						getDoubleFromIProperty(args[1]),
						getDoubleFromIProperty(args[2]),
						getDoubleFromIProperty(args[3])
					  );

		// simple calculate boundingbox of curve (inexact)
		Point h_width;
		state->transformDelta( state->getLineWidth()/2, state->getLineWidth()/2, & h_width.x, & h_width.y );
		rc->xleft -= h_width.x;
		rc->yleft -= h_width.y;
		rc->xright += h_width.x;
		rc->yright += h_width.y;
		Point h_pt;
		for (int i=0; i<4 ;i+=2) {
			state->transform(getDoubleFromIProperty(args[i]), getDoubleFromIProperty(args[i+1]), & h_pt.x, & h_pt.y);
			rc->xleft = min( rc->xleft, h_pt.x - h_width.x );
			rc->xright = max( rc->xright, h_pt.x + h_width.x );
			rc->yleft = min( rc->yleft, h_pt.y - h_width.y );
			rc->yright = max( rc->yright, h_pt.y + h_width.y );
		}

		// return changed state
		return state;
	}
	// "y"
	GfxState *
	opyUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (4 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		state->curveTo ( getDoubleFromIProperty(args[0]),
						getDoubleFromIProperty(args[1]),
						getDoubleFromIProperty(args[2]),
						getDoubleFromIProperty(args[3]),
						getDoubleFromIProperty(args[2]),
						getDoubleFromIProperty(args[3])
					  );

		// simple calculate boundingbox of curve (inexact)
		Point h_width;
		state->transformDelta( state->getLineWidth()/2, state->getLineWidth()/2, & h_width.x, & h_width.y );
		rc->xleft -= h_width.x;
		rc->yleft -= h_width.y;
		rc->xright += h_width.x;
		rc->yright += h_width.y;
		Point h_pt;
		for (int i=0; i<4 ;i+=2) {
			state->transform(getDoubleFromIProperty(args[i]), getDoubleFromIProperty(args[i+1]), & h_pt.x, & h_pt.y);
			rc->xleft = min( rc->xleft, h_pt.x - h_width.x );
			rc->xright = max( rc->xright, h_pt.x + h_width.x );
			rc->yleft = min( rc->yleft, h_pt.y - h_width.y );
			rc->yright = max( rc->yright, h_pt.y + h_width.y );
		}

		// return changed state
		return state;
	}
	// "re"
	GfxState *
	opreUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (4 <= args.size ());
		double x = getDoubleFromIProperty(args[0]);
		double y = getDoubleFromIProperty(args[1]);
		double w = getDoubleFromIProperty(args[2]);
		double h = getDoubleFromIProperty(args[3]);
		state->moveTo(x, y);
		state->lineTo(x + w, y);
		state->lineTo(x + w, y + h);
		state->lineTo(x, y + h);
		state->closePath();

		// Set rectangle from operands position on output devices
		state->transform(x - state->getLineWidth()/2, y - state->getLineWidth()/2, & rc->xleft, & rc->yleft);
		state->transform(x+w+ state->getLineWidth()/2, y+h+ state->getLineWidth()/2, & rc->xright, & rc->yright);
		Point h_pt;
		state->transform(x - state->getLineWidth()/2, y+h+ state->getLineWidth()/2, & h_pt.x, & h_pt.y);
			rc->xleft = min( rc->xleft, h_pt.x );
			rc->xright = max( rc->xright, h_pt.x );
			rc->yleft = min( rc->yleft, h_pt.y );
			rc->yright = max( rc->yright, h_pt.y );
		state->transform(x+w+ state->getLineWidth()/2, y - state->getLineWidth()/2, & h_pt.x, & h_pt.y);
			rc->xleft = min( rc->xleft, h_pt.x );
			rc->xright = max( rc->xright, h_pt.x );
			rc->yleft = min( rc->yleft, h_pt.y );
			rc->yright = max( rc->yright, h_pt.y );
		
		// return changed state
		return state;
	}
	// "h"
	GfxState *
	ophUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state->closePath ();

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}
	// "Tc"
	GfxState *
	opTcUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

  		state->setCharSpace (getDoubleFromIProperty(args[0]));

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}
	// "Tf"
	GfxState *
	opTfUpdate (GfxState* state, boost::shared_ptr<GfxResources> res, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (2 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		assert( isName( args[0] ) );
		std::string val;
		IProperty::getSmartCObjectPtr<CName> (args[0]) ->getValue (val);

		GfxFont *font = NULL;

		if (!(font = res.get()->lookupFont (val.c_str())))
			return state;		// same as displaing with xpdf/Gfx
		
		state->setFont (font, getDoubleFromIProperty (args[1]));
		
		// return changed state
		return state;
	}
	// "Ts"
	GfxState *
	opTsUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state->setRise (getDoubleFromIProperty (args[0])); 

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}
	// "Tw"
	GfxState *
	opTwUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

  		state->setWordSpace (getDoubleFromIProperty (args[0]));

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}
	// "Tz"
	GfxState *
	opTzUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state->setHorizScaling (getDoubleFromIProperty (args[0]));

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}
	// "TD"
	GfxState *
	opTDUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (2 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		double tx = state->getLineX() + getDoubleFromIProperty (args[0]);
		double ty = getDoubleFromIProperty (args[1]);
		state->setLeading(-ty);
		ty += state->getLineY();
		state->textMoveTo(tx, ty);

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}
	// "T*"
	GfxState *
	opTstarUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		double tx = state->getLineX();
		double ty = state->getLineY() - state->getLeading();
		state->textMoveTo (tx, ty);

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}
	// "Tj"
	GfxState *
	opTjUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size ());

		// This can happen in really damaged pdfs
 		if (state->getFont())
			printTextUpdate (state, getStringFromIProperty (args[0]), rc);
		
		// return changed state
		return state;
	}
	// "'"
	GfxState *
	opApoUpdate (GfxState* state, boost::shared_ptr<GfxResources> res, const boost::shared_ptr<PdfOperator> op, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size ());

		// Set edge of rectangle from actual position on output devices
		//state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state = opTstarUpdate( state, res, op, args, rc );
		state = opTjUpdate( state, res, op, args, rc );		// to 'rc' has saved only text bbox

		// Set edge of rectangle from actual position on output devices
		//state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}
	// "TL"
	GfxState *
	opTLUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size ());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

  		state->setLeading ( getDoubleFromIProperty (args[0]));

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}
	// "\"
	GfxState *
	opSlashUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (3 <= args.size ());
		
		// This can happen in really damaged pdfs
		if (!state->getFont())	// No font for text
			return state;
		
		// Set edge of rectangle from actual position on output devices
		//state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state->setWordSpace (getDoubleFromIProperty (args[0]));
		state->setCharSpace (getDoubleFromIProperty (args[1]));
		double tx = state->getLineX();
		double ty = state->getLineY() - state->getLeading();
		state->textMoveTo(tx, ty);

 		printTextUpdate (state, getStringFromIProperty (args[2]), rc);	// to 'rc' save only text bbox

		// Set edge of rectangle from actual position on output devices
		//state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}
	// "TJ"
	GfxState *
	opTJUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size ());

		// This can happen in really damaged pdfs
		if (!state->getFont()) 
			return state;

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		if (!isArray(args[0]))
		{
			assert (!"opTJUpdate: Invalid first argument.");
			throw ElementBadTypeException ("opTJUpdate: Object in bad state->");
		}

		int wMode;
		wMode = state->getFont()->getWMode();

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
					state->transform(state->getCurX (), state->getCurY(), & h_rc.xleft, & h_rc.yleft);

					double dd = -getDoubleFromIProperty (item);
					if (wMode) {
						state->textShift(0, dd * 0.001 * fabs(state->getFontSize()));
					} else {
						state->textShift(dd * 0.001 * fabs(state->getFontSize()), 0);
					}

					// Set edge of rectangle from actual position on output devices
					state->transform(state->getCurX (), state->getCurY(), & h_rc.xright, & h_rc.yright);
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
		
		// return changed state
		return state;
	}
	// "cm"
	GfxState *
	opcmUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (6 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state->concatCTM (
				getDoubleFromIProperty(args[0]),
				getDoubleFromIProperty(args[1]),
				getDoubleFromIProperty(args[2]),
				getDoubleFromIProperty(args[3]),
				getDoubleFromIProperty(args[4]),
				getDoubleFromIProperty(args[5]));

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}

	// "Q"
	GfxState *
	opQUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state = state->restore();

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}

	// "q"
	GfxState *
	opqUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state = state->save();

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}

	// "Tr"
	GfxState *
	opTrUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		assert (1 <= args.size());
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		state->setRender( getIntFromIProperty(args[0]) );

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}

	// "w"
	GfxState *
	opwUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);
		rc->xright = rc->xleft;
		rc->yright = rc->yleft;

		state->setLineWidth( getDoubleFromIProperty(args[0]) );
		
		// return changed state
		return state;
	}
	
	// "BI"
	GfxState *
	opBIUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator> op, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		utilsPrintDbg (DBG_DBG, "NOT IMPLEMENTED");
		utilsPrintDbg (DBG_DBG, " " << op->getParametersCount() << " , " << op->getChildrenCount());

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}

	// "ID"
	GfxState *
	opIDUpdate (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator> op, const PdfOperator::Operands&, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);

		utilsPrintDbg (DBG_DBG, "NOT IMPLEMENTED");
		utilsPrintDbg (DBG_DBG, " " << op->getParametersCount() << " , " << op->getChildrenCount());

		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}

/*	// ""
	GfxState *
	op (GfxState* state, boost::shared_ptr<GfxResources>, const boost::shared_ptr<PdfOperator>, const PdfOperator::Operands& args, Rectangle* rc)
	{
		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);


		// Set edge of rectangle from actual position on output devices
		state->transform(state->getCurX (), state->getCurY(), & rc->xright, & rc->yright);
		
		// return changed state
		return state;
	}
*/
	//==========================================================
	// End of actual state (position) updaters
	//==========================================================

//==========================================================
} // namespace
//==========================================================


//
//
//
StateUpdater::CheckTypes StateUpdater::KNOWN_OPERATORS[] = {
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
			opDoUpdate, "" },	
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


//
//
//
GfxState *
StateUpdater::unknownUpdate (GfxState* state, 
							boost::shared_ptr<GfxResources>, 
							const boost::shared_ptr<PdfOperator>, 
							const PdfOperator::Operands&, Rectangle* rc)
{
	// Set rectangle from actual position on output devices
	state->transform(state->getCurX (), state->getCurY(), & rc->xleft, & rc->yleft);
	rc->xright = rc->xleft;
	rc->yright = rc->yleft;
	/* utilsPrintDbg (debug::DBG_DBG, "Unknown updater.");*/

	// return changed state
	return state;
}


//
//
//
const StateUpdater::CheckTypes*
StateUpdater::findOp (const string& opName) const
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

//
//
//
string
StateUpdater::getEndTag (const string& name) const
{
	const StateUpdater::CheckTypes* chcktp = findOp (name);
	return string (chcktp->endTag);
}

//==========================================================
} // namespace pdfobjects
//==========================================================
