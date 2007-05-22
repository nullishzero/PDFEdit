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

#include "conf.h"
#include <qapplication.h>
#include <qfont.h>
#include <qcolor.h>
#include <qsettings.h>

const QLatin1String path("QSAScriptEditor");

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

    s.font = QFont( commentFamily, normalSize, normalWeight, true );
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

QMap<QString, ConfigStyle> Config::readStyles()
{
    QMap<QString, ConfigStyle> styles;
    styles = defaultStyles();

    QString family;
    int size = 10;
    bool bold = false, italic = false, underline = false;
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
	QSettings settings("Trolltech", "QSA");
        settings.beginGroup(path);
        settings.beginGroup(elements[i]);
        const bool ok = settings.contains("family")
                        && settings.contains("size")
                        && settings.contains("bold")
                        && settings.contains("italic")
                        && settings.contains("underline")
                        && settings.contains("red")
                        && settings.contains("green")
                        && settings.contains("blue")
                        ;
        if (ok) {
            family = settings.value("family").toString();
	    size = settings.value("size").toInt();
	    bold = settings.value("bold").toBool();
	    italic = settings.value("italic").toBool();
	    underline = settings.value("underline").toBool();
	    red = settings.value("red").toInt();
	    green = settings.value("green").toInt();
	    blue = settings.value("blue").toInt();
	}
        settings.endGroup();
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

void Config::saveStyles(const QMap<QString, ConfigStyle> &styles)
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

    QSettings settings("Trolltech", "QSA");
    settings.beginGroup(path);

    for ( int i = 0; elements[ i ] != QString::null; ++i ) {
        settings.beginGroup(elements[i]);
	settings.setValue("family", styles[ elements[ i ] ].font.family() );
	settings.setValue("size", styles[ elements[ i ] ].font.pointSize() );
	settings.setValue("bold", styles[ elements[ i ] ].font.bold() );
	settings.setValue("italic", styles[ elements[ i ] ].font.italic() );
	settings.setValue("underline", styles[ elements[ i ] ].font.underline() );
	settings.setValue("red", styles[ elements[ i ] ].color.red() );
	settings.setValue("green", styles[ elements[ i ] ].color.green() );
	settings.setValue("blue", styles[ elements[ i ] ].color.blue() );
        settings.endGroup();
    }

    settings.endGroup();
}

bool Config::completion()
{
    QSettings settings("Trolltech", "QSA");
    bool ret = settings.value( path + QString::fromLatin1("/completion"), true ).toBool();
    return ret;
}

bool Config::wordWrap()
{
    QSettings settings("Trolltech", "QSA");
    bool ret = settings.value( path + QString::fromLatin1("/wordWrap"), true ).toBool();
    return ret;
}

bool Config::parenMatching()
{
    QSettings settings("Trolltech", "QSA");
    bool ret = settings.value( path + QString::fromLatin1("/parenMatching"), true ).toBool();
    return ret;
}

int Config::indentTabSize()
{
    QSettings settings("Trolltech", "QSA");
    int ret = settings.value( path + QString::fromLatin1("/indentTabSize"), 8 ).toInt();
    return ret;
}

int Config::indentIndentSize()
{
    QSettings settings("Trolltech", "QSA");
    int ret = settings.value( path + QString::fromLatin1("/indentIndentSize"), 4 ).toInt();
    return ret;
}

bool Config::indentKeepTabs()
{
    QSettings settings("Trolltech", "QSA");
    bool ret = settings.value( path + QString::fromLatin1("/indentKeepTabs"), true ).toBool();
    return ret;
}

bool Config::indentAutoIndent()
{
    QSettings settings("Trolltech", "QSA");
    bool ret = settings.value( path + QString::fromLatin1("/indentAutoIndent"), true ).toBool();
    return ret;
}

void Config::setCompletion(bool b)
{
    QSettings settings("Trolltech", "QSA");
    settings.setValue( path + QString::fromLatin1("/completion"), b );
}

void Config::setWordWrap(bool b)
{
    QSettings settings("Trolltech", "QSA");
    settings.setValue( path + QString::fromLatin1("/wordWrap"), b );
}

void Config::setParenMatching(bool b)
{
    QSettings settings("Trolltech", "QSA");
    settings.setValue( path + QString::fromLatin1("/parenMatching"), b );
}

void Config::setIndentTabSize(int s)
{
    QSettings settings("Trolltech", "QSA");
    settings.setValue( path + QString::fromLatin1("/indentTabSize"), s );
}

void Config::setIndentIndentSize(int s)
{
    QSettings settings("Trolltech", "QSA");
    settings.setValue( path + QString::fromLatin1("/indentIndentSize"), s );
}

void Config::setIndentKeepTabs(bool b)
{
    QSettings settings("Trolltech", "QSA");
    settings.setValue( path + QString::fromLatin1("/indentKeepTabs"), b );
}

void Config::setIndentAutoIndent(bool b)
{
    QSettings settings("Trolltech", "QSA");
    settings.setValue( path + QString::fromLatin1("/indentAutoIndent"), b );
}
