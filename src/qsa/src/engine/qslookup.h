/****************************************************************************
** $Id$
**
** Copyright (C) 2001-2006 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Script for Applications framework (QSA).
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding a valid Qt Script for Applications license may use
** this file in accordance with the Qt Script for Applications License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about QSA Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
*****************************************************************************/

#ifndef QSLOOKUP_H
#define QSLOOKUP_H

#include <qstring.h>

struct QSHashEntry {
    const char *s;
    int value;
    int attr;
    const QSHashEntry *next;
};

struct QSHashTable {
    int type;
    int size;
    const QSHashEntry *entries;
    int hashSize;
};

/**
 * @short Fast keyword lookup.
 */
class QSLookup {
public:
    static int find( const struct QSHashTable *table, const QString &s );
    static int find( const struct QSHashTable *table,
		     const QChar *c, unsigned int len);
    static unsigned int hash( const QChar *c, unsigned int len );
    static unsigned int hash( const QString &key );
    static unsigned int hash( const char *s );
};

#endif
