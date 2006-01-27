#ifndef _XREFWRITER_
#define _XREFWRITER_

/* 
 * $RCSfile$
 *
 * $Log$
 * Revision 1.1  2006/01/27 19:02:15  hockm0bm
 * writer class for the CXref class
 *
 *
 */

/**
 * @file xrefwriter.h
 *
 * Implementation of CXRef wrapper to enable making changes to the
 * CXref.
 */
#include "xpdf/Object.h"

using namespace pdfobjects;

/** CXref writer class.
 *
 * This wrapper of the CXref class enables to make changes to the to 
 * CXref.
 * <br>
 * TODO provide undo
 */
class XRefWriter:public CXRef
{
public:
        /** Releases object.
         * @param num Number of object.
         * @param gen Generation number of object.
         *
         * Delegates to the CXref::releaseObject method.
         * <br>
         * TODO provide undo information
         */
        void releaseObject(int num, int gen);

        /** Inserts new object.
         * @param num Number of object.
         * @param gen Generation number of object.
         * @param obj Instance of object.
         *
         * Delegates to the CXref::insertObject method/
         * <br>
         * TODO provide undo information
         *
         */ 
        void insertObject(int num, int gen, Object * obj);
}

#endif
