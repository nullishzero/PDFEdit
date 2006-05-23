#ifndef __TREEITEMPDFOPERATOR_H__
#define __TREEITEMPDFOPERATOR_H__

#include <iproperty.h>
#include <qlistview.h>
#include "treeitem.h"
#include <pdfoperators.h>
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;

class TreeItemPdfOperator : public TreeItemAbstract {
public:
 TreeItemPdfOperator(TreeData *_data,QListView *parent,boost::shared_ptr<PdfOperator> pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItemPdfOperator(TreeData *_data,QListViewItem *parent,boost::shared_ptr<PdfOperator> pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 virtual ~TreeItemPdfOperator();
 boost::shared_ptr<PdfOperator> getObject();
 //From TreeItemAbstract interface
 virtual void remove();
 virtual void reloadSelf();
 virtual bool validChild(const QString &name,QListViewItem *oldChild);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
 virtual QSCObject* getQSObject();
private:
 /** Operator inside this item */
 boost::shared_ptr<PdfOperator> obj;
 /** Vector with pdf operators (children) */
 std::vector<boost::shared_ptr<PdfOperator> > op;
 /** Vector with pdf operands (parameters) */
 std::vector<boost::shared_ptr<IProperty> > params;
};

} // namespace gui

#endif
