/***************************************************************************
 *   Copyright (C) 2003-2004 by Christophe Devriese                        *
 *                         <Christophe.Devriese@student.kuleuven.ac.be>    *
 *   Copyright (C) 2003 by Helio Chissini de Castro                        *
 *                           <helio@conectiva.com.br>                      *
 *   Copyright (C) 2003 by Dirk Mueller <mueller@kde.org>                  *
 *   Copyright (C) 2003 by Scott Wheeler <wheeler@kde.org>                 *
 *   Copyright (C) 2003 by Ingo Klöcker <kloecker@kde.org>                 *
 *   Copyright (C) 2003 by Andy Goossens <andygoossens@telenet.be>         *
 *   Copyright (C) 2004 by Dominique Devriese <devriese@kde.org>           *
 *   Copyright (C) 2004 by Albert Astals Cid <tsdgeos@terra.es>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <xpdf/GfxState.h>

#include <qpixmap.h>
#include <qimage.h>

//#include <kdebug.h>

#include "splash/SplashBitmap.h"
#include "splash/SplashTypes.h"

#include "QOutputDevPixmap.h"

//------------------------------------------------------------------------
// QOutputDevPixmap
//------------------------------------------------------------------------

QOutputDevPixmap::QOutputDevPixmap(SplashColor paperColor) : QOutputDev(paperColor), m_image((const char * const*)0)
{
	raw_image = NULL;	//  -- this is necessary while Splash.cc::drawImage will not complete --
}

QOutputDevPixmap::~QOutputDevPixmap( )
{
	free(raw_image);	//  -- this is necessary while Splash.cc::drawImage will not complete --
}

void QOutputDevPixmap::endPage ( )
{
	SplashColorPtr dataPtr;
	int bh, bw;
	
	QOutputDev::endPage();
	bh = getBitmap()->getHeight();
	bw = getBitmap()->getWidth();
	dataPtr = getBitmap()->getDataPtr();
	//  -- this block is necessary while Splash.cc::drawImage will not complete --
	int pc = bh * bw;
	free(raw_image);
	if ((raw_image = (Guchar *)malloc( pc * 4 )) == NULL) {
		m_image.reset();
		return;
	}
	int h_bgr  = 0;
	int h_bgra = 0;
	for (int i = 0; i < pc ;++i) {
		raw_image[ h_bgra ] = dataPtr[ h_bgr ];
		raw_image[ h_bgra +1 ] = dataPtr[ h_bgr +1 ];
		raw_image[ h_bgra +2 ] = dataPtr[ h_bgr +2 ];
		raw_image[ h_bgra +3 ] = 0 ;	// alpha
		h_bgra += 4;
		h_bgr += 3;
	}
	dataPtr = raw_image;
	//  -- end the block --

	m_image = QImage((uchar*)dataPtr/*.rgb8*/, bw, bh, 32, 0, 0, QImage::IgnoreEndian);

}
