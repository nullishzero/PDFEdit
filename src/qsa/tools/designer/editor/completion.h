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

#ifndef COMPLETION_H
#define COMPLETION_H

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qmap.h>

class QTextDocument;
class Editor;
class QVBox;
class QListBox;
class ArgHintWidget;

struct CompletionEntry
{
    QString type;
    QString text;
    QString postfix;
    QString prefix;
    QString postfix2;

    bool operator==( const CompletionEntry &c ) const {
	return ( c.type == type &&
		 c.text == text &&
		 c.postfix == postfix &&
		 c.prefix == prefix &&
		 c.postfix2 == postfix2 );
    }
};

class EditorCompletion : public QObject
{
    Q_OBJECT

public:
    EditorCompletion( Editor *e );
    ~EditorCompletion();

    virtual void addCompletionEntry( const QString &s, QTextDocument *doc, bool strict );
    virtual QValueList<CompletionEntry> completionList( const QString &s, QTextDocument *doc ) const;
    virtual void updateCompletionMap( QTextDocument *doc );

    bool eventFilter( QObject *o, QEvent *e );
    virtual void setCurrentEdior( Editor *e );
    virtual bool doCompletion();
    virtual bool doObjectCompletion();
    virtual bool doObjectCompletion( const QString &object );
    virtual bool doArgumentHint( bool useIndex );

    virtual void addEditor( Editor *e );
    virtual QValueList<QStringList> functionParameters( const QString &func, QChar &, QString &prefix, QString &postfix );

    virtual void setContext( QObject *this_ );

    void setEnabled( bool b ) { enabled = b; }

protected:
    virtual bool continueComplete();
    virtual void showCompletion( const QValueList<CompletionEntry> &lst );
    virtual void completeCompletion();

protected:
    QVBox *completionPopup;
    QListBox *completionListBox;
    ArgHintWidget *functionLabel;
    int completionOffset;
    Editor *curEditor;
    QString searchString;
    QValueList<CompletionEntry> cList;
    QMap<QChar, QStringList> completionMap;
    bool enabled;
    QTextDocument *lastDoc;

};

#endif
