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
#include "stateupdater.h"
#include "cobject.h"
#include "factories.h"
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

	
	/**
	 * Set pdf, indiref, contentstream to operands, alse register observers on all operands.
	 *
	 * This is vital when operands are changed.
	 *
	 * @param first First operator to set pdf to.
	 * @param pdf Valid pdf where operands belong.
	 * @param rf  Valid Indiref of a cstream parent.
	 * @param cs Content stream where the operator is.
	 * @param observer Operand observer.
	 */
	void
	opsSetPdfRefCs (shared_ptr<PdfOperator> first, 
					CPdf& pdf, 
					IndiRef rf, 
					CContentStream& cs, 
					boost::shared_ptr<IIPropertyObserver> observer)
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
			
			//
			// Set valid pdf and ref but lock the change 
			// 	-- cstream won't get notified by its children, not needed
			// 
			PdfOperator::Operands::iterator oper = operands.begin ();
			for (; oper != operands.end (); ++oper)
			{
				if (hasValidPdf(*oper))
				{ // We do not support adding operators from another stream so if
				  // it has valid pdf, it has to be the same
					if ( ((*oper)->getPdf() != &pdf) || !((*oper)->getIndiRef() == rf) )
					{
						kernelPrintDbg (debug::DBG_CRIT, "Pdf or indiref do not match: want " << rf << 
														 " op has" <<(*oper)->getIndiRef());
						throw CObjInvalidObject ();
					}
				
					
				}else
				{
					(*oper)->setPdf (&pdf);
					(*oper)->setIndiRef (rf);
					(*oper)->registerObserver (observer);
					(*oper)->lockChange ();
				}
			}
			
			it = it.next ();
		}
	}
	
	/**
	 * Check if the operands match the specification and replace operand with
	 * its stronger equivalent.
	 *
	 * (e.g. When xpdf returns an object with integer type, but the operand can be a real, we have to
	 * convert it to real.)
	 *
	 * @param ops Operator specification
	 * @param operands Operand stack.
	 *
	 * @return True if type and count match, false otherwise.
	 */
	bool checkAndFix (const StateUpdater::CheckTypes& ops, PdfOperator::Operands& operands)
	{
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
		PdfOperator::Operands::reverse_iterator rit = operands.rbegin ();
		// Be careful -- buffer overflow
		argNum = std::min (argNum, operands.size());
		advance (rit, argNum);
		PdfOperator::Operands::iterator it = rit.base ();
		// Loop from the first operator to the end
		for (int pos = 0; it != operands.end (); ++it, ++pos)
		{			
  			if (!isBitSet(ops.types[pos], (*it)->getType()))
			{
				utilsPrintDbg (DBG_ERR, "Bad " << pos << "-th operand type [" << (*it)->getType() << "] " << hex << " 0x" << ops.types[pos]);
				return false;
			}

			// 
			// If xpdf returned an Int, bu the operand can be a real convert it
			// 
			if (isInt(*it))
			{
  				if (isBitSet(ops.types[pos], pReal))
				{ // Convert it to real
					double dval = 0.0;
					dval = IProperty::getSmartCObjectPtr<CInt>(*it)->getValue();
					shared_ptr<IProperty> pIp (new CReal (dval));
					std::replace (operands.begin(), operands.end(), *it, pIp);
				}
			}
		}

		return true;
	}

	
	/**
	 * Parse inline image. 
	 *
	 * Inline image is a stream withing anothers stream. It has to be treated
	 * separately because it is the only case when another stream is present in
	 * a content stream.
	 * Binary data can make text parser to behave incorrectly.
	 *
	 * @param streamreader Actual parser.
	 *
	 * @return CStream representing inline image.
	 */
	CInlineImage*
	getInlineImage (CStreamsXpdfReader<CContentStream::CStreams>& streamreader) 
	{
		kernelPrintDbg (DBG_DBG, "");
		::Object dict;
		dict.initDict ((XRef*)NULL); // We do not have (need) valid xref, but be CAREFUL

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
				dict.dictAdd (key, &o);
			
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
		boost::scoped_ptr<Stream> str (new ::EmbedStream (streamreader.getXpdfStream (), &dict, gFalse, 0));
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
			return new CInlineImage (dict, buf);
			// dict will get deallocated when str gets deallocated
				
		}else
		{
			assert (!"Bad embedded stream.");
			throw CObjInvalidObject ();
		}
	}

	/**
	 * Create simple operator and its operands.
	 *
	 * @return True if everything ok, false if end of stream reached.
	 */
	bool
	createOperands (CStreamsXpdfReader<CContentStream::CStreams>& streamreader, 
					PdfOperator::Operands& operands,
					xpdf::XpdfObject& o)
	{
		// Get first object
		streamreader.getXpdfObject (*o);

		//
		// Loop through all object, if it is an operator create pdfoperator else assume it is an operand
		//
		while (!streamreader.eof()) 
		{
			if (o->isCmd ())
			{// We have an OPERATOR
				return true;
			
			}else 
			{// We have an OPERAND
				
				shared_ptr<IProperty> pIp (createObjFromXpdfObj (*o));
				operands.push_back (pIp);
			}

			o->free ();
			// Grab the next object
			streamreader.getXpdfObject (*o);

		} // while
		
		return false;
	}
	
	/**
	 * Create operator from xpdf object.
	 *
	 * @param streamreader CStreams parser from which we get an xpdf object.
	 * @param operands Operands of operator. They are shared through subcalls.
	 */
	shared_ptr<PdfOperator>
	createOperator (CStreamsXpdfReader<CContentStream::CStreams>& streamreader, 
					PdfOperator::Operands& operands)
	{
		// Get operands
		xpdf::XpdfObject o;
		if (!createOperands (streamreader, operands, o))
			return shared_ptr<PdfOperator> ();
		
		// Try to find the op by its name
		const StateUpdater::CheckTypes* chcktp = StateUpdaterFactory::getInstance()->findOp (o->getCmd());
		// Operator not found, create unknown operator
		if (NULL == chcktp)
			return shared_ptr<PdfOperator> (new SimpleGenericOperator (string (o->getCmd()),operands));
		
		assert (chcktp);
		utilsPrintDbg (DBG_DBG, "Operator found. " << chcktp->name);

		//
		// Check the type against specification
		// 
		if (!checkAndFix (*chcktp, operands))
		{
			//assert (!"Content stream bad operator type.");
			throw ElementBadTypeException ("Content stream operator has incorrect operand type.");
		}

		//
		// SPECIAL CASE for inline image (stream within a text stream)
		//
		if ( 0 == strncmp (chcktp->name, "BI", 2))
		{
			utilsPrintDbg (debug::DBG_DBG, "");
			
			shared_ptr<CInlineImage> inimg (getInlineImage (streamreader));
			return shared_ptr<PdfOperator> (new InlineImageCompositePdfOperator (chcktp->name, chcktp->endTag, inimg));
		}
		
		// Get operands count
		size_t argNum = static_cast<size_t> ((chcktp->argNum > 0) ? chcktp->argNum : -chcktp->argNum);

		//
		// If endTag is "" it is a simple operator, composite otherwise
		// 
		if (isSimpleOp(*chcktp))
			return shared_ptr<PdfOperator> (new SimpleGenericOperator (chcktp->name, argNum, operands));
			
		else // Composite operator
			return shared_ptr<PdfOperator> (new UnknownCompositePdfOperator (chcktp->name, chcktp->endTag));	
	}
	
	/**
	 * Create an operator from a stream. 
	 *
	 * First read its operands, then the operator itself. Inline image is a
	 * special case.
	 *
	 * This function is called recursively to create the tree like structure of
	 * pdf operators
	 *
	 * @param streamreader CStreams parser from which we get an xpdf object.
	 * @param operands Operands of operator. They are shared through subcalls.
	 *
	 * @return New pdf operator.
	 */
	shared_ptr<PdfOperator>
	parseOp (CStreamsXpdfReader<CContentStream::CStreams>& streamreader, PdfOperator::Operands& operands)
	{
		// Create operator with its operands
		shared_ptr<PdfOperator> result = createOperator (streamreader, operands);
	
		if (result && isCompositeOp (result) && !isInlineImageOp (result))
		{
			string opname;
			result->getOperatorName (opname);
			string endtag = StateUpdaterFactory::getInstance()->getEndTag (opname);
			bool foundEndTag = false;
			
			// The same as in (re)parseContentStream
			shared_ptr<PdfOperator> newop, previousLast = result;

			//
			// Use recursion to get all operators
			//
			while (newop=parseOp (streamreader, operands))
			{
				result->push_back (newop, previousLast);

				// Is it the end tag?
				string tag;
				newop->getOperatorName (tag);
				if (tag == endtag)
				{
					foundEndTag = true;
					break;
				}
				
				// Save last as previous
				previousLast = getLastOperator (newop);
			}

			if (!foundEndTag)
			{
				//assert (!"Bad content stream while reparsing. End tag was not found.");
				throw CObjInvalidObject ();
			}
		}
		
		return result;
	}

	/**
	 * Parse the stream for the first time into pdf operators. 
	 *
	 * Problem with content stream is, that it can be splitted in many streams
	 * and the split points are really insane. And moreover some pdf creators 
	 * produce even more insane split points. So we try to parse a valid stream
	 * (all composite objects are ended with their ending tags) and look if we
	 * are at the end of one of the streams. If positive, we claim this content
	 * stream to be a valid one.
	 * 
	 * @param operators Operator stack.
	 * @param streams 	Streams to be parsed.
	 * @param cs 		Content stream in which operators belong
	 * @param observer 	Operand observer.
	 * @param parsedstreams Streams that have been really parsed.
	 */
	void
	parseContentStream (CContentStream::Operators& operators, 
						CContentStream::CStreams& streams, 
						CContentStream& cs,
						boost::shared_ptr<IIPropertyObserver> observer,
						CContentStream::CStreams* parsedstreams = NULL)
	{
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
		CStreamsXpdfReader<CContentStream::CStreams> streamreader (streams);
		streamreader.open ();
	
		PdfOperator::Operands operands;
		shared_ptr<PdfOperator> topoperator (new UnknownCompositePdfOperator ("",""));	
		shared_ptr<PdfOperator> newop, previousLast = topoperator;

		//
		// Parsing can throw, if so the stream is invalid
		// 	-- copy them to parsed streams any way
		//
		try 
		{
			while (newop=parseOp (streamreader, operands))
			{
				topoperator->push_back (newop, previousLast);
				previousLast = getLastOperator (newop);

				if (parsedstreams)
				{ // FIRST PARSE

					// We have found complete and correct content stream.
					if (streamreader.eofOfActualStream())
						break;				
				}
			}

		}catch (CObjectException&)
		{
			kernelPrintDbg (debug::DBG_ERR, "Invalid content stream...");
			operands.clear ();
			//operators.clear ();
			/** \todo SET INVALID CONTENTSTREAM. */
		}
		
		//
		// Copy operands
		//
		topoperator->getChildren (operators);
		// Set prev of first valid operator to NULL
		if (!operators.empty())
			PdfOperator::getIterator (topoperator).next().getCurrent()->setPrev (PdfOperator::ListItem ());

		// Delete topoperator
		topoperator.reset();

		if (parsedstreams)  // Save which streams were parsed and close
			streamreader.close (*parsedstreams);
		else
			streamreader.close ();


		if (parsedstreams)
		{ // FIRST PARSE
	
			// Check if they match the input streams and delete them
			CContentStream::CStreams::iterator itparsed = parsedstreams->begin (); 
			for (; itparsed != parsedstreams->end(); ++itparsed)
			{			
				assert (*(streams.begin()) == *itparsed);
				//streams.erase (streams.begin());
				streams.pop_front();
			}
		}

		assert (operands.empty());
		assert (observer);
		// Set pdf ref and cs
		if (!operators.empty())
			opsSetPdfRefCs (operators.front(), *pdf, rf, cs, observer);
	}


	//==========================================================
	// Gfx state updater functors
	//==========================================================

	/**
 	 * BBox updater.
	 */
	struct BBoxUpdater 
	{
		void operator() (shared_ptr<PdfOperator> op, const GfxState&, Rectangle rc) const
		{
			// If not initialized, means an error occured (missing font etc..)
			if (!Rectangle::isInitialized (rc))
				rc.xleft = rc.xright = rc.yleft = rc.yright = 0;
			op->setBBox (rc);
		}
	};

	
//==========================================================
} // namespace
//==========================================================

//==========================================================
// Observer interface
//==========================================================

//
// CContentStream observer
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
	// Check if the property has correct values if it exists (it could be
	// deleted and newValue is NULL
	if (newValue)
	{
		assert (hasValidPdf (newValue));
		assert (hasValidRef (newValue));
	}

	// Stream has changed, reparse it
	contentstream->reparse ();
	
	}catch (...)
	{
		assert (!"This is very very bad because this function can't throw according to the interface.");
	}
}

//
// Operand observer
//

//
//
//
void 
CContentStream::OperandObserver::notify (boost::shared_ptr<IProperty> newValue, 
										  boost::shared_ptr<const IProperty::ObserverContext>) const 
throw ()
{
	try {

	utilsPrintDbg (debug::DBG_DBG, "");
	
	assert (newValue);
	if (!isNull(newValue)) 
	{ // We could have deleted a property
		assert (hasValidPdf (newValue));
		assert (hasValidRef (newValue));
	}

	// Stream has changed, reparse it
	contentstream->saveChange ();
	
	// Reparse bounding box
	contentstream->reparse (true);
	
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
	
	// Create cstream observer and register it on all operands
	cstreamobserver = boost::shared_ptr<CStreamObserver> (new CStreamObserver (this));
	// Create operand observer
	operandobserver = boost::shared_ptr<OperandObserver> (new OperandObserver (this));
	
	// Parse it, move parsed straems from strs to cstreams
	parseContentStream (operators, strs, *this, operandobserver, &cstreams);
	
	// Save bounding boxes
	if (!operators.empty()) 
		StateUpdaterFactory::getInstance()->updatePdfOperators (PdfOperator::getIterator (operators.front()), gfxres, *gfxstate, BBoxUpdater());

	// Register observer on all cstream
	registerCStreamObservers ();
}


//
// Helper methods
//

//
//
//
void
CContentStream::reparse (bool bboxOnly, boost::shared_ptr<GfxState> state, boost::shared_ptr<GfxResources> res)
{
	// Save resources if new
	if (state)
		gfxstate = state;
	if (res)
		gfxres = res;
	
	assert (gfxres);
	assert (gfxstate);
	
	// Reparse it if needed
	if (!bboxOnly)
	{
		// Clear operators	
		operators.clear ();
		parseContentStream (operators, cstreams, *this, operandobserver);
	}
	
	// Save bounding boxes
	if (!operators.empty()) 
		StateUpdaterFactory::getInstance()->updatePdfOperators (PdfOperator::getIterator (operators.front()), gfxres, *gfxstate, BBoxUpdater());
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
	unregisterCStreamObservers ();

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
	registerCStreamObservers ();
	
	// Notify observers
	boost::shared_ptr<CContentStream> current (this, EmptyDeallocator<CContentStream> ());
	this->notifyObservers (current, shared_ptr<const ObserverContext> (new BasicObserverContext (current)));
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
	boost::shared_ptr<PdfOperator> nxt, prv;
	if (!itNxt.isEnd())
		nxt = itNxt.getCurrent ();
	if (!itPrv.isBegin())
		prv = itPrv.getCurrent ();
	
	if (!itNxt.isEnd())
		itNxt.getCurrent()->setPrev (prv);
	if (!itPrv.isBegin())
		itPrv.getCurrent()->setNext (nxt);

	//
	// To be sure
	//
	toDel->setPrev (PdfOperator::ListItem());
	getLastOperator(toDel)->setNext (PdfOperator::ListItem());
	
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
		assert (!it.valid());
		operators.push_back (newOper);
		return;
	}
	assert (!it.isEnd());
	assert (!cstreams.empty());

	// Set correct IndiRef, CPdf and cs to inserted operator
	assert (hasValidRef (cstreams.front()));
	assert (hasValidPdf (cstreams.front()));
	CPdf* pdf = cstreams.front()->getPdf();
	assert (pdf);
	IndiRef rf = cstreams.front()->getIndiRef ();
	opsSetPdfRefCs (newOper, *pdf, rf, *this, operandobserver);

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
//
//
void 
CContentStream::frontInsertOperator (boost::shared_ptr<PdfOperator> newoper, 
									 bool indicateChange)
{
	if (operators.empty ())
	{ // Insert into empty contentstream
		
		operators.push_back (newoper);
	
	}else
	{ // Insert into

		assert (!cstreams.empty());
		// Set correct IndiRef, CPdf and cs to inserted operator
		assert (hasValidRef (cstreams.front()));
		assert (hasValidPdf (cstreams.front()));
		CPdf* pdf = cstreams.front()->getPdf();
		assert (pdf);
		IndiRef rf = cstreams.front()->getIndiRef ();
		opsSetPdfRefCs (newoper, *pdf, rf, *this, operandobserver);

		shared_ptr<PdfOperator> secondoper = operators.front();
		operators.push_front (newoper);
		shared_ptr<PdfOperator> lastofnew = getLastOperator (newoper);
		secondoper->setPrev (lastofnew);
		lastofnew->setNext (secondoper);
	}

	// If indicateChange is true, pdf&rf&contenstream is set when reparsing
	if (indicateChange)
		_objectChanged ();

}

//
//
//
void
CContentStream::replaceOperator (OperatorIterator it, 
								 boost::shared_ptr<PdfOperator> newOper, 
								 bool indicateChange)
{


	kernelPrintDbg (debug::DBG_DBG, "");
	assert (!operators.empty());
	assert (!cstreams.empty());

	// Be sure that the operator won't get deallocated along the way
	boost::shared_ptr<PdfOperator> toReplace = it.getCurrent ();
	
	// Set correct IndiRef, CPdf and cs to inserted operator
	assert (hasValidRef (cstreams.front()));
	assert (hasValidPdf (cstreams.front()));
	CPdf* pdf = cstreams.front()->getPdf();
	assert (pdf);
	IndiRef rf = cstreams.front()->getIndiRef ();
	opsSetPdfRefCs (newOper, *pdf, rf, *this, operandobserver);

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

	// Prev and next of old operator
	OperatorIterator itPrv = it; itPrv.prev();
	OperatorIterator itNxt = PdfOperator::getIterator(getLastOperator (toReplace)).next();

	if (!itNxt.isEnd())
	{
		itNxt.getCurrent()->setPrev (itCurLast.getCurrent());
		itCurLast.getCurrent()->setNext (itNxt.getCurrent());
	}
	if (!itPrv.isBegin())
	{
		itPrv.getCurrent()->setNext (itCur.getCurrent());
		itCur.getCurrent()->setPrev (itPrv.getCurrent());
	}
	
	//
	// To be sure
	//
	toReplace->setPrev (PdfOperator::ListItem());
	getLastOperator(toReplace)->setNext (PdfOperator::ListItem());

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
CContentStream::registerCStreamObservers () const
{
	if (cstreamobserver)
	{
		// Register cstream observer
		for (CStreams::const_iterator it = cstreams.begin(); it != cstreams.end(); ++it)
			(*it)->registerObserver (cstreamobserver);
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
CContentStream::unregisterCStreamObservers () const
{
	if (cstreamobserver)
	{
		// Unregister cstream observer
		for (CStreams::const_iterator it = cstreams.begin(); it != cstreams.end(); ++it)
			(*it)->unregisterObserver (cstreamobserver);
	}else
	{
		assert (!"Observer is not initialized.");
		throw CObjInvalidOperation ();
	}
}


//==========================================================
// Operator helper functions
//==========================================================

//
//
//
bool containsNonStrokingOperator (boost::shared_ptr<PdfOperator> oper)
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
bool containsStrokingOperator (boost::shared_ptr<PdfOperator> oper)
{
	StrokingOperatorIterator it = PdfOperator::getIterator<StrokingOperatorIterator> (oper);
	if (it.isEnd())
		return false;
	else
		return true;
}


//==========================================================
} // namespace pdfobjects
//==========================================================
