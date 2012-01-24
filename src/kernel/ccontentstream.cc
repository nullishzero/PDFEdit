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
#include "kernel/ccontentstream.h"

//
#include "kernel/cpdf.h"
#include "kernel/pdfoperators.h"
#include "kernel/stateupdater.h"
#include "kernel/cobject.h"
#include "kernel/cstreamsxpdfreader.h"
#include "kernel/cinlineimage.h"
#include "kernel/contentschangetag.h"
#include "kernel/pdfoperatorsiter.h"

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
	opsSetPdfRefCs (boost::shared_ptr<PdfOperator> first, 
					boost::weak_ptr<CPdf> pdf, 
					IndiRef rf, 
					CContentStream& cs, 
					boost::shared_ptr<IPropertyObserver> observer)
	{
		utilsPrintDbg (DBG_DBG, "");
		CContentStream::OperatorIterator it = PdfOperator::getIterator (first);
		while (!it.isEnd())
		{
			// Set cs
			it.getCurrent()->setContentStream (cs);

			//
			// Set valid pdf and ref but lock the change 
			// 	-- cstream won't get notified by its children, not needed
			// 
			if (0 < it.getCurrent()->getParametersCount())
				it.getCurrent()->init_operands (observer, pdf, &rf);
			
			it = it.next ();
		}
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
		boost::shared_ptr< ::Object> o(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
		streamreader.getXpdfObject (*o);

		//
		// Get the inline image dictionary
		// 
		while (!streamreader.eof() && !o->isCmd("ID")) 
		{
			if (o->isName())
			{
				char* key = ::copyString (o->getName());
				streamreader.getXpdfObject (*o);
				if (streamreader.eof()) 
				{
					gfree (key);
					assert (!"Bad inline image.");
					throw CObjInvalidObject ();
				}
				dict.dictAdd (key, o.get());
			
			}
			
			streamreader.getXpdfObject (*o);
		}

		// Bad content stream
		if (streamreader.eof())
		{
			utilsPrintDbg (debug::DBG_ERR, "Content stream is damaged...");
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
	createOperandsFromStream (CStreamsXpdfReader<CContentStream::CStreams>& streamreader, 
					PdfOperator::Operands& operands,
					boost::shared_ptr< ::Object>& o)
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
				
				boost::shared_ptr<IProperty> pIp (createObjFromXpdfObj (*o));
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
	boost::shared_ptr<PdfOperator>
	createOperatorFromStream (CStreamsXpdfReader<CContentStream::CStreams>& streamreader, 
					PdfOperator::Operands& operands)
	{
		// Get operands
		boost::shared_ptr< ::Object> o(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
		if (!createOperandsFromStream (streamreader, operands, o))
			return boost::shared_ptr<PdfOperator> ();
		
		//
		// SPECIAL CASE for inline image (stream within a text stream)
		//
		if ( 0 == strncmp (o->getCmd(), "BI", 2))
		{
			utilsPrintDbg (debug::DBG_DBG, "");
			const StateUpdater::CheckTypes* chcktp = StateUpdater::findOp (o->getCmd());
			assert(chcktp);
			if (!checkAndFixOperator (*chcktp, operands))
			{
				//assert (!"Content stream bad operator type.");
				throw ElementBadTypeException ("Content stream operator has incorrect operand type.");
			}
			
			boost::shared_ptr<CInlineImage> inimg (getInlineImage (streamreader));
			return boost::shared_ptr<PdfOperator> (new InlineImageCompositePdfOperator (inimg, chcktp->name, chcktp->endTag));
		}

		// factory function for all other operators
		std::string name = o->getCmd();
		return createOperator(name, operands);
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
	boost::shared_ptr<PdfOperator>
	parseOp (CStreamsXpdfReader<CContentStream::CStreams>& streamreader, PdfOperator::Operands& operands)
	{
		// Create operator with its operands
		boost::shared_ptr<PdfOperator> result = createOperatorFromStream (streamreader, operands);
	
		if (result && isCompositeOp (result) && !isInlineImageOp (result))
		{
			string opname;
			result->getOperatorName (opname);
			string endtag = StateUpdater::getEndTag (opname);
			bool foundEndTag = false;
			
			// The same as in (re)parse
			boost::shared_ptr<PdfOperator> newop, previousLast = result;

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
	parse (CContentStream::Operators& operators, 
						CContentStream::CStreams& streams, 
						CContentStream& cs,
						boost::shared_ptr<IPropertyObserver> observer,
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
		boost::weak_ptr<CPdf> pdf = streams.front()->getPdf ();
		assert (pdf.lock());
		IndiRef rf = streams.front()->getIndiRef ();

		assert (!streams.empty());
		CStreamsXpdfReader<CContentStream::CStreams> streamreader (streams);
		streamreader.open ();
	
		PdfOperator::Operands operands;
		boost::shared_ptr<PdfOperator> topoperator (new UnknownCompositePdfOperator ("",""));	
		boost::shared_ptr<PdfOperator> newop, previousLast = topoperator;

		//
		// Parsing can throw, if so the stream is invalid
		// 	-- copy them to parsed streams any way
		//
		try 
		{
			bool our_change = false;
			while (newop=parseOp (streamreader, operands))
			{
				//
				// Is it our change
				//
				if (isPdfOp (*newop,ContentsChangeTag::CHANGE_TAG_NAME))
				{	
					PdfOperator::Operands ops;
					newop->getParameters (ops);
					if (!ops.empty())
					{
						try {
							if (ContentsChangeTag::CHANGE_TAG_ID == getNameFromIProperty(ops.front()))
								our_change = true;
						}catch (ElementBadTypeException&)
							{}
					}
				}

				topoperator->push_back (newop, previousLast);
				previousLast = getLastOperator (newop);

				if (parsedstreams)
				{ // FIRST PARSE
					//
					// We have found complete and correct content stream.
					//  BUT this can be tricky -- e.g. 
					//  Tf ...
					//  <<END>>
					//  Tj 
					// if we want to do something with all operators (xml
					// output) we have a problem
					//
					if (streamreader.eofOfActualStream())
					{
						if (our_change)
							break;
						boost::shared_ptr< ::Object> o(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
						streamreader.lookXpdfObject (*o);
						if (o->isName (ContentsChangeTag::CHANGE_TAG_ID))
							break;
					}
				}
			}

		}catch (CObjectException&)
		{
			kernelPrintDbg (debug::DBG_ERR, "Invalid content stream...");
			operands.clear ();
			//operators.clear ();
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
			opsSetPdfRefCs (operators.front(), pdf, rf, cs, observer);
	}


	//==========================================================
	// Gfx state updater functors
	//==========================================================

	/**
 	 * BBox updater.
	 */
	struct BBoxUpdater 
	{
		typedef PdfOperator::BBox BBox;

		// Init resources
		void operator() (boost::shared_ptr<GfxResources>) const {}

		// Loop through operators
		void operator() (boost::shared_ptr<PdfOperator> op, BBox rc, const GfxState&) const
		{
			// If not initialized, means an error occured (missing font etc..)
			if (!BBox::isInitialized (rc))
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
		if (_locked) 
			return;

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
		
	}catch (ReadOnlyDocumentException&)
	{
		kernelPrintDbg (debug::DBG_WARN, "Catching read-only exception...");
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
CContentStream::CContentStream (boost::shared_ptr<GfxState> state, 
		boost::shared_ptr<GfxResources> res) : gfxstate (state), gfxres (res) {
}

CContentStream::CContentStream (CStreams& strs, 
								boost::shared_ptr<GfxState> state, 
								boost::shared_ptr<GfxResources> res) 
	: gfxstate (state), gfxres (res)
{
	kernelPrintDbg (DBG_DBG, "");
	setStreams(strs);
}

void CContentStream::setStreams (CStreams& strs) {
	
	// cleanup
	if (cstreamobserver)
		unregisterCStreamObservers();

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
	
	// Parse it, move parsed streams from strs to cstreams
	parse (operators, strs, *this, operandobserver, &cstreams);
	
	// Save bounding boxes
	if (!operators.empty()) 
		StateUpdater::updatePdfOperators (PdfOperator::getIterator (operators.front()), gfxres, *gfxstate, BBoxUpdater());

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
		parse (operators, cstreams, *this, operandobserver);
	}
	
	// Save bounding boxes
	if (!operators.empty()) 
		StateUpdater::updatePdfOperators (PdfOperator::getIterator (operators.front()), gfxres, *gfxstate, BBoxUpdater());
}

//
//
//
void 
CContentStream::replaceText (const std::string& what, const std::string& with)
{
	bool dirty = false;
		if (operators.empty())
			return;

	operandobserver->lock();

	TextOperatorIterator tit = PdfOperator::getIterator<TextOperatorIterator> (operators.front());
	while (!tit.isEnd())
	{
		// uff
		boost::shared_ptr<TextSimpleOperator> _cur 
				= boost::dynamic_pointer_cast<TextSimpleOperator, PdfOperator> (tit.getCurrent());
		std::string tmp;
		_cur->getFontText (tmp);
		string replaced = boost::replace_all_copy (tmp, what, with);
		if (tmp != replaced)
		{
			dirty = true;
			boost::shared_ptr<TextSimpleOperator> _cur 
					= boost::dynamic_pointer_cast<TextSimpleOperator, PdfOperator> (tit.getCurrent());
			_cur->setFontText (replaced);
		}
		tit.next();
	}

	operandobserver->unlock();
	if (dirty)
		_objectChanged();
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

	try {
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

	}catch (PdfException&)
	{
		kernelPrintDbg (debug::DBG_WARN, "Restoring old value...");
		// Register observers again
		registerCStreamObservers ();
		throw;
	}

	//
	// Register observers again
	// 
	registerCStreamObservers ();
	
	// Update bboxes
	reparse (true);

	// Notify observers
	boost::shared_ptr<CContentStream> current (this, EmptyDeallocator<CContentStream> ());
	this->notifyObservers (current, boost::shared_ptr<const ObserverContext> (new BasicObserverContext (current)));
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
	assert (!cstreams.empty());

	// Check whether we can make the change
	cstreams.front()->canChange();
	
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
	{
		try {
			// notify observers and dispatch the change
			_objectChanged ();

		}catch (PdfException&)
		{
			assert (!"Should not happen.. Condition must be included in CPdf::canChange()...");
			throw;
		}
	}
}


//
//
//
void
CContentStream::insertOperator (OperatorIterator it, boost::shared_ptr<PdfOperator> newOper, bool indicateChange)
{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (!cstreams.empty());

	// Check whether we can make the change
	cstreams.front()->canChange();

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
	boost::weak_ptr<CPdf> pdf = cstreams.front()->getPdf();
	assert (pdf.lock());
	IndiRef rf = cstreams.front()->getIndiRef ();
	opsSetPdfRefCs (newOper, pdf, rf, *this, operandobserver);

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
	{
		try {
			// notify observers and dispatch the change
			_objectChanged ();

		}catch (PdfException&)
		{
			assert (!"Should not happen.. Condition must be included in CPdf::canChange()...");
			throw;
		}
	}
}

//
//
//
void 
CContentStream::frontInsertOperator (boost::shared_ptr<PdfOperator> newoper, 
		bool indicateChange)
{
		assert (!cstreams.empty());
		// Set correct IndiRef, CPdf and cs to inserted operator
		assert (hasValidRef (cstreams.front()));
		assert (hasValidPdf (cstreams.front()));

	// Check whether we can make the change
	cstreams.front()->canChange();
	IndiRef rf = cstreams.front()->getIndiRef ();
	boost::weak_ptr<CPdf> pdf = cstreams.front()->getPdf();
	assert (pdf.lock());
	// set accordingly	
	opsSetPdfRefCs (newoper, pdf, rf, *this, operandobserver);

	if (operators.empty ())
	{ // Insert into empty contentstream
		operators.push_back (newoper);
	}else
	{ // Insert into
		opsSetPdfRefCs (newoper, pdf, rf, *this, operandobserver);

		boost::shared_ptr<PdfOperator> secondoper = operators.front();
		operators.push_front (newoper);
		boost::shared_ptr<PdfOperator> lastofnew = getLastOperator (newoper);
		secondoper->setPrev (lastofnew);
		lastofnew->setNext (secondoper);
	}

	// If indicateChange is true, pdf&rf&contenstream is set when reparsing
	if (indicateChange)
	{
		try {
			// notify observers and dispatch the change
			_objectChanged ();

		}catch (PdfException&)
		{
			assert (!"Should not happen.. Condition must be included in CPdf::canChange()...");
			throw;
		}
	}
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

	// Check whether we can make the change
	cstreams.front()->canChange();

	// Be sure that the operator won't get deallocated along the way
	boost::shared_ptr<PdfOperator> toReplace = it.getCurrent ();
	
	// Set correct IndiRef, CPdf and cs to inserted operator
	assert (hasValidRef (cstreams.front()));
	assert (hasValidPdf (cstreams.front()));
	boost::weak_ptr<CPdf> pdf = cstreams.front()->getPdf();
	assert (pdf.lock());
	IndiRef rf = cstreams.front()->getIndiRef ();
	opsSetPdfRefCs (newOper, pdf, rf, *this, operandobserver);

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
	{
		try {
			// notify observers and dispatch the change
			_objectChanged ();

		}catch (PdfException&)
		{
			assert (!"Should not happen.. Condition must be included in CPdf::canChange()...");
			throw;
		}
	}
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
			REGISTER_SHAREDPTR_OBSERVER((*it), cstreamobserver);
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
			UNREGISTER_SHAREDPTR_OBSERVER((*it), cstreamobserver);
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
