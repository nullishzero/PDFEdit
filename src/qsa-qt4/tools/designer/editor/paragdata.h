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

#ifndef PARAGDATA_H
#define PARAGDATA_H

#include "parenmatcher.h"
#include "q3richtext_p.h"

struct ParagData : public Q3TextParagraphData
{
public:
    enum MarkerType { NoMarker, Error, Breakpoint };
    enum LineState { FunctionStart, InFunction, FunctionEnd, Invalid };

    ParagData() : lastLengthForCompletion( -1 ), marker( NoMarker ),
	lineState( Invalid ), functionOpen( true ), step( false ), stackFrame( false ) {}
    ~ParagData() {}
    void join( Q3TextParagraphData *data ) {
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
