/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "cindent.h"
#include "qregexp.h"

extern int indentForBottomLine( const QStringList& program, QChar typedIn );
extern void setTabSize( int s );
extern void setIndentSize( int s );

CIndent::CIndent()
    : Q3TextIndent(), tabSize( 8 ), indentSize( 4 ),
      autoIndent( true ), keepTabs( true ), lastDoc( 0 )
{
}

static int indentation( const QString &s )
{
    if ( s.simplifyWhiteSpace().length() == 0 )
	return 0;
    int i = 0;
    int ind = 0;
    while ( i < (int)s.length() ) {
	QChar c = s.at( i );
	if ( c == ' ' )
	    ind++;
	else if ( c == '\t' )
	    ind += 8;
	else
	    break;
	++i;
    }
    return ind;
}

void CIndent::tabify( QString &s )
{
    if ( !keepTabs )
	return;
    int i = 0;
    for ( ;; ) {
	for ( int j = i; j < (int)s.length(); ++j ) {
	    if ( s[ j ] != ' ' && s[ j ] != '\t' ) {
		if ( j > i ) {
		    QString t  = s.mid( i, j - i );
		    int spaces = 0;
		    for ( int k = 0; k < (int)t.length(); ++k )
			spaces += ( t[ k ] == ' ' ? 1 : tabSize );
		    s.remove( i, t.length() );
		    int tabs = spaces / tabSize;
		    spaces = spaces - ( tabSize * tabs );
		    QString tmp;
		    tmp.fill( ' ', spaces );
		    if ( spaces > 0 )
			s.insert( i, tmp );
		    tmp.fill( '\t', tabs );
		    if ( tabs > 0 )
			s.insert( i, tmp );
		}
		break;
	    }
	}
	i = s.find( '\n', i );
	if ( i == -1 )
	    break;
	++i;
    }
}

void CIndent::indentLine( Q3TextParagraph *p, int &oldIndent, int &newIndent )
{
    QString indentString;
    indentString.fill( ' ', newIndent );
    indentString.append( QString::fromLatin1("a") );
    tabify( indentString );
    indentString.remove( indentString.length() - 1, 1 );
    newIndent = indentString.length();
    oldIndent = 0;
    while ( p->length() > 0 && ( p->at( 0 )->c == ' ' || p->at( 0 )->c == '\t' ) ) {
	++oldIndent;
	p->remove( 0, 1 );
    }
    if (p->string()->length() == 0)
        p->insert(0, QLatin1String(" "));
    if (!indentString.isEmpty())
	p->insert(0, indentString);
}


void CIndent::indent( Q3TextDocument *doc, Q3TextParagraph *p, int *oldIndent, int *newIndent )
{
    lastDoc = doc;
    int oi = indentation( p->string()->toString() );
    QStringList code;
    Q3TextParagraph *parag = doc->firstParagraph();
    while ( parag ) {
	code << parag->string()->toString();
	if ( parag == p )
	    break;
	parag = parag->next();
    }

    int ind = indentForBottomLine( code, QChar::Null );
    indentLine( p, oi, ind );
    if ( oldIndent )
	*oldIndent = oi;
    if ( newIndent )
	*newIndent = ind;
}

void CIndent::reindent()
{
    if ( !lastDoc )
	return;
    // #### this is sloooooooow (O(n^2))
    Q3TextParagraph *parag = lastDoc->firstParagraph();
    while ( parag ) {
	indent( lastDoc, parag, 0, 0 );
	parag = parag->next();
    }
}

void CIndent::setTabSize( int ts )
{
    tabSize = ts;
    ::setTabSize( ts );
}

void CIndent::setIndentSize( int is )
{
    indentSize = is;
    ::setIndentSize( is );
}
