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
#ifndef __TREEITEMANNOTATION_H__
#define __TREEITEMANNOTATION_H__

#include "treeitemabstract.h"
#include <boost/shared_ptr.hpp>
#include <vector>
class QString;
namespace pdfobjects {
 class CPage;
 class CAnnotation;
}

namespace gui {

using namespace pdfobjects;

class TreeData;

/**
 Class holding one Annotation in tree.
 \brief Tree item containing CAnnotation
 */
class TreeItemAnnotation : public TreeItemAbstract {
public:
 TreeItemAnnotation(TreeData *_data,Q_ListView *parent,boost::shared_ptr<CAnnotation> pdfObj,boost::shared_ptr<CPage> _page,const QString name=QString::null,Q_ListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemAnnotation(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<CAnnotation> pdfObj,boost::shared_ptr<CPage> _page,const QString name=QString::null,Q_ListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemAnnotation();
 boost::shared_ptr<CAnnotation> getObject();
 //From TreeItemAbstract interface
 virtual bool haveChild();
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,Q_ListViewItem *after=NULL);
 virtual void reloadSelf();
 virtual ChildType getChildType(const QString &name);
 virtual QStringList getChildNames();
 virtual QSCObject* getQSObject();
 virtual void remove();
 virtual bool validChild(const QString &name,Q_ListViewItem *oldChild);
private:
 void init();
private:
 /** Annotation stored inside */
 boost::shared_ptr<CAnnotation> obj;
 /** CPage containing this annotation */
 boost::shared_ptr<CPage> page;
 /** Annotation title (name) */
 QString title;
};

} // namespace gui

#endif
