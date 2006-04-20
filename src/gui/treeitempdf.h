#ifndef __TREEITEMPDF_H__
#define __TREEITEMPDF_H__

#include <qlistview.h>
#include "treeitemabstract.h"
class QString;
namespace pdfobjects {
class CPdf;
}

namespace gui {

class TreeData;

using namespace pdfobjects;

class TreeItemPdf : public TreeItemAbstract {
public:
 TreeItemPdf(TreeData *_data,CPdf *_pdf,QListView *parent,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItemPdf(TreeData *_data,CPdf *_pdf,QListViewItem *parent,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItemPdf(TreeData *_data,TreeItemPdf *parent,const QString name,QListViewItem *after=NULL);
 ~TreeItemPdf();
 CPdf* getObject();
 //From TreeItemAbstract interface
 virtual TreeItemAbstract* createChild(const QString &name,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual void reloadSelf();
private:
 void init(CPdf *pdf,const QString &name);
 void initSpec(CPdf *pdf,const QString &name);
 /** CPdf stored in this TreeItem */
 CPdf *obj;
 /** Node type (Null, Pages,Outlines) */
 QString nType;
 /** Data from treewindow containing reference to it and other needed data */
 TreeData *data;
};

} // namespace gui

#endif
