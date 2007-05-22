/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech AS. All rights reserved.
**
** This file is part of the QSA of the Qt Toolkit.
**
** For QSA Commercial License Holders (non-open source):
** 
** Licensees holding a valid Qt Script for Applications (QSA) License Agreement
** may use this file in accordance with the rights, responsibilities and
** obligations contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of the Licensing Agreement are
** not clear to you.
** 
** Further information about QSA licensing is available at:
** http://www.trolltech.com/products/qsa/licensing.html or by contacting
** info@trolltech.com.
** 
** 
** For Open Source Edition:  
** 
** This file may be used under the terms of the GNU General Public License
** version 2 as published by the Free Software Foundation and appearing in the
** file LICENSE.GPL included in the packaging of this file.  Please review the
** following information to ensure GNU General Public Licensing requirements
** will be met:  http://www.trolltech.com/products/qt/opensource.html 
** 
** If you are unsure which license is appropriate for your use, please review
** the following information:
** http://www.trolltech.com/products/qsa/licensing.html or contact the 
** sales department at sales@trolltech.com.

**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSTYPES_H
#define QSTYPES_H

#include "qsvalues.h"

class QSList;
class QSListIterator;

/**
   * @internal
   */
class ListNode {
    friend class QSList;
    friend class QSListIterator;
    ListNode(QSObject obj, ListNode *p, ListNode *n)
	: member(obj), prev(p), next(n) {};
    QSObject member;
    ListNode *prev, *next;
};

/**
   * @short Iterator for @ref QSList objects.
   */
class QSListIterator {
    friend class QSList;
    QSListIterator();
    QSListIterator(ListNode *n) : node(n) { }
public:
    /**
     * Construct an iterator that points to the first element of the list.
     * @param l The list the iterator will operate on.
     */
    QSListIterator(const QSList &list);
    /**
     * Assignment constructor.
     */
    QSListIterator& operator=(const QSListIterator &iterator)
    { node=iterator.node; return *this; }
    /**
     * Copy constructor.
     */
    QSListIterator(const QSListIterator &i) : node(i.node) { }
    /**
     * Dereference the iterator.
     * @return A pointer to the element the iterator operates on.
     */
    QSObject* operator->() const { return &node->member; }
    QSObject operator*() const { return node->member; }
    /**
     * Postfix increment operator.
     * @return The element after the increment.
     */
    QSObject operator++() { node = node->next; return node->member; }
    /**
     * Prefix increment operator.
     */
    QSObject operator++(int) { const ListNode *n = node; ++*this; return n->member; }
    /**
     * Postfix decrement operator.
     */
    QSObject operator--() { node = node->prev; return node->member; }
    /**
     * Prefix decrement operator.
     */
    QSObject operator--(int) { const ListNode *n = node; --*this; return n->member; }
    /**
     * Compare the iterator with another one.
     * @return True if the two iterators operate on the same list element.
     * False otherwise.
     */
    bool operator==(const QSListIterator &it) const { return node==it.node; }
    /**
     * Check for inequality with another iterator.
     * @return True if the two iterators operate on different list elements.
     */
    bool operator!=(const QSListIterator &it) const { return node!=it.node; }
private:
    ListNode *node;
};

/**
   * @short Native list type.
   *
   * List is a native ECMAScript type. List values are only used for
   * intermediate results of expression evaluation and cannot be stored
   * as properties of objects.
   *
   * The list is explicitly shared. Note that while copy() returns a deep
   * copy of the list the referenced objects are still shared.
   */
class QSList {
    friend class QSListIterator;
public:
    /**
     * Constructor.
     */
    QSList();

    QSList(const QSObject &first);

    /**
     * Destructor.
     */
    ~QSList();
    /**
     * Append an object to the end of the list.
     *
     * @param obj Pointer to object.
     */
    void append(const QSObject& obj);
    /**
     * Insert an object at the beginning of the list.
     *
     * @param obj Pointer to object.
     */
    void prepend(const QSObject& obj);
    /**
     * Remove the element at the beginning of the list.
     */
    void removeFirst();
    /**
     * Remove the element at the end of the list.
     */
    void removeLast();
    /*
     * Remove obj from list.
     */
    void remove(const QSObject &obj);
    /**
     * Remove all elements from the list.
     */
    void clear();
    /**
     * Returns a deep copy of the list. Ownership is passed to the user
     * who is responsible for deleting the list then.
     */
    QSList *copy() const;
    /**
     * @return A @ref QSListIterator pointing to the first element.
     */
    QSListIterator begin() const { return QSListIterator(hook->next); }
    /**
     * @return A @ref QSListIterator pointing to the last element.
     */
    QSListIterator end() const { return QSListIterator(hook); }
    /**
     * @return true if the list is empty. false otherwise.
     */
    bool isEmpty() const { return hook->prev == hook; }
    /**
     * @return the current size of the list.
     */
    int size() const;
    /**
     * Retrieve an element at an indexed position. If you want to iterate
     * trough the whole list using @ref QSListIterator will be faster.
     *
     * @param i List index.
     * @return Return the element at position i. @ref QSUndefined if the
     * index is out of range.
     */
    QSObject at(int i) const;
    /**
     * Equivalent to @ref at.
     */
    QSObject operator[](int i) const { return at(i); }

#ifdef QSDEBUG_MEM
    /**
     * @internal
     */
    static int count;
#endif
private:
    // not implemented
    QSList(const QSList &);
    QSList &operator=(const QSList &);

    void init();
    void erase(ListNode *n);
    ListNode *hook;
};

#endif
