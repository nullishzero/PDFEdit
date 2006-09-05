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

#ifndef MARKERWIDGET_H
#define MARKERWIDGET_H

#include <qwidget.h>
#include <qpixmap.h>

class ViewManager;
class QTextParagraph;

class MarkerWidget : public QWidget
{
    Q_OBJECT

public:
    MarkerWidget( ViewManager *parent, const char*name );

signals:
    void markersChanged();
    void expandFunction( QTextParagraph *p );
    void collapseFunction( QTextParagraph *p );
    void collapse( bool all /*else only functions*/ );
    void expand( bool all /*else only functions*/ );
    void editBreakPoints();
    void isBreakpointPossible( bool &possible, const QString &code, int line );
    void showMessage( const QString &msg );

public slots:
    void doRepaint() { repaint( FALSE ); }

protected:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void contextMenuEvent( QContextMenuEvent *e );

private:
    QPixmap buffer;
    ViewManager *viewManager;

};

#endif
