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
#ifndef _OBJECTCOMPARATOR_H_
#define _OBJECTCOMPARATOR_H_

#include <map>


/**
 * @file objectstorage.h
 *
 * File which implements template object storage class. This is basicaly
 * mapping keys to objects and provide simple interface to manipulate
 * with it. It wrapps STL map class functionality.
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

        /** Constant iterator type. */
        typedef typename Mapping::const_iterator ConstIterator;

        /** Empty constructor.
         *
         */
        ObjectStorage(){}

        /** Clears mapping.
         *
         * Doesn't deallocate values!
         */
        void clear()
        {
                mapping.clear();
        }

        /** Add/change mapping.
         * @param key Key of the mapping.
         * @param value Value of the mapping (must be non null).
         *
         * If the key is not in the mapping yet, associates it with the value
         * and puts to the mapping.
         * Otherwise update value of the previous mapping.
         * <br>
         * This method doesn't invalidate existing (if any) iterator pointing
         * to the association.
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
        }

        /** Finds value with the key.
         * @param key Key of the value.
         *
         * Gets value associated with the key.
         *
         * @return value of the value (direct pointer) or 0 if no such key
         * found.
         */
        V get(K & key)const
        {
                ConstIterator iter=mapping.find(key);
                if(iter==mapping.end())
                        // association not found
                        return 0;

                return iter->second;
        }

        /** Checks of given key is in the storage.
         * @param key Key object.
         *
         * @return true if given key is in the storage, false otherwise.
         */
        bool contains(K & key)const
        {
                ConstIterator iter=mapping.find(key);

                // end iterator means not found
                return !(iter==mapping.end());
        }

        /** Removes association.
         * @param key Key of the value.
         *
         * Gets value of the key and removes association from mapping.
         * <br>
         * This method doesn't invalidate interators, except one which points
         * to the removed element.
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
        }

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

        /** Returns const iterator to first element.
         *
         * @return ConstIterator instance.
         */
	ConstIterator begin()const
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

        /** Returns const iterator to end iterator.
         *
         * @return ConstIterator instance.
         */
	ConstIterator end()const
	{
		return mapping.end();
	}
};
#endif
