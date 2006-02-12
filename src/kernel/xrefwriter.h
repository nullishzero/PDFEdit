#ifndef _XREFWRITER_
#define _XREFWRITER_

/* 
 * $RCSfile$
 *
 * $Log$
 * Revision 1.2  2006/02/12 18:30:07  hockm0bm
 * first implementation (compileable) - not for real use
 *
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
#include "cxref.h"

using namespace pdfobjects;

/** Mode for XRefWriter.
 * This controls safety checking in insertObject method.
 * <ul>
 * <li>easy - no checking is performed
 * <li>paranoid - type safety and reference existance is checked.
 * </ul>
 */
enum XRefWriterMode {easy, paranoid};

/** CXref writer class.
 *
 * This wrapper of the CXref class enables to make changes to the to 
 * CXref.
 * <br>
 * TODO provide undo
 */
class XRefWriter:public CXref
{
        /** Mode for checking. */
        XRefWriterMode mode;

protected:
        /* Empty constructor.
         *
         * It's not available to prevent uninitialized instances.
         * Sets mode to paranoid.
         */
        XRefWriter():CXref(), mode(paranoid){}

        /** Checking for paranoid mode.
         * @param ref Reference of object.
         * @param obj Object to check.
         *
         * Checks if we are in paranoid mode. If not immediately returns.
         * Otherwise checks if given reference exists (uses knowsRef method).
         * Then replacing with given object is type safe.
         * <br>
         * TODO error handlign exceptions
         */
        bool paranoidCheck(Ref ref, Object * obj)
        {
                bool reinicialization=false;

                if(mode==paranoid)
                {
                        // reference known test
                        if(!knowsRef(ref))
                        {
                                // if reference is not known, it may be new 
                                // which is not changed after initialization
                                if(!(reinicialization=newStorage.contains(ref)))
                                {
                                        // TODO handle (not found at all)
                                        return false;
                                }
                        }
                        
                        // type safety test only if object has initialized 
                        // value already (so new and not changed are not test
                        if(!reinicialization)
                        {
                                // gets original value and uses typeSafe to 
                                // compare with given value type
                                Object original;
                                fetch(ref.num, ref.gen, &original);
                                bool safe=typeSafe(&original, obj);
                                original.free();
                                if(!safe)
                                {
                                        // TODO handle type error
                                        return false;
                                }
                        }
                }

                return true;
        }
public:
        /** Initialize constructor with stream.
         * @param stream Stream fith file data.
         *
         * Delegates to CXref with stream parameter.
         * Sets mode to paranoid.
         */
        XRefWriter(BaseStream * stream):CXref(stream), mode(paranoid){};

        /** Initialize constructor with cache.
         * @param stream Stream with file data.
         * @param c Cache instance.
         *
         * Delegates to CXref constructor with the stream and cache parameter.
         */
        /* FIXME uncoment when cache is available
        XRefWriter(BaseStream * stream, ObjectCache * c):CXRef(stream, c){};
        */
        
        /** Gets mode.
         * 
         * @return Actualy set mode.
         */
        XRefWriterMode getMode()const
        {
                return mode;
        }

        /** Sets mode.
         * @param mode Mode to set.
         *
         */
        void setMode(XRefWriterMode mode)
        {
                this->mode=mode;
        }
        
        /** Releases object.
         * @param num Number of object.
         * @param gen Generation number of object.
         *
         * Delegates to the CXref::releaseObject method.
         * <br>
         * TODO provide undo information
         */
        void releaseObject(int num, int gen)
        {
                Ref ref={num, gen};
                // checks if reference exists
                if(mode==paranoid && !knowsRef(ref))
                {
                        // TODO handle
                        return;
                }

                // delegates to CXref
                CXref::releaseObject(ref);
        }

        /** Inserts new object.
         * @param num Number of object.
         * @param gen Generation number of object.
         * @param obj Instance of object.
         *
         * Delegates to the CXref::insertObject method.
         * <br>
         * If mode is set to paranoid, checks the reference existence and after
         * type safety. If tests are ok, operation is permitted otherwise 
         * operation fails.
         * <br>
         * TODO provide undo information
         *
         */ 
        void changeObject(int num, int gen, Object * obj)
        {
                Ref ref={num, gen};
                
                // paranoid checking
                paranoidCheck(ref, obj);

                // everything ok
                CXref::changeObject(ref, obj);
        }

        /** Changes trailer entry.
         * @param name Name of the entry.
         * @param obj New value.
         *
         * Delegates to the CXref::changeTrailer method.
         * <br>
         * If mode is set to paranoid, checks the reference existence and after
         * type safety. If tests are ok, operation is permitted otherwise 
         * operation fails.
         * <br>
         * TODO provide undo information
         * 
         */
        Object * changeTrailer(char * name, Object * value)
        {
                // paranoid checking
                if(mode==paranoid)
                {
                        Object original;

                        // checks if value if indirect and if so, checks
                        // if reference is known
                        if((value->getType()==objRef) &&
                          (!CXref::knowsRef(value->getRef())))
                        {
                                // TODO handle
                                return 0;
                        }

                        // gets original value of value
                        Dict * dict=trailerDict.getDict();
                        dict->lookupNF(name, &original);
                        if(original.getType()!=objNull)
                        {
                                // original value exists, so checks type safety
                                bool safe=typeSafe(&original, value);
                                original.free();
                                if(!safe)
                                {
                                        // TODO handle type error
                                        return 0;
                                }
                        }
                }

                // everything ok
                return CXref::changeTrailer(name, value);
        }

        /** Saves xref with changes.
         * @param f File where to write (has to be open).
         *
         * Appends new xref with new trailer to the given file.
         */
        void saveXref(FILE * f)const
        {
                // TODO
        }
};

#endif
