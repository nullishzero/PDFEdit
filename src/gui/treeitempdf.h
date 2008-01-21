/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __TREEITEMPDF_H__
#define __TREEITEMPDF_H__

#include "treeitemabstract.h"
#include "treeitemoutline.h"

class QString;

namespace pdfobjects {
class CPdf;
class IProperty;
}

namespace gui {

class TreeData;
class QSCObject;
class TreeItemObserver;

using namespace pdfobjects;

/**
 Class holding CPDF object (document) in tree<br>
 This class can actually be in the tree multiple times, in root of the tree
 it represents the document, but the document have some special children,
 which are of this class too (for simplicity they "map" to PDF too)<br>

 These are:<br>

  List of outlines<br>
  List of pages<br>
 \brief Tree item containing PDF document or special parts of it
*/
class TreeItemPdf : public TreeItemAbstract {
public:
 TreeItemPdf(TreeData *_data,CPdf *_pdf,Q_ListView *parent,const QString &name=QString::null,Q_ListViewItem *after=NULL);
 TreeItemPdf(TreeData *_data,CPdf *_pdf,Q_ListViewItem *parent,const QString &name=QString::null,Q_ListViewItem *after=NULL);
 TreeItemPdf(TreeData *_data,TreeItemPdf *parent,const QString &name,Q_ListViewItem *after=NULL);
 virtual ~TreeItemPdf();
 CPdf* getObject();
 //From TreeItemAbstract interface
 virtual bool validChild(const QString &name,Q_ListViewItem *oldChild);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,Q_ListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool deepReload(const QString &childName,Q_ListViewItem *oldItem);
 virtual bool haveChild();
 virtual void reloadSelf();
 virtual QSCObject* getQSObject();
 virtual void remove();
private:
 void observePageDict();
 void removeObserver();
 void init(CPdf *pdf,const QString &name);
 void initSpec(CPdf *pdf,const QString &name);
 /** CPdf stored in this TreeItem */
 CPdf *obj;
 /** Node type (Null, Pages,Outlines) */
 QString nType;
 /** Observer registered for this item */
 boost::shared_ptr<TreeItemObserver> observer;
 /** Reference to page dictionary */
 boost::shared_ptr<IProperty> pageDictionary;
 /** Outline list */
 std::vector<OutlineItem> outlines;
};

} // namespace gui

#endif
