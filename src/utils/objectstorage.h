#ifndef _OBJECTCOMPARATOR_H_
#define _OBJECTCOMPARATOR_H_

#include <map>
#include "xpdf/Object.h"

/*
 * $RCSfile$
 * 
 * $Log$
 * Revision 1.4  2006/02/28 19:10:12  hockm0bm
 * Compile errors corrected
 *
 * Revision 1.3  2006/02/28 19:04:00  hockm0bm
 * Add iterators manipulation with ObjectStorage
 *
 * Revision 1.2  2006/02/13 19:05:35  hockm0bm
 * Template implementation of class.
 *
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
template<typename K, typename V, typename Comp> class ObjectStorage
{
private:
        typedef std::map<K, V, Comp> Mapping;
        typedef typename Mapping::value_type Association;
        Mapping mapping;

public:
        /** Iterator type. */
        typedef typename Mapping::iterator Iterator;

        /* Constant iterator type. */
        typedef typename Mapping::const_iterator ConstIterator;

        /** Empty constructor.
         *
         */
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
        V put(K key, V value)
        {
                // try to find at first
                Iterator iter=mapping.find(key);
                if(iter==mapping.end())
                {
                        mapping.insert(Association(key, value));
                        return 0;
                }
                
                V old=iter->second;
                
                // sets new value for association
                iter->second=value;
                
                // returns old value
                return old;
        };

        /** Finds value with the key.
         * @param key Key of the value.
         *
         * Gets value associated with the key.
         *
         * @return value of the value (direct pointer) or 0 if no such key 
         * found.
         */
        V get(K key)const
        {
                ConstIterator iter=mapping.find(key);
                if(iter==mapping.end())
                        // association not found
                        return 0;

                return iter->second;
        };

        /** Checks of given key is in the storage.
         * @param key Key object.
         *
         * @return true if given key is in the storage, false otherwise.
         */
        bool contains(K key)const
        {
                ConstIterator iter=mapping.find(key);

                // end iterator means not found
                return !(iter==mapping.end());
        }

        /** Removes association.
         * @param key Key of the value.
         *
         * Gets value of the key and removes association from mapping.
         *
         * @return Value of the key or 0 if not found (and not removed).
         */
        V remove(K key)
        {
                Iterator iter=mapping.find(key);
                if(iter==mapping.end())
                        // association not found
                        return 0;

                // gets old value and
                // removes association
                V old=get(key);
                mapping.erase(key);
                
                return old;
        };

        /** Number of elements.
         *
         * @return Elements count.
         */
        size_t size()const
        {
           return mapping.size();
        }

        /** Returns iterator to first element.
         *
         * @return Iterator instance.
         */
        Iterator begin()
        {
                return mapping.begin();
        }
        
        /** Returns iterator to end iterator.
         *
         * @return Iterator instance.
         */
        Iterator end()
        {
                return mapping.end();
        }
};
#endif
