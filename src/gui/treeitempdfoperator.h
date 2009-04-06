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
#ifndef __TREEITEMPDFOPERATOR_H__
#define __TREEITEMPDFOPERATOR_H__

#include "treeitemabstract.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <qstring.h>
namespace pdfobjects {
 class IProperty;
 class PdfOperator;
 class CContentStream;
}

namespace gui {

using namespace pdfobjects;

class TreeData;

/**
 class holding one PDF Operator in tree
 \brief Tree item containing PdfOperator
 */
class TreeItemPdfOperator : public TreeItemAbstract {
public:
 TreeItemPdfOperator(TreeData *_data,Q_ListView *parent,boost::shared_ptr<PdfOperator> pdfObj,boost::shared_ptr<CContentStream> cs,const QString name=QString::null,Q_ListViewItem *after=NULL);
 TreeItemPdfOperator(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<PdfOperator> pdfObj,boost::shared_ptr<CContentStream> cs,const QString name=QString::null,Q_ListViewItem *after=NULL);
 TreeItemPdfOperator(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<PdfOperator> pdfObj,const QString name=QString::null,Q_ListViewItem *after=NULL);
 virtual ~TreeItemPdfOperator();
 boost::shared_ptr<PdfOperator> getObject();
 //From TreeItemAbstract interface
 virtual void remove();
 virtual void reloadSelf();
 virtual bool validChild(const QString &name,Q_ListViewItem *oldChild);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,Q_ListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
 virtual QSCObject* getQSObject();
 virtual QString itemHint();
private:
 /** Operator inside this item */
 boost::shared_ptr<PdfOperator> obj;
 /** Vector with pdf operators (children) */
 std::vector<boost::shared_ptr<PdfOperator> > op;
 /** Vector with pdf operands (parameters) */
 std::vector<boost::shared_ptr<IProperty> > params;
 /** Reference to content stream that is holding this operator. */
 boost::shared_ptr<CContentStream> csRef;
};

} // namespace gui

#endif
