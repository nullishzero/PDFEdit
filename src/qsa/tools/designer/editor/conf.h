/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#ifndef CONF_H
#define CONF_H

#include <qfont.h>
#include <qcolor.h>
#include <qmap.h>

struct ConfigStyle
{
    QFont font;
    QColor color;

    Q_DUMMY_COMPARISON_OPERATOR( ConfigStyle )
};

struct Config
{
    QMap<QString, ConfigStyle> styles;
    bool hasCompletion, hasParenMatching, hasWordWrap;

    static QMap<QString, ConfigStyle> defaultStyles();
    static QMap<QString, ConfigStyle> readStyles( const QString &path );
    static void saveStyles( const QMap<QString, ConfigStyle> &styles, const QString &path );
    static bool completion( const QString &path );
    static bool wordWrap( const QString &path );
    static bool parenMatching( const QString &path );
    static int indentTabSize( const QString &path );
    static int indentIndentSize( const QString &path );
    static bool indentKeepTabs( const QString &path );
    static bool indentAutoIndent( const QString &path );

    static void setCompletion( bool b, const QString &path );
    static void setWordWrap( bool b, const QString &path );
    static void setParenMatching( bool b,const QString &path );
    static void setIndentTabSize( int s, const QString &path );
    static void setIndentIndentSize( int s, const QString &path );
    static void setIndentKeepTabs( bool b, const QString &path );
    static void setIndentAutoIndent( bool b, const QString &path );

};

#endif
