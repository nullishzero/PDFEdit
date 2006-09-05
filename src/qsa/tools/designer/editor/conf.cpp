/**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "conf.h"
#include <qapplication.h>
#include <qfont.h>
#include <qcolor.h>
#include <qsettings.h>

QMap<QString, ConfigStyle> Config::defaultStyles()
{
    ConfigStyle s;
    QMap<QString, ConfigStyle> styles;
    int normalSize =  qApp->font().pointSize();
    QString normalFamily = qApp->font().family();
    QString commentFamily = QString::fromLatin1("times");
    int normalWeight = qApp->font().weight();

    s.font = QFont( normalFamily, normalSize, normalWeight );
    s.color = Qt::black;
    styles.insert( QString::fromLatin1("Standard"), s );

    s.font = QFont( commentFamily, normalSize, normalWeight, TRUE );
    s.color = Qt::red;
    styles.insert( QString::fromLatin1("Comment"), s );

    s.font = QFont( normalFamily, normalSize, normalWeight );
    s.color = Qt::blue;
    styles.insert( QString::fromLatin1("Number"), s );

    s.font = QFont( normalFamily, normalSize, normalWeight );
    s.color = Qt::darkGreen;
    styles.insert( QString::fromLatin1("String"), s );

    s.font = QFont( normalFamily, normalSize, normalWeight );
    s.color = Qt::darkMagenta;
    styles.insert( QString::fromLatin1("Type"), s );

    s.font = QFont( normalFamily, normalSize, normalWeight );
    s.color = Qt::darkYellow;
    styles.insert( QString::fromLatin1("Keyword"), s );

    s.font = QFont( normalFamily, normalSize, normalWeight );
    s.color = Qt::darkBlue;
    styles.insert( QString::fromLatin1("Preprocessor"), s );

    s.font = QFont( normalFamily, normalSize, normalWeight );
    s.color = Qt::darkRed;
    styles.insert( QString::fromLatin1("Label"), s );

    return styles;
}

QMap<QString, ConfigStyle> Config::readStyles( const QString &path )
{
    QMap<QString, ConfigStyle> styles;
    styles = defaultStyles();

    QString family;
    int size = 10;
    bool bold = FALSE, italic = FALSE, underline = FALSE;
    int red = 0, green = 0, blue = 0;

    QString elements[] = {
	QString::fromLatin1("Comment"),
	QString::fromLatin1("Number"),
	QString::fromLatin1("String"),
	QString::fromLatin1("Type"),
	QString::fromLatin1("Keyword"),
	QString::fromLatin1("Preprocessor"),
	QString::fromLatin1("Label"),
	QString::fromLatin1("Standard"),
	QString::null
    };

    for ( int i = 0; elements[ i ] != QString::null; ++i ) {
	QSettings settings;
	bool ok = TRUE;
	for (;;) {
	    family = settings.readEntry( path + elements[ i ] + "/family", QString::null, &ok );
	    if ( !ok )
		break;
	    size = settings.readNumEntry( path + elements[ i ] + "/size", 10, &ok );
	    if ( !ok )
		break;
	    bold = settings.readBoolEntry( path + elements[ i ] + "/bold", FALSE, &ok );
	    if ( !ok )
		break;
	    italic = settings.readBoolEntry( path + elements[ i ] + "/italic", FALSE, &ok );
	    if ( !ok )
		break;
	    underline = settings.readBoolEntry( path + elements[ i ] + "/underline", FALSE, &ok );
	    if ( !ok )
		break;
	    red = settings.readNumEntry( path + elements[ i ] + "/red", 0, &ok );
	    if ( !ok )
		break;
	    green = settings.readNumEntry( path + elements[ i ] + "/green", 0, &ok );
	    if ( !ok )
		break;
	    blue = settings.readNumEntry( path + elements[ i ] + "/blue", 0, &ok );
	    if ( !ok )
		break;
	    break;
	}
	if ( !ok )
	    continue;
	QFont f( family );
	f.setPointSize( size );
	f.setBold( bold );
	f.setItalic( italic );
	f.setUnderline( underline );
	QColor c( red, green, blue );
	ConfigStyle s;
	s.font = f;
	s.color = c;
	styles.remove( elements[ i ] );
	styles.insert( elements[ i ], s );
    }
    return styles;
}

void Config::saveStyles( const QMap<QString, ConfigStyle> &styles, const QString &path )
{
    QString elements[] = {
	QString::fromLatin1("Comment"),
	QString::fromLatin1("Number"),
	QString::fromLatin1("String"),
	QString::fromLatin1("Type"),
	QString::fromLatin1("Keyword"),
	QString::fromLatin1("Preprocessor"),
	QString::fromLatin1("Label"),
	QString::fromLatin1("Standard"),
	QString::null
    };

    QSettings settings;
    for ( int i = 0; elements[ i ] != QString::null; ++i ) {
	settings.writeEntry( path + QString::fromLatin1("/") + elements[ i ] + QString::fromLatin1("/family"), styles[ elements[ i ] ].font.family() );
	settings.writeEntry( path + QString::fromLatin1("/")  + elements[ i ] + QString::fromLatin1("/size"), styles[ elements[ i ] ].font.pointSize() );
	settings.writeEntry( path + QString::fromLatin1("/")  + elements[ i ] + QString::fromLatin1("/bold"), styles[ elements[ i ] ].font.bold() );
	settings.writeEntry( path + QString::fromLatin1("/")  + elements[ i ] + QString::fromLatin1("/italic"), styles[ elements[ i ] ].font.italic() );
	settings.writeEntry( path + QString::fromLatin1("/")  + elements[ i ] + QString::fromLatin1("/underline"), styles[ elements[ i ] ].font.underline() );
	settings.writeEntry( path + QString::fromLatin1("/")  + elements[ i ] + QString::fromLatin1("/red"), styles[ elements[ i ] ].color.red() );
	settings.writeEntry( path + QString::fromLatin1("/")  + elements[ i ] + QString::fromLatin1("/green"), styles[ elements[ i ] ].color.green() );
	settings.writeEntry( path + QString::fromLatin1("/")  + elements[ i ] + QString::fromLatin1("/blue"), styles[ elements[ i ] ].color.blue() );
    }
}

bool Config::completion( const QString &path )
{
    QSettings settings;
    bool ret = settings.readBoolEntry( path + QString::fromLatin1("/completion"), TRUE );
    return ret;
}

bool Config::wordWrap( const QString &path )
{
    QSettings settings;
    bool ret = settings.readBoolEntry( path + QString::fromLatin1("/wordWrap"), TRUE );
    return ret;
}

bool Config::parenMatching( const QString &path )
{
    QSettings settings;
    bool ret = settings.readBoolEntry( path + QString::fromLatin1("/parenMatching"), TRUE );
    return ret;
}

int Config::indentTabSize( const QString &path )
{
    QSettings settings;
    int ret = settings.readNumEntry( path + QString::fromLatin1("/indentTabSize"), 8 );
    return ret;
}

int Config::indentIndentSize( const QString &path )
{
    QSettings settings;
    int ret = settings.readNumEntry( path + QString::fromLatin1("/indentIndentSize"), 4 );
    return ret;
}

bool Config::indentKeepTabs( const QString &path )
{
    QSettings settings;
    bool ret = settings.readBoolEntry( path + QString::fromLatin1("/indentKeepTabs"), TRUE );
    return ret;
}

bool Config::indentAutoIndent( const QString &path )
{
    QSettings settings;
    bool ret = settings.readBoolEntry( path + QString::fromLatin1("/indentAutoIndent"), TRUE );
    return ret;
}

void Config::setCompletion( bool b, const QString &path )
{
    QSettings settings;
    settings.writeEntry( path + QString::fromLatin1("/completion"), b );
}

void Config::setWordWrap( bool b, const QString &path )
{
    QSettings settings;
    settings.writeEntry( path + QString::fromLatin1("/wordWrap"), b );
}

void Config::setParenMatching( bool b,const QString &path )
{
    QSettings settings;
    settings.writeEntry( path + QString::fromLatin1("/parenMatching"), b );
}

void Config::setIndentTabSize( int s, const QString &path )
{
    QSettings settings;
    settings.writeEntry( path + QString::fromLatin1("/indentTabSize"), s );
}

void Config::setIndentIndentSize( int s, const QString &path )
{
    QSettings settings;
    settings.writeEntry( path + QString::fromLatin1("/indentIndentSize"), s );
}

void Config::setIndentKeepTabs( bool b, const QString &path )
{
    QSettings settings;
    settings.writeEntry( path + QString::fromLatin1("/indentKeepTabs"), b );
}

void Config::setIndentAutoIndent( bool b, const QString &path )
{
    QSettings settings;
    settings.writeEntry( path + QString::fromLatin1("/indentAutoIndent"), b );
}
