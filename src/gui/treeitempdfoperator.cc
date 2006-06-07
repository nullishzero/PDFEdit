/** @file
 TreeItemPdfOperator - class holding one PDF Operator in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include "treeitempdfoperator.h"
#include "treeitem.h"
#include "treedata.h"
#include "pdfutil.h"
#include "qspdfoperator.h"
#include "util.h"
#include <cobject.h>
#include <ccontentstream.h>
#include <pdfoperators.h>

namespace gui {

using namespace std;
using namespace util;

/**
 constructor of TreeItemPdfOperator - create root item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListView in which to put this item
 @param pdfObj Operator contained in this item
 @param cs Content stream in which this pdf operator is contained
 @param name Internal name of this item (order of operator)
 @param after Item after which this one will be inserted
 */
TreeItemPdfOperator::TreeItemPdfOperator(TreeData *_data,QListView *parent,boost::shared_ptr<PdfOperator> pdfObj,boost::shared_ptr<CContentStream> cs,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(name,_data,parent,after) {
 csRef=cs;
// assert(csRef.get());//This assert was not valid, since contentstreamless operator will have contentstreamless suboperators
 assert(data);
 obj=pdfObj;
 reload();
}

/**
 constructor of TreeItemPdfOperator - create child item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListViewItem under which to put this item
 @param pdfObj Operator contained in this item
 @param cs Content stream in which this pdf operator is contained
 @param name Internal name of this item (order of operator)
 @param after Item after which this one will be inserted
 */
TreeItemPdfOperator::TreeItemPdfOperator(TreeData *_data,QListViewItem *parent,boost::shared_ptr<PdfOperator> pdfObj,boost::shared_ptr<CContentStream> cs,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(name,_data,parent,after) {
 csRef=cs;
// assert(csRef.get());//This assert was not valid, since contentstreamless operator will have contentstreamless suboperators
 assert(data);
 obj=pdfObj;
 reload();
}

/**
 constructor of TreeItemPdfOperator - create child item from given object (without any content stream information)
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListViewItem under which to put this item
 @param pdfObj Operator contained in this item
 @param name Internal name of this item (order of operator)
 @param after Item after which this one will be inserted
 */
TreeItemPdfOperator::TreeItemPdfOperator(TreeData *_data,QListViewItem *parent,boost::shared_ptr<PdfOperator> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(name,_data,parent,after) {
 assert(data);
 obj=pdfObj;
 reload();
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemPdfOperator::createChild(const QString &name,__attribute__((unused)) ChildType typ,QListViewItem *after/*=NULL*/) {
 int position=name.toInt();
 if (position>=0) {//Operator
  return new TreeItemPdfOperator(data,this,op[position],csRef,name,after);
 } else { //Operand
  guiPrintDbg(debug::DBG_DBG,"Position " << position);
  position=-position-1;
  guiPrintDbg(debug::DBG_DBG,"Position mod " << position);
  assert(params[position].get());
  QString xName=QObject::tr("Parameter")+" "+QString::number(position);
  return TreeItem::create(data,this,params[position],xName,after); 
 }
}

/**
 Return PDF operator inside this tree items
 @return PDF operator inside this tree items
*/
boost::shared_ptr<PdfOperator> TreeItemPdfOperator::getObject() {
 return obj;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemPdfOperator::validChild(const QString &name,QListViewItem *oldChild) {
 int position=name.toInt();
 if (position>=0) {//Check Operator
  TreeItemPdfOperator *it=dynamic_cast<TreeItemPdfOperator*>(oldChild);
  if (!it) return false;
  return op[position]==it->getObject();
 } else { //Check Operand
  TreeItem *it=dynamic_cast<TreeItem*>(oldChild);
  position=-position-1;
  assert(params[position].get());
  return params[position]==it->getObject();
 }
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemPdfOperator::getChildType(__attribute__((unused)) const QString &name) {
 return 1;//Just one type : PDF Operator or Operand
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemPdfOperator::getChildNames() {
 assert(data);
 //Operators
 PdfOperator::PdfOperators opList;
 obj->getChildren(opList);
 //We need vector. We get list. We must copy.
 op.clear();
 std::copy(opList.begin(),opList.end(),std::back_inserter(op));
 //Operands
 PdfOperator::Operands paramList;
 obj->getParameters(paramList);
 //We need vector. We get deque. We must copy.
 params.clear();
 std::copy(paramList.begin(),paramList.end(),std::back_inserter(params));
 QStringList childs;
 for (size_t o=0;o<params.size();o++) {
  childs+=QString::number(-((int)o)-1);
 }
 for (size_t i=0;i<op.size();i++) {
  childs+=QString::number(i);
 }
 return childs;
}


//See TreeItemAbstract for description of this virtual method
bool TreeItemPdfOperator::haveChild() {
 return obj->getChildrenCount()+obj->getParametersCount()>0;
}

/** default destructor */
TreeItemPdfOperator::~TreeItemPdfOperator() {
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemPdfOperator::getQSObject() {
 return new QSPdfOperator(obj,csRef,data->base());
}

//See TreeItemAbstract for description of this virtual method
void TreeItemPdfOperator::remove() {
 if (!obj->getContentStream()) return;// Content stream is not known, so deletion is not possible
 TreeItemAbstract *parentItem=dynamic_cast<TreeItemAbstract*>(parent());
 obj->getContentStream()->deleteOperator(obj);
 //TODO: monitor parentItem for deletion, or maybe this should not be necessary (observers ..)
 if (parentItem) parentItem->reload(); ///This will delete the operator treeitem and also the operator itself
 return;
}

//See TreeItemAbstract for description of this virtual method
void TreeItemPdfOperator::reloadSelf() {
 std::string operatorName;
 obj->getOperatorName(operatorName);
 setText(0,operatorName);
 // object type
 setText(1,QObject::tr("Operator"));
 int paramCount=obj->getParametersCount();
 int childCount=obj->getChildrenCount();
 QString params="";
 if (paramCount) params=countString(paramCount,"parameter","parameters");
 else if (childCount) params=countString(childCount,"child","childs");
 setText(2,params);//params
}

} // namespace gui
