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

#ifndef QSAGLOBAL_H
#define QSAGLOBAL_H

#include <qglobal.h>

#include "qsconfig.h"

#define QSA_VERSION_STRING "1.1.5"
#define QSA_VERSION_MAJOR 1
#define QSA_VERSION_MINOR 1
#define QSA_VERSION_MICRO 5

// helper macros
#define QSA_MAKE_VERSION(maj, min, mic) (((maj) << 16) | ((min) << 8) | (mic))
#define QSA_VERSION \
    QSA_MAKE_VERSION(QSA_VERSION_MAJOR, QSA_VERSION_MINOR, QSA_VERSION_MICRO)

// evaluate to true if this version is (maj, min, mic) at minimum
#define QSA_IS_VERSION(maj, min, mic) \
    (QSA_VERSION >= QSA_MAKE_VERSION(maj, min, mic))

#if defined(Q_OS_WIN32) && !defined(QSA_NO_DLL)
#  if defined(QSA_DLL)
#    define QSA_EXPORT __declspec(dllexport)
#  else
#    define QSA_EXPORT __declspec(dllimport)
#  endif
#else
#  define QSA_EXPORT
#endif


#if defined (QSA_DLL)
#  define QSA_TEMPLATE_EXTERN
#elif defined (Q_TEMPLATEDLL)
#  define QSA_TEMPLATE_EXTERN Q_TEMPLATE_EXTERN
#else
#  define QSA_TEMPLATE_EXTERN
#endif

#endif
