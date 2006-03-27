/*
 * =====================================================================================
 *        Filename:  ccontentstream.cc
 *         Created:  03/24/2006 10:33:34 PM CET
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

// static
#include "static.h"
//
#include "cobject.h"
#include "ccontentstream.h"


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
	// Known operators, extern is a must here
	//
	extern const char OPER_MOVETONEXTLINE [] 		= "'"; 
	extern const char OPER_FILLSTROKE [] 			= "B"; 
	extern const char OPER_EOFILLSTROKE [] 			= "B*"; 
	//extern const char OPER_FILLSTROKE [] = "123" ; 
	//extern const char OPER_EOFILLSTROKE [] = "123"; 

	/**
	 *
	 */
	class KnownOperators
	{
		public:
			/**
			 * Structure representing operator and function for creating specified operator.
			 *
			 * It is represented in a structure to optimize searching in throught the text 
			 * representing operators.
			 * 
			 */
			typedef struct
			{
				const char* name;					// Operator
				shared_ptr<PdfOperator> (*func) 	// Creator function
						(PdfOperator::Operands& operands);
				
			} OperatorTab;


		private:
			static const OperatorTab tab[];
			
		public:

			/**
			 * Find the operator and create it.
			 *
			 * @param op String representation of the operator. It is used to find the operator and
			 * 			 sometimes to initialize it.
			 * @param operands Operand stack.
			 *
			 * @return Created pdf operator.
			 */
			static shared_ptr<PdfOperator>
			findOp (const string& op, PdfOperator::Operands& operands);

			//
			// Creators
			//
			typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_MOVETONEXTLINE> 
					COperMoveShowText;
			static shared_ptr<PdfOperator> createOperatorMoveShowText
				(PdfOperator::Operands& operands)
					{ return shared_ptr<PdfOperator> (new COperMoveShowText (operands)); };



	};

//typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>,OPER_FILLSTROKE> 		COperFillStroke;
//typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>,OPER_EOFILLSTROKE> 		COperEoFillStroke;

	//
	// Table specifying known operators
	//
	const KnownOperators::OperatorTab KnownOperators::tab[] = { 

		{OPER_MOVETONEXTLINE, KnownOperators::createOperatorMoveShowText},
	
	};

	//
	// NEEDS to be AFTER the declaration of tab
	//
	shared_ptr<PdfOperator>
	KnownOperators::findOp (const string& op, PdfOperator::Operands& operands) 
	{
		printDbg (DBG_DBG, "Finding operator: " << op);

		int lo, hi, med, cmp;
		
		cmp = std::numeric_limits<int>::max ();
		lo = -1;
		hi = sizeof (tab) / sizeof (OperatorTab);
		
		// 
		// dividing of interval
		// 
		while (hi - lo > 1) 
		{
			med = (lo + hi) / 2;
			cmp = op.compare (tab[med].name);
			if (cmp > 0)
				lo = med;
			else if (cmp < 0)
				hi = med;
			else
				lo = hi = med;
		}
		if (0 != cmp)
		{
			printDbg (DBG_DBG, "Operator not found.");
			return shared_ptr<UnknownPdfOperator> (new UnknownPdfOperator (operands, op));
		}
				
		printDbg (DBG_DBG, "Operator found. tab[" << lo << "]");
		// Result in lo
		return tab[lo].func (operands);
  	}

	
	
  /*{"B",   0, {tchkNone},
          &Gfx::opFillStroke},
  {"B*",  0, {tchkNone},
          &Gfx::opEOFillStroke},
  {"BDC", 2, {tchkName,   tchkProps},
          &Gfx::opBeginMarkedContent},
  {"BI",  0, {tchkNone},
          &Gfx::opBeginImage},
  {"BMC", 1, {tchkName},
          &Gfx::opBeginMarkedContent},
  {"BT",  0, {tchkNone},
          &Gfx::opBeginText},
  {"BX",  0, {tchkNone},
          &Gfx::opBeginIgnoreUndef},
  {"CS",  1, {tchkName},
          &Gfx::opSetStrokeColorSpace},
  {"DP",  2, {tchkName,   tchkProps},
          &Gfx::opMarkPoint},
  {"Do",  1, {tchkName},
          &Gfx::opXObject},
  {"EI",  0, {tchkNone},
          &Gfx::opEndImage},
  {"EMC", 0, {tchkNone},
          &Gfx::opEndMarkedContent},
  {"ET",  0, {tchkNone},
          &Gfx::opEndText},
  {"EX",  0, {tchkNone},
          &Gfx::opEndIgnoreUndef},
  {"F",   0, {tchkNone},
          &Gfx::opFill},
  {"G",   1, {tchkNum},
          &Gfx::opSetStrokeGray},
  {"ID",  0, {tchkNone},
          &Gfx::opImageData},
  {"J",   1, {tchkInt},
          &Gfx::opSetLineCap},
  {"K",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeCMYKColor},
  {"M",   1, {tchkNum},
          &Gfx::opSetMiterLimit},
  {"MP",  1, {tchkName},
          &Gfx::opMarkPoint},
  {"Q",   0, {tchkNone},
          &Gfx::opRestore},
  {"RG",  3, {tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeRGBColor},
  {"S",   0, {tchkNone},
          &Gfx::opStroke},
  {"SC",  -4, {tchkNum,   tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeColor},
  {"SCN", -5, {tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	       tchkSCN},
          &Gfx::opSetStrokeColorN},
  {"T*",  0, {tchkNone},
          &Gfx::opTextNextLine},
  {"TD",  2, {tchkNum,    tchkNum},
          &Gfx::opTextMoveSet},
  {"TJ",  1, {tchkArray},
          &Gfx::opShowSpaceText},
  {"TL",  1, {tchkNum},
          &Gfx::opSetTextLeading},
  {"Tc",  1, {tchkNum},
          &Gfx::opSetCharSpacing},
  {"Td",  2, {tchkNum,    tchkNum},
          &Gfx::opTextMove},
  {"Tf",  2, {tchkName,   tchkNum},
          &Gfx::opSetFont},
  {"Tj",  1, {tchkString},
          &Gfx::opShowText},
  {"Tm",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opSetTextMatrix},
  {"Tr",  1, {tchkInt},
          &Gfx::opSetTextRender},
  {"Ts",  1, {tchkNum},
          &Gfx::opSetTextRise},
  {"Tw",  1, {tchkNum},
          &Gfx::opSetWordSpacing},
  {"Tz",  1, {tchkNum},
          &Gfx::opSetHorizScaling},
  {"W",   0, {tchkNone},
          &Gfx::opClip},
  {"W*",  0, {tchkNone},
          &Gfx::opEOClip},
  {"b",   0, {tchkNone},
          &Gfx::opCloseFillStroke},
  {"b*",  0, {tchkNone},
          &Gfx::opCloseEOFillStroke},
  {"c",   6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opCurveTo},
  {"cm",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opConcat},
  {"cs",  1, {tchkName},
          &Gfx::opSetFillColorSpace},
  {"d",   2, {tchkArray,  tchkNum},
          &Gfx::opSetDash},
  {"d0",  2, {tchkNum,    tchkNum},
          &Gfx::opSetCharWidth},
  {"d1",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opSetCacheDevice},
  {"f",   0, {tchkNone},
          &Gfx::opFill},
  {"f*",  0, {tchkNone},
          &Gfx::opEOFill},
  {"g",   1, {tchkNum},
          &Gfx::opSetFillGray},
  {"gs",  1, {tchkName},
          &Gfx::opSetExtGState},
  {"h",   0, {tchkNone},
          &Gfx::opClosePath},
  {"i",   1, {tchkNum},
          &Gfx::opSetFlat},
  {"j",   1, {tchkInt},
          &Gfx::opSetLineJoin},
  {"k",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillCMYKColor},
  {"l",   2, {tchkNum,    tchkNum},
          &Gfx::opLineTo},
  {"m",   2, {tchkNum,    tchkNum},
          &Gfx::opMoveTo},
  {"n",   0, {tchkNone},
          &Gfx::opEndPath},
  {"q",   0, {tchkNone},
          &Gfx::opSave},
  {"re",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opRectangle},
  {"rg",  3, {tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillRGBColor},
  {"ri",  1, {tchkName},
          &Gfx::opSetRenderingIntent},
  {"s",   0, {tchkNone},
          &Gfx::opCloseStroke},
  {"sc",  -4, {tchkNum,   tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillColor},
  {"scn", -5, {tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	       tchkSCN},
          &Gfx::opSetFillColorN},
  {"sh",  1, {tchkName},
          &Gfx::opShFill},
  {"v",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opCurveTo1},
  {"w",   1, {tchkNum},
          &Gfx::opSetLineWidth},
  {"y",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opCurveTo2},
*/

//==========================================================
} // namespace
//==========================================================




//
// Parse the xpdf object, representing the content stream
//
CContentStream::CContentStream (shared_ptr<IProperty> stream, Object* obj) : contentstream (stream)
{
	// not implemented yet
	assert (obj != NULL);

	printDbg (DBG_DBG, "Creating content stream.");
	
	//
	// Create the parser and lexer and get objects from it
	//
	scoped_ptr<Parser> parser (new Parser (NULL, new Lexer(NULL, obj)));

	PdfOperator::Operands operands;
	
	Object o;
	parser->getObj(&o);

	//
	// Loop through all object, if it is an operator create pdfoperator else assume it is an operand
	//
	while (!o.isEOF()) 
	{
		if (o.isCmd ())
		{
			// Create operator
			operators.push_back (KnownOperators::findOp (string (o.getCmd ()), operands));
			
			assert (operands.empty());
			if (!operands.empty ())
				throw MalformedFormatExeption ("Operands left on stack in pdf content stream after operator.");
				
		}else
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

//
// 
//
void
CContentStream::getStringRepresentation (string& str) const
{
	printDbg (DBG_DBG, " ()");
	string frst, lst, tmp;

	str = "";
	for (Operators::const_iterator it = operators.begin (); it != operators.end(); ++it)
	{
			
		(*it)->getOperatorName (frst, lst);
		printDbg (DBG_DBG, "Operator name: " << frst << " " << lst << " param count: " << (*it)->getParametersCount() );
		
		(*it)->getStringRepresentation (tmp);
		str += tmp + "\n";
		tmp = "";
	}
}

//==========================================================
} // namespace pdfobjects
//==========================================================
