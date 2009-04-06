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
/*
 * =====================================================================================
 *        Filename:  selectpagesdialog.cc
 *         Created:  03/12/2006 07:41:44 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#include "selectpagesdialog.h"
#include "pdfutil.h"
#include "util.h"
#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qframe.h>
#include QLISTBOX
#include <qlayout.h>
#include <kernel/cpdf.h>

//=====================================================================================
namespace gui {
//=====================================================================================

using namespace std;
using namespace pdfobjects;


//=====================================================================================
namespace {
//=====================================================================================

	/**
	 * List box item.
	 * Inherits from QListBoxText item type and adds NodeData data field.
	 */
	struct ListItem : public Q_ListBoxText {
			typedef size_t Position;

		protected:
			Position _position;

			//
			// Ctor
			//
		public:
			ListItem (Position pos, Q_ListBox* _parent, const QString& text = QString::null)
				: Q_ListBoxText(_parent, text),_position(pos) 	{}
			ListItem (Position pos, const QString&  text = QString::null)
				: Q_ListBoxText(text),_position(pos) {}
			ListItem (Position pos, Q_ListBox* _parent, const QString& text, Q_ListBoxItem* after)
				: Q_ListBoxText(_parent, text, after),_position(pos) {}

			/**
			 * Returns position.
			 */
			Position position () const
				{ return _position; }
	};

//=====================================================================================
} // namespace
//=====================================================================================


//=====================================================================================
// SelectPagesDialog
//=====================================================================================

//
// Constructor
//
SelectPagesDialog::SelectPagesDialog (const QString& filename) : QDialog (NULL, NULL, false, 0) {
	setName ("Select page dialog.");
    SelectPagesDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "SelectPagesDialogLayout");

    layout48 = new QVBoxLayout();
    layout48->setMargin(0);
    layout48->setSpacing(6);
    layout27 = new QHBoxLayout();
    layout27->setMargin(0);
    layout27->setSpacing(6);

    textLabel1 = new QLabel( this);
	textLabel1->setText (filename);
    textLabel1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, textLabel1->sizePolicy().hasHeightForWidth() ) );
    layout27->addWidget( textLabel1 );
    layout48->addLayout( layout27 );
    layout47 = new QHBoxLayout();
    layout47->setMargin(0);
    layout47->setSpacing(6);
	// selectedList
    selectedList = new Q_ListBox( this, "selectedList" );
    selectedList->setEnabled( FALSE );
    layout47->addWidget( selectedList );
    layout4 = new QVBoxLayout( 0, 0, 6, "layout4");
	// Add buttno
    addBtn = new QPushButton( this, "addBtn" );
    addBtn->setEnabled( FALSE );
    layout4->addWidget( addBtn );
	// Remove button
    removeBtn = new QPushButton( this, "removeBtn" );
    removeBtn->setEnabled( FALSE );
    layout4->addWidget( removeBtn );
    spacer3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout4->addItem( spacer3 );
    layout47->addLayout( layout4 );
	// Filelist
    originalList = new Q_ListBox( this, "originalList" );
    originalList->setEnabled( FALSE );
    originalList->setFrameShape( Q_ListBox::StyledPanel );
    originalList->setSelectionMode( Q_ListBox::Extended );
    layout47->addWidget( originalList );
    layout48->addLayout( layout47 );
    layout6 = new QHBoxLayout( 0, 0, 5, "layout6");
    spacer5 = new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout6->addItem( spacer5 );
	// Ok button
    okBtn = new QPushButton( this, "okBtn" );
    okBtn->setEnabled( FALSE );
    layout6->addWidget( okBtn );
    spacer6 = new QSpacerItem( 41, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout6->addItem( spacer6 );
	// Cancel button
    cancelBtn = new QPushButton( this, "cancelBtn" );
    layout6->addWidget( cancelBtn );
    layout48->addLayout( layout6 );
    SelectPagesDialogLayout->addLayout( layout48, 0, 0 );

    languageChange();
    resize( QSize(759, 380).expandedTo(minimumSizeHint()) );
//    clearWState( WState_Polished );

    // signals and slots connections
    connect( cancelBtn, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( okBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( addBtn, SIGNAL( clicked() ), this, SLOT( addBtn_clicked() ) );
    connect( removeBtn, SIGNAL( clicked() ), this, SLOT( removeBtn_clicked() ) );
#ifdef QT3
    connect( selectedList, SIGNAL( currentChanged(QListBoxItem*) ), this, SLOT( selectedList_changed(QListBoxItem*) ) );
    connect( originalList, SIGNAL( currentChanged(QListBoxItem*) ), this, SLOT( originalList_changed(QListBoxItem*) ) );
#else
    connect( selectedList, SIGNAL( currentChanged(Q3ListBoxItem*) ), this, SLOT( selectedList_changed(Q3ListBoxItem*) ) );
    connect( originalList, SIGNAL( currentChanged(Q3ListBoxItem*) ), this, SLOT( originalList_changed(Q3ListBoxItem*) ) );
#endif

    // tab order
    setTabOrder( addBtn, removeBtn );
    setTabOrder( removeBtn, okBtn );
    setTabOrder( okBtn, cancelBtn );

	//
	// Init pages
	//
	init (filename);
	selectedList->setEnabled (TRUE);
	originalList->setEnabled (TRUE);
    okBtn->setEnabled (TRUE);
}


//
// Event handlers
//

//
//
//
void SelectPagesDialog::originalList_changed (Q_ListBoxItem* item) {
	ListItem * listItem=dynamic_cast<ListItem *>(item);
	if(listItem==NULL)
			return;
			
	addBtn->setEnabled(TRUE);
}


//
//
//
void SelectPagesDialog::selectedList_changed (Q_ListBoxItem*) {
	// allways enable add button when something is selected
	removeBtn->setEnabled(TRUE);
}

//
// !source code copied from MergeDialog!
//
void SelectPagesDialog::addBtn_clicked () {
	int oldPos=-1;
	ListItem * oldItem=NULL;

	for(int i=originalList->count()-1; i>=0; --i)
	{
			// skips unselected items
			if(!originalList->isSelected(i))
					continue;
			
			ListItem * fileItem=dynamic_cast<ListItem *>(originalList->item(i));
			if(!fileItem)
					continue;

			// removes fileItem from originalList
			oldPos=originalList->index (fileItem);
			oldItem=fileItem;
			originalList->takeItem(fileItem);
			if(oldPos>=(int)(originalList->count()))
					// correction for last item in list
					--oldPos;

			// gets position of selected item in selectedList
			int pos=selectedList->currentItem();

			// insert fileItem before currently selected node in selectedList
			selectedList->insertItem(fileItem, (pos<0)?0:pos);
			selectedList->setSelected(fileItem, FALSE);
	}

	// sets new currentItem in selectedList to oldItem
	if(oldItem)
	{
			selectedList->setCurrentItem(oldItem);
			selectedList->setSelected(oldItem, TRUE);
	}

	// nothing has left in originalList box
	if(!originalList->count())
	{
			addBtn->setEnabled(FALSE);
			return;
	}
	
	// select and mark as current element behind last added
	// to selectedList
	if(oldPos>=0)
	{
			originalList->setCurrentItem(oldPos);
			originalList->setSelected(oldPos, TRUE);
			return;
	}
}


//
// !source code copied from MergeDialog!
//
void SelectPagesDialog::removeBtn_clicked () {
	if(selectedList->selectedItem())
	{
			ListItem * mergeItem=dynamic_cast<ListItem *>(selectedList->selectedItem());
			if(!mergeItem)
			{
					// nothing selected
					removeBtn->setEnabled(FALSE);
					return;
			}
			// removes item from selectedList
			int oldPos=selectedList->index(mergeItem);
			selectedList->takeItem(mergeItem);

			// insert to correct position in originalList - keeps ordering
			int pos=0;
			while(Q_ListBoxItem * item=originalList->item(pos))
			{
					ListItem * i=dynamic_cast<ListItem *>(item);
					if(!i)
							// bad type, this should not happen
							continue;
					// finish with first node with higer position
					// mergeItem will be stored before this position
					if(i->position() > mergeItem->position())
							break;
					++pos;
			}

			// inserts mergeItem to correct position and unselect it
			originalList->insertItem(mergeItem, pos);
			originalList->setCurrentItem(mergeItem);
			selectedList->setCurrentItem(oldPos);
			selectedList->setSelected(oldPos, TRUE);
	}else
			removeBtn->setEnabled(FALSE);
}


//
// Helper functions
//

template<typename Container> void SelectPagesDialog::getResult (Container& cont) const {
	for (size_t pos = 0; pos < selectedList->count(); ++pos)
	{
		ListItem* item = dynamic_cast<ListItem*>(selectedList->item(pos));
		if(!item)
			continue;
		cont.push_back (item->position());
	}
}
// Explicit instantiation
template void SelectPagesDialog::getResult<list<size_t> > (list<size_t>& cont) const;

//
//
//
void SelectPagesDialog::init (size_t count) {
 char itemLabel[128];
 for(size_t i=1; i<=count; ++i) {
  snprintf(itemLabel, 127, "Page%d", (int)i);
  new ListItem (i, originalList, itemLabel);
 }

}

//
//
//
bool SelectPagesDialog::init (const QString& fileName) {
	boost::shared_ptr<CPdf> document;
	CPdf::OpenMode mode = CPdf::ReadOnly;

	try {

		guiPrintDbg (debug::DBG_DBG,"Opening document.");
		document = util::getPdfInstance (this,fileName,mode);
		assert(document);
		guiPrintDbg (debug::DBG_DBG,"Document opened.");
	
	} catch (PdfOpenException& e) {
		std::string err;
		e.getMessage(err);
		//TODO: some messagebox?
		guiPrintDbg(debug::DBG_DBG,"Failed opening document " << err);
		return false;
	}

	// Init pages
	init (document->getPageCount());
	document.reset();
	
	return true;
}


//
//  Sets the strings of the subwidgets using the current language.
//
void SelectPagesDialog::languageChange() {
 setCaption(tr("Convert pdf to xml"));
 textLabel1->setText(tr(textLabel1->text()));
 addBtn->setText("<<");
 removeBtn->setText(">>");
 cancelBtn->setText(QObject::tr("&Cancel"));
 cancelBtn->setAccel(QKeySequence("Alt+C"));
 okBtn->setText(QObject::tr("&Ok"));
 okBtn->setAccel(QKeySequence("Alt+O"));
}

//=====================================================================================
} //namespace gui
//=====================================================================================
