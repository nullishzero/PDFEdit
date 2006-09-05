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

#ifndef PARAGDATA_H
#define PARAGDATA_H

#include "parenmatcher.h"
#include <private/qrichtext_p.h>

struct ParagData : public QTextParagraphData
{
public:
    enum MarkerType { NoMarker, Error, Breakpoint };
    enum LineState { FunctionStart, InFunction, FunctionEnd, Invalid };

    ParagData() : lastLengthForCompletion( -1 ), marker( NoMarker ),
	lineState( Invalid ), functionOpen( TRUE ), step( FALSE ), stackFrame( FALSE ) {}
    ~ParagData() {}
    void join( QTextParagraphData *data ) {
	ParagData *d = (ParagData*)data;
	if ( marker == NoMarker )
	    marker = d->marker;
	lineState = d->lineState;
    }
    ParenList parenList;
    int lastLengthForCompletion;
    MarkerType marker;
    LineState lineState;
    bool functionOpen;
    bool step;
    bool stackFrame;

};

#endif
