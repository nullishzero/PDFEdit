/** @file
 TreeItemPdfOperator - class holding one PDF Operator in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include <cobject.h>
#include "treeitempdfoperator.h"
#include "treeitem.h"
#include "treedata.h"
#include "pdfutil.h"
#include "util.h"

namespace gui {

using namespace std;
using namespace util;

/**
 constructor of TreeItemPdfOperator - create root item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListView in which to put this item
 @param pdfObj Operator contained in this item
 @param name Internal name of this item (order of operator)
 @param after Item after which this one will be inserted
 */
TreeItemPdfOperator::TreeItemPdfOperator(TreeData *_data,QListView *parent,boost::shared_ptr<PdfOperator> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(name,_data,parent,after) {
 assert(data);
 obj=pdfObj;
 reload();
}

/**
 constructor of TreeItemPdfOperator - create child item from given object
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
  return new TreeItemPdfOperator(data,this,op[position],name,after);
 } else { //Operand
  guiPrintDbg(debug::DBG_DBG,"Position " << position);
  position=-position-1;
  guiPrintDbg(debug::DBG_DBG,"Position mod " << position);
  assert(params[position].get());
  QString xName=QObject::tr("Parameter")+" "+QString::number(position);
  return TreeItem::create(data,this,params[position],xName,after); 
 }
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemPdfOperator::getChildType(const QString &name) {
 return 1;//Just one type : PDF Operator or Operand
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemPdfOperator::getChildNames() {
 assert(data);

 //Operators
 PdfOperator::PdfOperators opList;
 guiPrintDbg(debug::DBG_DBG,"");
// if (obj->getChildrenCount()) 
 obj->getChildren(opList);
 //We need vector. We get list. We must copy.
 guiPrintDbg(debug::DBG_DBG,"");
 op.clear();
 std::copy(opList.begin(),opList.end(),std::back_inserter(op));
 //Operands
 PdfOperator::Operands paramList;
 obj->getParameters(paramList);
 //We need vector. We get deque. We must copy.
 params.clear();
 std::copy(paramList.begin(),paramList.end(),std::back_inserter(params));
 QStringList childs;
 for (int o=0;o<params.size();o++) {
  childs+=QString::number(-o-1);
 }
 for (int i=0;i<op.size();i++) {
  childs+=QString::number(i);
 }
 //TODO: support property editor on operator
 return childs;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemPdfOperator::validChild(const QString &name,QListViewItem *oldChild) {
 //TODO: do the validation
 return false;//We're not sure
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
 //TODO: need to create the wrapper
 return NULL;
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemPdfOperator::getQSObject(Base *_base) {
 //TODO: need to create the wrapper
 return NULL;
}

//See TreeItemAbstract for description of this virtual method
void TreeItemPdfOperator::remove() {
 //TODO: remove operator?
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
 if (paramCount) params=QString::number(paramCount)+QObject::tr(" parameters");
 else if (childCount) params=QString::number(childCount)+QObject::tr(" childs");
 setText(2,params);//params
 return;
}

} // namespace gui
