#ifndef _OBJECTCOMPARATOR_H_
#define _OBJECTCOMPARATOR_H_

#include <map>
#include "xpdf/Object.h"

/*
 * $RCSfile$
 * 
 * $Log$
 * Revision 1.1  2006/01/29 17:09:07  hockm0bm
 * Object storage implementation
 * TODO - implement in template way
 *
 *
 */

/** Reference comparator class.
 *
 * Implements functional operator to compare two referencies defined as
 * Ref structures.
 * <br>
 * This can be used as comparator predicate in ObjectStorage where value is
 * Object type.
 */
class RefComparator
{
public:
        /** Comparator operator.
         * @param v1 First value.
         * @param v2 Second value.
         *
         * @returns true iff v1.num and v1.gen are less than v2.num and v2.gen.
         *
         */ 
        bool operator()(const Ref v1, const Ref v2)const
        {
                return (v1.num < v2.num) && (v1.gen < v2.gen);
        };
};

/**
 * @file objectstorage.h
 *
 * File which implements template object storage class. This is basicaly
 * mapping keys to objects and provide simple interface to manipulate 
 * with.
 */

/**
 */
class ObjectStorage
{
private:
        typedef std::map<Ref, Object *, RefComparator> Mapping;
        typedef Mapping::iterator Iterator;
        typedef Mapping::const_iterator ConstIterator;
        typedef Mapping::value_type Association;
        Mapping mapping;

public:
        ObjectStorage(){};

        /** Clears mapping.
         */
        void clear()
        {
                mapping.clear();
        };

        /** Add/change mapping.
         * @param key Key of the mapping.
         * @param value Value of the mapping (must be non null).
         *
         * If the key is not in the mapping yet, associates it with the value
         * and puts to the mapping.
         * Otherwise update value of the previous mapping.
         *
         * @returns Value of the previous mapping or 0 if the key was inserted
         * to the mapping.
         */
        Object * put(Ref key, Object * value)
        {
                // try to find at first
                Iterator iter=mapping.find(key);
                if(iter==mapping.end())
                {
                        mapping.insert(Association(key, value));
                        return 0;
                }
                
                Object * old=iter->second;
                
                // sets new value for association
                iter->second=value;
                
                // returns old value
                return old;
        };

        /** Finds value with the key.
         * @param key Refey of the value.
         *
         * Gets value associated with the key.
         *
         * @return value of the value (direct pointer) or 0 if no such key 
         * found.
         */
        Object * get(Ref key)const
        {
                ConstIterator iter=mapping.find(key);
                if(iter==mapping.end())
                        // association not found
                        return 0;

                return iter->second;
        };

        /** Removes association.
         * @param key Refey of the value.
         *
         * Gets value of the key and removes association from mapping.
         *
         * @return Object *alue of the key or 0 if not found (and not removed).
         */
        Object * remove(Ref key)
        {
                Iterator iter=mapping.find(key);
                if(iter==mapping.end())
                        // association not found
                        return 0;

                // gets old value and
                // removes association
                Object * old=get(key);
                mapping.erase(key);
                
                return old;
        };
};
#endif
