#include "xrefwriter.h"

using namespace pdfobjects;

bool XRefWriter::paranoidCheck(::Ref ref, ::Object * obj)
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
                        ::Object original;
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

void XRefWriter::releaseObject(int num, int gen)
{
        if(revision)
        {
                // we are in later revision, so no changes can't be
                // done
                // TODO handle
        }
                
        ::Ref ref={num, gen};
        // checks if reference exists
        if(mode==paranoid && !knowsRef(ref))
        {
                // TODO handle
                return;
        }

        // delegates to CXref
        CXref::releaseObject(ref);
}

void XRefWriter::changeObject(int num, int gen, ::Object * obj)
{
        if(revision)
        {
                // we are in later revision, so no changes can't be
                // done
                // TODO handle
        }

        ::Ref ref={num, gen};
        
        // paranoid checking
        paranoidCheck(ref, obj);

        // everything ok
        CXref::changeObject(ref, obj);
}

::Object * XRefWriter::changeTrailer(char * name, ::Object * value)
{
        if(revision)
        {
                // we are in later revision, so no changes can't be
                // done
                // TODO handle
        }
                
        // paranoid checking
        if(mode==paranoid)
        {
                ::Object original;

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

