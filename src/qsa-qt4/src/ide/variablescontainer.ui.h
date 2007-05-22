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

extern QuickInterpreter *get_quick_interpreter(QSInterpreter*);

void QSVariablesContainer::updateScope()
{
#if QT_VERSION > 0x030100
    QSProject *pro = ::qt_cast<QSProject*>(QObject::sender());
#else
    const QObject *obj = QObject::sender();
    const QSProject *pro = obj ? qobject_cast<QSProject *>(obj) : 0;
#endif
    updateScope(pro->interpreter());
}

void QSVariablesContainer::updateScope(QSInterpreter *interpreter)
{
    // get scope
    QSEnv *env = get_quick_interpreter(interpreter)->env();

    // build tree
    // FIXME: list all the added objects too
    variablesListView->clear();
    int level = 0;
    QSObject scope = env->scopeObjectAt(level++);
    while ( scope.isValid() ) {
	QListViewItem *item = new QListViewItem(variablesListView, scope.objectType()->identifier());
	updateMembers(item, &scope);
	item->setOpen(true);
	scope = env->scopeObjectAt(level++);
    }
}

void QSVariablesContainer::updateMembers(QListViewItem *parent, QSObject *object)
{
    QSEnv *env = object->objectType()->env();
    QSMemberMap *mmap = object->objectType()->definedMembers();
    QSMemberMap::Iterator it = mmap->begin();
    for (; it != mmap->end(); ++it) {
	QSMember member = *it;
	if (member.type() == QSMember::Variable) {
	    QSObject value = object->fetchValue(member);
	    if (!(value.isA(env->typeClass()) ||
		  member.name() == "undefined" ||
		  member.name() == "NaN" ||
		  member.name() == "Infinity")) {
		QListViewItem *item  = new QListViewItem(parent,
							 member.name(),
							 value.toString(),
							 value.objectType()->name());
		if (value.objectType()->asClass()) {
		    updateMembers(item, &value);
		    item->setOpen(true);
		}
	    }
	}
    }
}
