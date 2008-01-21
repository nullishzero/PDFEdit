/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
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
