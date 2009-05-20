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
/** @file
 "Merge documents pages" dialog<br>
 This dialog is used to select another PDF document and
 merge pages from that document in currently edited document
*/

#include "mergeform.h"
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

namespace gui {

using namespace pdfobjects;

/** Data for ListItem.
 * Gathers ListItem type and page position information.
 */
struct NodeData {
        enum Type{ORIGINAL, FROMFILE};

        /** Type of the item.
         */
        Type type;

        /** Page position in type document.
         */
        int position;

        /** Default constructor.
         * Initializes type to ORIGINAL and position to 0.
         */
        NodeData():type(ORIGINAL), position(0)
        {
        }

        /** Initialization constructor.
         * @param _type Type value.
         * @param _position Position value.
         *
         * Doesn't perform any parameters checking.
         */
        NodeData(Type _type, int _position):type(_type), position(_position)
        {
        }
};

/** List box item.
 * Inherits from QListBoxText item type and adds NodeData data field.
 */
class ListItem:public Q_ListBoxText {
        NodeData * nodeData;
public:
        ListItem(NodeData * _nodeData, Q_ListBox * _parent, const QString & text = QString::null):Q_ListBoxText(_parent, text), nodeData(_nodeData)
        {
        }
        ListItem(NodeData * _nodeData, const QString & text = QString::null):Q_ListBoxText(text), nodeData(_nodeData)
        {
        }
        ListItem(NodeData * _nodeData, Q_ListBox * _parent, const QString & text, Q_ListBoxItem * after):Q_ListBoxText(_parent, text, after), nodeData(_nodeData)
        {
        }

        /** Returns pointer to nodeData field.
         * @return Constant pointer to nodeData.
         */
        const NodeData * getNodeData()const
        {
                return nodeData;
        }
        /** Returns pointer to nodeData field.
         * @return Pointer to nodeData.
         */
        NodeData * getNodeData()
        {
                return nodeData;
        }

        /** Checks wheter this item has node data with given type.
         * @param type Type to compare with.
         * @return true if nodeData is non null and has the same type as given
         * one, false otherwise.
         */
        bool isType(NodeData::Type type)
        {
                return (nodeData) && (nodeData->type == type);
        }
};

void MergeDialog::mergeList_currentChanged( Q_ListBoxItem * item)
{
        ListItem * listItem=dynamic_cast<ListItem *>(item);
        if(listItem==NULL)
                return;

        // enables to manipulate only with FROMFILE nodes
        // original items has to be in same order as in
        // creation time
        if(listItem->isType(NodeData::FROMFILE)) {
                removeBtn->setEnabled(TRUE);

                // up button is enabled only if we are not in first
                // item
                if(mergeList->currentItem()>0)
                        upBtn->setEnabled(TRUE);
                else
                        upBtn->setEnabled(FALSE);

                // down button is enabled only if we are before END page
                // pageCount includes also already added pages
                if(mergeList->currentItem()+1 >= pageCount) 
			downBtn->setEnabled(FALSE);
                else 
			downBtn->setEnabled(TRUE);
        } else {
                upBtn->setEnabled(FALSE);
                downBtn->setEnabled(FALSE);
                removeBtn->setEnabled(FALSE);
        }
}


void MergeDialog::fileList_currentChanged( Q_ListBoxItem * ) {
 // allways enable add button when something is selected
 if(!addBtn->isEnabled()) addBtn->setEnabled(TRUE);
}


void MergeDialog::addBtn_clicked()
{
        int oldPos=-1;
        ListItem * oldItem=NULL;

        for(int i=fileList->count()-1; i>=0; --i)
        {
                // skips unselected items
                if(!fileList->isSelected(i))
                        continue;

                ListItem * fileItem=dynamic_cast<ListItem *>(fileList->item(i));
                if(!fileItem)
                        continue;

                // removes fileItem from fileList
                oldPos=fileList->index(fileItem);
                oldItem=fileItem;
                fileList->takeItem(fileItem);
                if(oldPos>=(int)(fileList->count()))
                        // correction for last item in list
                        --oldPos;

                // gets position of selected item in mergeList
                int pos=mergeList->currentItem();

                // increase total page count
                pageCount++;

                // insert fileItem before currently selected node in mergeList
                mergeList->insertItem(fileItem, (pos<0)?0:pos);
                mergeList->setSelected(fileItem, FALSE);
        }

        // sets new currentItem in mergeList to oldItem
        if(oldItem)
        {
                mergeList->setCurrentItem(oldItem);
                mergeList->setSelected(oldItem, TRUE);
        }

        // nothing has left in fileList box
        if(!fileList->count())
        {
                addBtn->setEnabled(FALSE);
                return;
        }

        // select and mark as current element behind last added
        // to mergeList
        if(oldPos>=0)
        {
                fileList->setCurrentItem(oldPos);
                fileList->setSelected(oldPos, TRUE);
                return;
        }
}


void MergeDialog::removeBtn_clicked()
{
        if(mergeList->selectedItem())
        {
                ListItem * mergeItem=dynamic_cast<ListItem *>(mergeList->selectedItem());
                if(!mergeItem)
                {
                        // nothing selected
                        removeBtn->setEnabled(FALSE);
                        return;
                }
                // removes item from mergeList
                int oldPos=mergeList->index(mergeItem);
                mergeList->takeItem(mergeItem);

                // insert to correct position in fileList - keeps ordering
                int pos=0;
                while(Q_ListBoxItem * item=fileList->item(pos))
                {
                        ListItem * i=dynamic_cast<ListItem *>(item);
                        if(!i)
                                // bad type, this should not happen
                                continue;
                        // finish with first node with higer position
                        // mergeItem will be stored before this position
                        if(i->getNodeData()->position > mergeItem->getNodeData()->position)
                                break;
                        ++pos;
                }

                // update total page count
                pageCount--;

                // inserts mergeItem to the correct position and select the next
                // one in the mergeList
                fileList->insertItem(mergeItem, pos);
                // QT3 bug work-around - currentChanged is not triggered when we add
                // the first item to an empt list - we have to force the handler for
                // proper enable/disable logic
                fileList->setCurrentItem(pos);
                fileList_currentChanged(mergeItem);
                mergeList->setCurrentItem(oldPos);
                mergeList->setSelected(oldPos, TRUE);
        }else
                removeBtn->setEnabled(FALSE);
}


void MergeDialog::upBtn_clicked() {
        int pos=mergeList->currentItem();
        if (pos>0) {
                // current item can be moved upwards
                Q_ListBoxItem * item=mergeList->item(pos);

                // removes and insert with decremented position
                mergeList->takeItem(item);
                mergeList->insertItem(item, pos-1);
                mergeList->setCurrentItem(item);
        } else upBtn->setEnabled(FALSE);
}


void MergeDialog::downBtn_clicked() {
        int pos=mergeList->currentItem();
        if (pos+1<pageCount) {
                Q_ListBoxItem * item=mergeList->item(pos);
                mergeList->takeItem(item);
                mergeList->insertItem(item, pos+1);
                mergeList->setCurrentItem(item);
        } else downBtn->setEnabled(FALSE);
}


void MergeDialog::openBtn_clicked() {
        // Fills fileList with items from opened file
        if(fileNameInput->text().isEmpty()) {
                // TODO dialog that nothing is specified
                return;
        }
        QString fileName=fileNameInput->text();
        initFileList(fileName);
        // TODO handle when file doesn't exist or error occured
        // during CPdf creation

        // Open dialog causes disabling fileNameInput and fileNameBtn
        // and enabling merging stuff
        openBtn->setEnabled(FALSE);
        fileNameInput->setEnabled(FALSE);
        fileNameBtn->setEnabled(FALSE);
        mergeList->setEnabled(TRUE);
        fileList->setEnabled(TRUE);
        okBtn->setEnabled(TRUE);
}


void MergeDialog::fileNameBtn_clicked() {
 QString fileName = QFileDialog::getOpenFileName(
      QString::null, QObject::tr("PDF files (*.pdf)"), this, "file open", tr("Open file with pages to be inserted"));
 if(!fileName.isEmpty()) {
  fileNameInput->setText(fileName);
 }
}


MergeArray<int> * MergeDialog::getResult()
{
        int * pages=new int[mergeList->count()];
        size_t * positions=new size_t[mergeList->count()];
        size_t length=0;
        size_t storePos=1;
        for(size_t pos=0; pos < mergeList->count(); ++pos)
        {
                ListItem * item=dynamic_cast<ListItem*>(mergeList->item(pos));
                if(!item)
                        continue;
                if(item->isType(NodeData::ORIGINAL))
                {
                        ++storePos;
                        continue;
                }
                pages[length]=item->getNodeData()->position;
                positions[length]=storePos;
                ++length;
        }

        MergeArray<int> * mergeArray=new MergeArray<int>(length);
        mergeArray->initItems(pages);
        mergeArray->initPositions(positions);

        return mergeArray;
}

QString MergeDialog::fileName() {
 return fileNameInput->text();
}

void MergeDialog::initOriginal( size_t count ) {
 char itemLabel[128];
 pageCount = count;
 for(size_t i=1; i<=count; ++i) {
  snprintf(itemLabel, 127, "Page%d", (int)i);
  //ListItem * listItem=
  new ListItem(new NodeData(NodeData::ORIGINAL, i), mergeList, itemLabel);
 }
 new ListItem(new NodeData(NodeData::ORIGINAL, count+1), mergeList, 
		 QString("[")+tr("END","end of document marker")+QString("]"));
}


void MergeDialog::destroyOpenFile()
{
        // TODO - close CPdf instance if created
}

/**
 Initialize file list from given file name
 @param fileName name of PDF file
 @return true in case of success, false in case of failure (missing ,broken or unreadable pdf file)
*/
bool MergeDialog::initFileList( QString & fileName ) {
 boost::shared_ptr<CPdf> document;
 CPdf::OpenMode mode=CPdf::ReadOnly;
 try {
  guiPrintDbg(debug::DBG_DBG,"Opening document");
  document=util::getPdfInstance(this,util::convertFromUnicode(fileName,util::NAME),mode);
  assert(document);
  guiPrintDbg(debug::DBG_DBG,"Opened document");
 } catch (PdfOpenException &ex) {
  std::string err;
  ex.getMessage(err);
  //TODO: some messagebox?
  return false;
 }
 size_t count = document->getPageCount();
 document.reset();
 char itemLabel[128];
 QFileInfo fi(fileName);
 QString baseName=fi.baseName(false);
 for(size_t i=1; i<=count; ++i) {
  snprintf(itemLabel, 127, "%s Page%d", baseName.ascii(), (int)i);
  //ListItem * listItem=
  new ListItem(new NodeData(NodeData::FROMFILE, i), fileList, itemLabel);
 }
 return true;
}


/*
 *  Constructs a MergeDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
MergeDialog::MergeDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "MergeDialog" );
    MergeDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "MergeDialogLayout");

    layout48 = new QVBoxLayout( 0, 0, 6, "layout48");

    layout27 = new QHBoxLayout( 0, 0, 6, "layout27");

    textLabel1 = new QLabel( this, "textLabel1" );
    textLabel1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, textLabel1->sizePolicy().hasHeightForWidth() ) );
    layout27->addWidget( textLabel1 );

    fileNameInput = new QLineEdit( this, "fileNameInput" );
    fileNameInput->setEnabled( TRUE );
    layout27->addWidget( fileNameInput );

    fileNameBtn = new QPushButton( this, "fileNameBtn" );
    fileNameBtn->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, fileNameBtn->sizePolicy().hasHeightForWidth() ) );
    fileNameBtn->setMaximumSize( QSize( 40, 32767 ) );
    layout27->addWidget( fileNameBtn );
    layout48->addLayout( layout27 );

    layout2 = new QHBoxLayout( 0, 0, 6, "layout2");
    spacer1 = new QSpacerItem( 131, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer1 );

    openBtn = new QPushButton( this, "openBtn" );
    openBtn->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, openBtn->sizePolicy().hasHeightForWidth() ) );
    layout2->addWidget( openBtn );
    spacer2 = new QSpacerItem( 151, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer2 );
    layout48->addLayout( layout2 );

    line1 = new QFrame( this, "line1" );
    line1->setFrameShape( QFrame::HLine );
    line1->setFrameShadow( QFrame::Sunken );
    line1->setFrameShape( QFrame::HLine );
    layout48->addWidget( line1 );

    layout47 = new QHBoxLayout( 0, 0, 6, "layout47");

    mergeList = new Q_ListBox( this, "mergeList" );
    mergeList->setEnabled( FALSE );
    layout47->addWidget( mergeList );

    layout4 = new QVBoxLayout( 0, 0, 6, "layout4");

    addBtn = new QPushButton( this, "addBtn" );
    addBtn->setEnabled( FALSE );
    layout4->addWidget( addBtn );

    removeBtn = new QPushButton( this, "removeBtn" );
    removeBtn->setEnabled( FALSE );
    layout4->addWidget( removeBtn );
    spacer3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout4->addItem( spacer3 );

    upBtn = new QPushButton( this, "upBtn" );
    upBtn->setEnabled( FALSE );
    layout4->addWidget( upBtn );

    downBtn = new QPushButton( this, "downBtn" );
    downBtn->setEnabled( FALSE );
    layout4->addWidget( downBtn );
    layout47->addLayout( layout4 );

    fileList = new Q_ListBox( this, "fileList" );
    fileList->setEnabled( FALSE );
    fileList->setFrameShape( Q_ListBox::StyledPanel );
    fileList->setSelectionMode( Q_ListBox::Extended );
    layout47->addWidget( fileList );
    layout48->addLayout( layout47 );

    layout6 = new QHBoxLayout( 0, 0, 5, "layout6");

    spacer5 = new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout6->addItem( spacer5 );

    okBtn = new QPushButton( this, "okBtn" );
    okBtn->setEnabled( FALSE );
    layout6->addWidget( okBtn );

    spacer6 = new QSpacerItem( 41, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout6->addItem( spacer6 );

    cancelBtn = new QPushButton( this, "cancelBtn" );
    layout6->addWidget( cancelBtn );

    layout48->addLayout( layout6 );

    MergeDialogLayout->addLayout( layout48, 0, 0 );
    languageChange();
    resize( QSize(759, 380).expandedTo(minimumSizeHint()) );
//    clearWState( WState_Polished );

    // signals and slots connections
    connect( cancelBtn, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( okBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( addBtn, SIGNAL( clicked() ), this, SLOT( addBtn_clicked() ) );
    connect( removeBtn, SIGNAL( clicked() ), this, SLOT( removeBtn_clicked() ) );
    connect( upBtn, SIGNAL( clicked() ), this, SLOT( upBtn_clicked() ) );
    connect( downBtn, SIGNAL( clicked() ), this, SLOT( downBtn_clicked() ) );
    connect( openBtn, SIGNAL( clicked() ), this, SLOT( openBtn_clicked() ) );
    connect( fileNameBtn, SIGNAL( clicked() ), this, SLOT( fileNameBtn_clicked() ) );
#ifdef QT3
    connect( mergeList, SIGNAL( currentChanged(QListBoxItem*) ), this, SLOT( mergeList_currentChanged(QListBoxItem*) ) );
    connect( fileList, SIGNAL( currentChanged(QListBoxItem*) ), this, SLOT( fileList_currentChanged(QListBoxItem*) ) );
#else
    connect( mergeList, SIGNAL( currentChanged(Q3ListBoxItem*) ), this, SLOT( mergeList_currentChanged(Q3ListBoxItem*) ) );
    connect( fileList, SIGNAL( currentChanged(Q3ListBoxItem*) ), this, SLOT( fileList_currentChanged(Q3ListBoxItem*) ) );
#endif

    // tab order
    setTabOrder( fileNameInput, fileNameBtn );
    setTabOrder( fileNameBtn, openBtn );
    setTabOrder( openBtn, addBtn );
    setTabOrder( addBtn, removeBtn );
    setTabOrder( removeBtn, upBtn );
    setTabOrder( upBtn, downBtn );
    setTabOrder( downBtn,  okBtn );
    setTabOrder( okBtn, cancelBtn );

    // buddies
    textLabel1->setBuddy( fileNameInput );
}

/*
 *  Destroys the object and frees any allocated resources
 */
MergeDialog::~MergeDialog() {
 // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void MergeDialog::languageChange() {
 setCaption(tr("Insert pages from another document"));
 textLabel1->setText(tr("&Filename"));
 fileNameBtn->setText("...");
 openBtn->setText(QObject::tr("&Open"));
 openBtn->setAccel(QKeySequence("Alt+O"));
 addBtn->setText("<<");
 removeBtn->setText(">>");
 upBtn->setText(QObject::tr("&Up"));
 upBtn->setAccel(QKeySequence("Alt+U"));
 downBtn->setText(QObject::tr("&Down"));
 downBtn->setAccel(QKeySequence("Alt+D"));
 cancelBtn->setText(QObject::tr("&Cancel"));
 cancelBtn->setAccel(QKeySequence("Alt+C"));
 okBtn->setText(QObject::tr("&Ok"));
 okBtn->setAccel(QKeySequence("Alt+O"));
}

} //namespace gui
