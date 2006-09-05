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

#include "qslookup.h"

#include <string.h>

int QSLookup::find( const struct QSHashTable *table,
		    const QChar *c, unsigned int len )
{
    char *ascii = new char[len+1];
    unsigned int i;
    for( i = 0; i < len; i++, c++ ) {
	if ( !c->row() )
	    ascii[i] = c->cell();
	else
	    break;
    }
    ascii[i] = '\0';

    int h = hash( ascii ) % table->hashSize;
    const QSHashEntry *e = &table->entries[h];

    // empty bucket ?
    if ( !e->s ) {
	delete [] ascii;
	return -1;
    }

    do {
	// compare strings
	if ( strcmp(ascii, e->s ) == 0) {
	    delete [] ascii;
	    return e->value;
	}
	// try next bucket
	e = e->next;
    } while ( e );

    delete [] ascii;
    return -1;
}

int QSLookup::find( const struct QSHashTable *table, const QString &s )
{
    return find( table, s.unicode(), s.length() );
}

unsigned int QSLookup::hash( const QChar *c, unsigned int len )
{
    unsigned int val = 0;
    // ignoring higher byte
    for ( unsigned int i = 0; i < len; i++, c++ )
	val += c->cell();

    return val;
}

unsigned int QSLookup::hash( const QString &key )
{
    return hash( key.unicode(), key.length() );
}

unsigned int QSLookup::hash( const char *s )
{
    unsigned int val = 0;
    while ( *s )
	val += *s++;

    return val;
}
