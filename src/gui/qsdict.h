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
#ifndef __QSDICT_H__
#define __QSDICT_H__

#include "qsiproperty.h"
#include <qstringlist.h>
#include <kernel/cobject.h>

namespace gui {

using namespace pdfobjects;

/*=
 This type represents a dictionary in PDF document.
 It hold keys (String) and values. Each key is corresponding to one value.
 Keys are strings, values can be of any type, either simple types (int, bool, float, string)
 or complex types (Dict, Array)
*/
/** \brief QObject wrapper around CDict */
class QSDict : public QSIProperty {
 Q_OBJECT
public:
 QSDict(QSDict &source);
 QSDict(boost::shared_ptr<CDict> _dict,BaseCore *_base);
 virtual ~QSDict();
 boost::shared_ptr<CDict> get();
public slots:
 /*- Add property with given name to this dictionary */
 void add(const QString &name,QSIProperty *ip);
 void add(const QString &name,QObject *ip);
 /*- Add string property with given name to this dictionary */
 void add(const QString &name,const QString &ip);
 /*- Add integer property with given name to this dictionary */
 void add(const QString &name,int ip);
 /*-
  Get Dict/Array property recursively
  Will take the name as slash-separated list of childs to traverse to get to target property,
  going through Dicts and Arrays.
  Any references on the way are automatically dereferenced
 */
 QSCObject* child(const QString &name);
 /*- Return number of properties held in this dictionary */
 int count();
 /*- Delete property with given name from this dictionary */
 void delProperty(const QString &name);
 /*- Check for existence of property with given name in this dictionary. If it exists, returns true */
 bool exist(const QString &name);
 /*- Return string representation of this dictionary */
 QString getText();
 /*- Get property with given name from this dictionary */
 QSCObject* property(const QString &name);
 /*-
  Get property with given name from this dictionary.
  If the property does not exist, add it to the dictionary with given defValue (as Int)
  and return it
 */
 QSCObject* propertyDef(const QString &name,int defValue);
 /*-
  Get property with given name from this dictionary.
  If the property does not exist, add it to the dictionary with given defValue (as String)
  and return it
 */
 QSCObject* propertyDef(const QString &name,QString defValue);
 /*- Return array containing names of all properties */
 QStringList propertyNames();
};

} // namespace gui

#endif
