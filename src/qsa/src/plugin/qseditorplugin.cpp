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

#ifndef QSEEDITORPROVIDERH_
#define QSEEDITORPROVIDER_H

#include <qwidgetplugin.h>
#include <qseditor.h>
#include "qsa.xpm"

class QSEditorProvider : public QWidgetPlugin
{
    QStringList keys() const
    {
	return QStringList() << "QSEditor";
    }

    QWidget *create( const QString &className,
		     QWidget *parent = 0,
		     const char *name = 0 )
    {
	if ( className == "QSEditor" )
	    return new QSEditor( parent, name );
	return 0;
    }

    QString group( const QString &key ) const
    {
	if ( key == "QSEditor" )
	    return  "QSA";
	return QString::null;
    }

    QString includeFile( const QString &key ) const
    {
	if ( key == "QSEditor" )
	    return "qseditor.h";
	return QString::null;
    }

    QIconSet iconSet( const QString &key ) const
    {
	static QPixmap icon( (const char **)qsa_xpm );
	if ( key == "QSEditor" )
	    return QIconSet( icon );
	return QIconSet();
    }
};

Q_EXPORT_PLUGIN( QSEditorProvider )

#endif
