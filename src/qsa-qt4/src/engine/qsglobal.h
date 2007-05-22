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

#ifndef QSGLOBAL_H
#define QSGLOBAL_H

// keep in sync with attrStr (qscheck.cpp)
enum QSAttribute {
    AttributeNone 	 = 0,
    AttributeAbstract 	 = 1 << 1,
    AttributeFinal	 = 1 << 2,
    AttributePrivate 	 = 1 << 3,
    AttributePublic	 = 1 << 4,
    AttributeStatic 	 = 1 << 5,
    AttributeTrue 	 = 1 << 6,
    AttributeFalse 	 = 1 << 7,
    AttributeConstructor = 1 << 8,
    AttributeExecutable  = 1 << 9,
    AttributeNonReadable = 1 << 10,
    AttributeNonWritable = 1 << 11,
    AttributeEnumerable  = 1 << 12
};

enum QSCompareResult {
    CompareLess = -1,
    CompareEqual = 0,
    CompareGreater = 1,
    CompareUndefined = 2
};

enum QSEqualsResult {
    EqualsNotEqual = 0,
    EqualsIsEqual = 1,
    EqualsUndefined = 2
};

// max call stack size. -1 means no limit
#define QS_MAX_STACK 500

// do some extra memory managment checking (0: off, 1: on)
#define QS_DEBUG_MEM 0

// used until reference counting is 100% safe
// #define QS_LEAK

#endif
