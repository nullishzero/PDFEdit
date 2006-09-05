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

#define HAVE_STRING_H 1

#include <qglobal.h>

#if defined(Q_OS_WIN32)
# define HAVE_FLOAT_H 1
# define HAVE_FUNC__FINITE 1
# define HAVE_SYS_TIMEB_H 1
#else
# define HAVE_SYS_TIME_H 1
# define HAVE_SYS_PARAM_H 1
# define HAVE_FUNC_ISNAN 1
# define HAVE_FUNC_ISINF 1
#endif

#if defined(Q_OS_MAC)
#define isnan __isnan
# undef HAVE_FUNC_ISINF
# define HAVE_FUNC_FINITE 1  // isinf not available on Mac OS X
#endif

#if defined(Q_OS_IRIX) || defined(Q_OS_SOLARIS)
# undef HAVE_FUNC_ISINF
# define HAVE_FUNC_FINITE 1
# define HAVE_IEEEFP_H 1
#endif
