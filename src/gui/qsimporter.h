/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __QSIMPORTER_H__
#define __QSIMPORTER_H__

#include <qobject.h>
#include <kernel/cobject.h>
class QSProject;
class QSInterpreter;
class QString;
namespace pdfobjects {
 class CPage;
 class PdfOperator;
 class CPdf;
}

namespace gui {

class QSCObject;
class QSPdf;
class TreeItemAbstract;
class BaseCore;

using namespace pdfobjects;

/**
 QSImporter -> import QObjects from application without re-evaluating project<br>
 Adding objects via addObject have disadvantage of clearing interpreter state
 (thus removing all functions loaded from initscript)
 Adding via addTransientObject disallow removing the object later.
 Can import any QObject into scripting layer under specified name and also can create
 QSCObjects from some common types (dict, page ..)
 \brief QObject importer into scripting
*/
class QSImporter : public QObject {
 Q_OBJECT
public:
 virtual ~QSImporter();
 void addQSObj(QObject *obj,const QString &name);
 //factory-style functions
 static QSCObject* createQSObject(boost::shared_ptr<IProperty> ip,BaseCore *_base);
 static QSCObject* createQSObject(TreeItemAbstract *item,BaseCore *_base);
 QSCObject* createQSObject(boost::shared_ptr<PdfOperator> op);
 QSCObject* createQSObject(boost::shared_ptr<IProperty> ip);
 QSCObject* createQSObject(boost::shared_ptr<CDict> dict);
 QSCObject* createQSObject(boost::shared_ptr<CPage> page);
 QSCObject* createQSObject(TreeItemAbstract *item);
 QSPdf* createQSObject(boost::shared_ptr<CPdf> pdf);
 QSImporter(QSProject *_qp,QObject *_context,BaseCore *_base);
public slots:
 QObject* getQSObj();
private:
 /** Current QObject to import */
 QObject *qobj;
 /** context in which objects will be imported */
 QObject *context;
 /** Interpreter from QProject qp. All objects will be imported in it. */
 QSInterpreter *qs;
 /** QSProject in which this importer is installed. */
 QSProject *qp;
 /** Scripting base for created objects */
 BaseCore *base;
};

} // namespace gui

#endif
