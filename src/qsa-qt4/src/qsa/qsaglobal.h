/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech AS. All rights reserved.
**
** This file is part of the QSA of the Qt Toolkit.
**
** For QSA Commercial License Holders (non-open source):
** 
** Licensees holding a valid Qt Script for Applications (QSA) License Agreement
** may use this file in accordance with the rights, responsibilities and
** obligations contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of the Licensing Agreement are
** not clear to you.
** 
** Further information about QSA licensing is available at:
** http://www.trolltech.com/products/qsa/licensing.html or by contacting
** info@trolltech.com.
** 
** 
** For Open Source Edition:  
** 
** This file may be used under the terms of the GNU General Public License
** version 2 as published by the Free Software Foundation and appearing in the
** file LICENSE.GPL included in the packaging of this file.  Please review the
** following information to ensure GNU General Public Licensing requirements
** will be met:  http://www.trolltech.com/products/qt/opensource.html 
** 
** If you are unsure which license is appropriate for your use, please review
** the following information:
** http://www.trolltech.com/products/qsa/licensing.html or contact the 
** sales department at sales@trolltech.com.

**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSAGLOBAL_H
#define QSAGLOBAL_H

#include <qglobal.h>

#include "qsconfig.h"

#define QSA_VERSION_STRING "1.2.2"
#define QSA_VERSION_MAJOR 1
#define QSA_VERSION_MINOR 2
#define QSA_VERSION_MICRO 2

#if QT_VERSION < 0x040001
#error This version of QSA requires Qt 4.0.1 or later
#endif

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

#endif
