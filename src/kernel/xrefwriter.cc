// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.5  2006/03/23 22:13:51  hockm0bm
 * printDbg added
 * exception handling
 * TODO removed
 * FIXME for revisions handling - throwing exeption
 *
 *
 */
#include "xrefwriter.h"

using namespace pdfobjects;
using namespace debug;

bool XRefWriter::paranoidCheck(::Ref ref, ::Object * obj)
{

	bool reinicialization=false;

	printDbg(DBG_DBG, "ref=["<<ref.num<<", "<<ref.gen<<"] type="<<obj->getType());

	if(mode==paranoid)
	{
		printDbg(DBG_DBG, "we are in paranoid mode");
		// reference known test
		if(!knowsRef(ref))
		{
			// if reference is not known, it may be new 
			// which is not changed after initialization
			if(!(reinicialization=newStorage.contains(ref)))
			{
				printDbg(DBG_WARN, "ref=["<<ref.num<<", "<<ref.gen<<"] is unknown");
				return false;
			}
		}
		
		// type safety test only if object has initialized 
		// value already (so new and not changed are not test
		if(!reinicialization)
		{
			// gets original value and uses typeSafe to 
			// compare with given value type
			::Object original;
			fetch(ref.num, ref.gen, &original);
			ObjType originalType=original.getType();
			bool safe=typeSafe(&original, obj);
			original.free();
			if(!safe)
			{
				printDbg(DBG_WARN, "ref=["<<ref.num<<", "<<ref.gen<<"] type="<<obj->getType()
						<<" is not compatible with original type="<<originalType);
				return false;
			}
		}
	}

	return true;
}

void XRefWriter::releaseObject(int num, int gen)
{
	printDbg(DBG_DBG, "num="<<num<<" gen="<<gen);
	
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		printDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(mode==readOnly)
	{
		// we are in read-only mode, no changes can be done
		printDbg(DBG_ERR, "no changes availabe, mode=readOnly");
		throw ReadOnlyDocumentException("Document is in readOnly mode.");
	}
		
	::Ref ref={num, gen};
	// checks if reference exists
	// FIXME really only in paranoid mode?
	if(mode==paranoid && !knowsRef(ref))
	{
		printDbg(DBG_ERR, "ref["<<ref.num<<", "<<ref.gen<<"] not found");
		throw IndirectObjectNotFoundException(ref.num, ref.gen);
	}

	// delegates to CXref
	CXref::releaseObject(ref);
}

void XRefWriter::changeObject(int num, int gen, ::Object * obj)
{
	printDbg(DBG_DBG, "ref=["<< num<<", "<<gen<<"]");
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		printDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(mode==readOnly)
	{
		// we are in read-only mode, no changes can be done
		printDbg(DBG_ERR, "no changes availabe, mode=readOnly");
		throw ReadOnlyDocumentException("Document is in readOnly mode.");
	}

	::Ref ref={num, gen};
	
	// paranoid checking
	if(!paranoidCheck(ref, obj))
	{
		printDbg(DBG_ERR, "paranoid check for ref=["<< num<<", "<<gen<<"] not successful");
		throw ElementBadTypeException("" /* FIXME "[" << num << ", " <<gen <<"]" */);
	}

	// everything ok
	CXref::changeObject(ref, obj);
}

::Object * XRefWriter::changeTrailer(char * name, ::Object * value)
{
	printDbg(DBG_DBG, "name="<<name);
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		printDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(mode==readOnly)
	{
		// we are in read-only mode, no changes can be done
		printDbg(DBG_ERR, "no changes availabe, mode=readOnly");
		throw ReadOnlyDocumentException("Document is in readOnly mode.");
	}
		
	// paranoid checking - can't use paranoidCheck because value may be also
	// direct - we are in trailer
	if(mode==paranoid)
	{
		::Object original;
		
		printDbg(DBG_DBG, "mode=paranoid type safety is checked");
		// gets original value of value
		Dict * dict=trailerDict.getDict();
		dict->lookupNF(name, &original);
		bool safe=typeSafe(&original, value);
		original.free();
		if(!safe)
		{
			printDbg(DBG_ERR, "type safety error");
			throw ElementBadTypeException(name);
		}
	}

	// everything ok
	return CXref::changeTrailer(name, value);
}

::Ref XRefWriter::reserveRef()
{
	printDbg(DBG_DBG, "");

	// checks read-only mode
	
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		printDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(mode==readOnly)
	{
		// we are in read-only mode, no changes can be done
		printDbg(DBG_ERR, "no changes availabe, mode=readOnly");
		throw ReadOnlyDocumentException("Document is in readOnly mode.");
	}

	// changes are availabe
	// delegates to CXref
	return CXref::reserveRef();
}


::Object * XRefWriter::createObject(::ObjType type, ::Ref * ref)
{
	printDbg(DBG_DBG, "type="<<type);

	// checks read-only mode
	
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		printDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(mode==readOnly)
	{
		// we are in read-only mode, no changes can be done
		printDbg(DBG_ERR, "no changes availabe, mode=readOnly");
		throw ReadOnlyDocumentException("Document is in readOnly mode.");
	}

	// changes are availabe
	// delegates to CXref
	return CXref::createObject(type, ref);
}
