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
#ifndef __TREEITEMCONTENTSTREAM_H__
#define __TREEITEMCONTENTSTREAM_H__

#include "treeitemabstract.h"
#include <boost/shared_ptr.hpp>
#include <vector>

class QString;
namespace pdfobjects {
 class IProperty;
 class PdfOperator;
 class CContentStream;
}

namespace gui {

using namespace pdfobjects;

class TreeData;
class TreeItemContentStreamObserver;

/** What should be shown as children under tree item wit Content stream */
typedef enum {
/** Show all operators */		All,
/** Show only text operators */		Text,
/** Show only font operators */		Font,
/** Show only graphical operators */	Graphic,
 } TreeItemContentStreamMode;

/**
 Class holding one Content Stream object in tree<br>
 On attempting to open the item in main window, the item opens itself automatically in secondary tree window.<br>
 This is to avoid cluttering the main window with too many items, making navigation in main tree difficult for user.
 \brief Tree item containing CContentStream
*/
class TreeItemContentStream : public TreeItemAbstract {
public:
 boost::shared_ptr<CContentStream> getObject();
 TreeItemContentStream(TreeData *_data,Q_ListView *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name=QString::null,Q_ListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemContentStream(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name=QString::null,Q_ListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemContentStream();
 virtual void setOpen(bool open);
 //From TreeItemAbstract interface
 virtual bool validChild(const QString &name,Q_ListViewItem *oldChild);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,Q_ListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
 virtual QSCObject* getQSObject();
 virtual void remove();
 virtual void reloadSelf();
 void setMode(TreeItemContentStreamMode newMode);
 void setMode(const QString &newMode);
 QString getMode();
private:
 void showMode();
 void initObserver();
 void uninitObserver();
private:
 void init(const QString &name);
 /**  ContentStream object held in this item */
 boost::shared_ptr<CContentStream> obj;
 /** Vector with pdf operators */
 std::vector<boost::shared_ptr<PdfOperator> > op;
 /** Observer registered for this item */
 boost::shared_ptr<TreeItemContentStreamObserver> observer;
 /** Mode - what should be shown? */
 TreeItemContentStreamMode mode;
};

} // namespace gui

#endif
