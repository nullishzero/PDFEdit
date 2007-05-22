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

#include "qsworkbench.h"
#include "idewindow.h"

/*!
  \class QSWorkbench qsscripter.h

  \brief The QSWorkbench class provides an interface to QSA Workbench, the
  QSA simple scripting environment.

*/


class QSWorkbenchPrivate
{
public:
    IdeWindow *ideWindow;
};


/*!
  Creates the simple scripting environment for the project \a project,
  with parent \a parent and name \a name.
*/
QSWorkbench::QSWorkbench( QSProject *project, QWidget *parent, const char *name )
    : QObject( parent ), d( new QSWorkbenchPrivate() )
{
/*    extern int qInitResources_ide();
    qInitResources_ide();*/
    d->ideWindow = new IdeWindow( parent, name );
    d->ideWindow->setProject( project );
}

/*!
  \internal
 */
QSWorkbench::~QSWorkbench()
{
    delete d->ideWindow;
    delete d;
}

/*!
  Returns the QSA Workbench main widget.
 */

QWidget *QSWorkbench::widget()
{
    return (QWidget *)d->ideWindow;
}

/*!
  Returns the project edited in QSA Workbench.
*/

QSProject *QSWorkbench::project()
{
    return d->ideWindow->project;
}

/*!
  Opens QSA Workbench, the QSA simple scripting environment.
*/
void QSWorkbench::open()
{
    d->ideWindow->show();
}

/*!
  Hides QSA Workbench. Hiding the Scripting environement does
  not trigger any change in the project.
 */
void QSWorkbench::close()
{
    d->ideWindow->close();
}

/*!
  Makes \a script the current script in the editor.
*/
void QSWorkbench::showScript( QSScript *script )
{
    d->ideWindow->showPage( script );
}

/*!
  Makes \a script the current script in the editor and highlights the
  declaration of \a function, if found.
*/
void QSWorkbench::showFunction( QSScript *script, const QString &function )
{
    d->ideWindow->showFunction( script, function );
}
