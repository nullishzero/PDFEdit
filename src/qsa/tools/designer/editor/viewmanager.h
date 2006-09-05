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

#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <qwidget.h>
#include <qvaluelist.h>

class QChildEvent;
class MarkerWidget;
class QVBoxLayout;
class QDockArea;
class QTextParagraph;
class QLabel;
class QTimer;

class ViewManager : public QWidget
{
    Q_OBJECT

public:
    ViewManager( QWidget *parent, const char *name );

    void addView( QWidget *view );
    QWidget *currentView() const;
    void showMarkerWidget( bool );

    void setError( int line );
    void setStep( int line );
    void setStackFrame( int line );
    void clearStep();
    void clearStackFrame();
    void setBreakPoints( const QValueList<uint> &l );
    QValueList<uint> breakPoints() const;

    void emitMarkersChanged();
    MarkerWidget *marker_widget() const { return markerWidget; }

signals:
    void markersChanged();
    void expandFunction( QTextParagraph *p );
    void collapseFunction( QTextParagraph *p );
    void collapse( bool all /*else only functions*/ );
    void expand( bool all /*else only functions*/ );
    void editBreakPoints();
    void isBreakpointPossible( bool &possible, const QString &code, int line );

protected slots:
    void clearErrorMarker();
    void cursorPositionChanged( int row, int col );
    void showMessage( const QString &msg );
    void clearStatusBar();

protected:
    void childEvent( QChildEvent *e );
    void resizeEvent( QResizeEvent *e );

private:
    QWidget *curView;
    MarkerWidget *markerWidget;
    QVBoxLayout *layout;
    QDockArea *dockArea;
    QLabel *posLabel;
    QString extraText;
    QTimer *messageTimer;

};

#endif
