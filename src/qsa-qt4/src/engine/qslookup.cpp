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
