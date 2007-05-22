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

#include "qsmember.h"

/*!
  Returns a human readable description of this member's type.
 */

QString QSMember::typeName() const
{
    switch( typ ) {
    case Variable:
	return QString::fromLatin1("member variable");
    case NativeFunction:
	return QString::fromLatin1("C++ function");
    case NativeMemberFunction:
	return QString::fromLatin1("C++ member function");
    case ScriptFunction:
	return QString::fromLatin1("Function");
    case Object:
	return QString::fromLatin1("Object");
    case Identifier:
	return QString::fromLatin1("Identifier");
    case Custom:
	return QString::fromLatin1("Custom");
    default:
	return QString::fromLatin1("<undefined>");
    }
}
