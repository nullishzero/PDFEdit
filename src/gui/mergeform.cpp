/****************************************************************************
** Form implementation generated from reading ui file 'mergeform.ui'
**
** Created: Ne aug 20 19:32:12 2006
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "mergeform.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/** Data for ListItem.
 * Gathers ListItem type and page position information.
 */
struct NodeData
{
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
class ListItem:public QListBoxText
{
        NodeData * nodeData;
public:
        ListItem(NodeData * _nodeData, QListBox * _parent, const QString & text = QString::null):QListBoxText(_parent, text), nodeData(_nodeData)
        {
        }
        ListItem(NodeData * _nodeData, const QString & text = QString::null):QListBoxText(text), nodeData(_nodeData)
        {
        }
        ListItem(NodeData * _nodeData, QListBox * _parent, const QString & text, QListBoxItem * after):QListBoxText(_parent, text, after), nodeData(_nodeData)
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

void MergeDialog::mergeList_currentChanged( QListBoxItem * item)
{
        ListItem * listItem=dynamic_cast<ListItem *>(item);
        if(listItem==NULL)
                return;

        // enables to manipulate only with FROMFILE nodes
        // original items has to be in same order as in 
        // creation time
        if(listItem->isType(NodeData::FROMFILE))
        {
                removeBtn->setEnabled(TRUE);

                // up button is enabled only if we are not in first
                // item
                if(mergeList->currentItem()>0)
                        upBtn->setEnabled(TRUE);
                else 
                        upBtn->setEnabled(FALSE);
                
                // down button is enabled only if we are not on last
                // item
                if(mergeList->currentItem() + 1>=mergeList->count())
                        downBtn->setEnabled(FALSE);
                else
                        downBtn->setEnabled(TRUE);
        }else
        {
                upBtn->setEnabled(FALSE);
                downBtn->setEnabled(FALSE);
                removeBtn->setEnabled(FALSE);
        }
}


void MergeDialog::fileList_currentChanged( QListBoxItem * )
{
        // allways enable add button when something is selected
        if(!addBtn->isEnabled())
                addBtn->setEnabled(TRUE);
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
                if(oldPos>=fileList->count())
                        // correction for last item in list
                        --oldPos;

                // gets position of selected item in mergeList
                int pos=mergeList->currentItem();

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
                while(QListBoxItem * item=fileList->item(pos))
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

                // inserts mergeItem to correct position and unselect it
                fileList->insertItem(mergeItem, pos);
                fileList->setCurrentItem(mergeItem);
                mergeList->setCurrentItem(oldPos);
                mergeList->setSelected(oldPos, TRUE);
        }else
                removeBtn->setEnabled(FALSE);
}


void MergeDialog::upBtn_clicked()
{
        int pos=mergeList->currentItem();
        if (pos>0)
        {
                // current item can be moved upwards
                QListBoxItem * item=mergeList->item(pos);

                // removes and insert with decremented position
                mergeList->takeItem(item);
                mergeList->insertItem(item, pos-1);
                mergeList->setCurrentItem(item);
        }else
                upBtn->setEnabled(FALSE);
}


void MergeDialog::downBtn_clicked()
{
        int pos=mergeList->currentItem();
        if (pos<mergeList->count())
        {
                QListBoxItem * item=mergeList->item(pos);
                mergeList->takeItem(item);
                mergeList->insertItem(item, pos+1);
                mergeList->setCurrentItem(item);
        }else
                downBtn->setEnabled(FALSE);
}


void MergeDialog::openBtn_clicked()
{
        // Fills fileList with items from opened file
        if(fileNameInput->text().isEmpty())
        {
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


void MergeDialog::fileNameBtn_clicked()
{
        QString fileName = QFileDialog::getOpenFileName(
                                QString::null, "Pdf document (*.pdf)", this,
                                "file open", "Merge dialog -- File Open" );
        if(!fileName.isEmpty())
        {
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


void MergeDialog::initOriginal( size_t count )
{
        char itemLabel[128];
        for(size_t i=1; i<=count; ++i)
        {
                snprintf(itemLabel, 127, "Page%d", i);
                ListItem * listItem=new ListItem(new NodeData(NodeData::ORIGINAL, i), mergeList, itemLabel);
        }

}


void MergeDialog::destroyOpenFile()
{
        // TODO - close CPdf instance if created
}


void MergeDialog::initFileList( QString & fileName )
{
        // FIXME - open CPdf, get page count
        size_t count = 10;
        char itemLabel[128];
        for(size_t i=1; i<=count; ++i)
        {
                // FIXME use just base name not whole fileName with path
                snprintf(itemLabel, 127, "%s Page%d", fileName.ascii(), i);
                ListItem * listItem=new ListItem(new NodeData(NodeData::FROMFILE, i), fileList, itemLabel);
        }
}

/******************************************************************************
 * Code generated by qt designer .ui and .moc generated 
 ******************************************************************************/

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

    mergeList = new QListBox( this, "mergeList" );
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

    fileList = new QListBox( this, "fileList" );
    fileList->setEnabled( FALSE );
    fileList->setFrameShape( QListBox::StyledPanel );
    fileList->setSelectionMode( QListBox::Extended );
    layout47->addWidget( fileList );
    layout48->addLayout( layout47 );

    layout6 = new QHBoxLayout( 0, 0, 6, "layout6"); 
    spacer5 = new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout6->addItem( spacer5 );

    cancelBtn = new QPushButton( this, "cancelBtn" );
    layout6->addWidget( cancelBtn );
    spacer4 = new QSpacerItem( 41, 21, QSizePolicy::Fixed, QSizePolicy::Minimum );
    layout6->addItem( spacer4 );

    okBtn = new QPushButton( this, "okBtn" );
    okBtn->setEnabled( FALSE );
    layout6->addWidget( okBtn );
    spacer6 = new QSpacerItem( 81, 31, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout6->addItem( spacer6 );
    layout48->addLayout( layout6 );

    MergeDialogLayout->addLayout( layout48, 0, 0 );
    languageChange();
    resize( QSize(759, 380).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( cancelBtn, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( okBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( addBtn, SIGNAL( clicked() ), this, SLOT( addBtn_clicked() ) );
    connect( removeBtn, SIGNAL( clicked() ), this, SLOT( removeBtn_clicked() ) );
    connect( upBtn, SIGNAL( clicked() ), this, SLOT( upBtn_clicked() ) );
    connect( downBtn, SIGNAL( clicked() ), this, SLOT( downBtn_clicked() ) );
    connect( openBtn, SIGNAL( clicked() ), this, SLOT( openBtn_clicked() ) );
    connect( fileNameBtn, SIGNAL( clicked() ), this, SLOT( fileNameBtn_clicked() ) );
    connect( mergeList, SIGNAL( currentChanged(QListBoxItem*) ), this, SLOT( mergeList_currentChanged(QListBoxItem*) ) );
    connect( fileList, SIGNAL( currentChanged(QListBoxItem*) ), this, SLOT( fileList_currentChanged(QListBoxItem*) ) );

    // tab order
    setTabOrder( fileNameInput, fileNameBtn );
    setTabOrder( fileNameBtn, openBtn );
    setTabOrder( openBtn, addBtn );
    setTabOrder( addBtn, removeBtn );
    setTabOrder( removeBtn, upBtn );
    setTabOrder( upBtn, downBtn );
    setTabOrder( downBtn, cancelBtn );
    setTabOrder( cancelBtn, okBtn );

    // buddies
    textLabel1->setBuddy( fileNameInput );
}

/*
 *  Destroys the object and frees any allocated resources
 */
MergeDialog::~MergeDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void MergeDialog::languageChange()
{
    setCaption( tr( "Merge dialog" ) );
    textLabel1->setText( tr( "&FileName" ) );
    fileNameBtn->setText( tr( "..." ) );
    openBtn->setText( tr( "&Open" ) );
    openBtn->setAccel( QKeySequence( tr( "Alt+O" ) ) );
    addBtn->setText( tr( "<<" ) );
    removeBtn->setText( tr( ">>" ) );
    upBtn->setText( tr( "&Up" ) );
    upBtn->setAccel( QKeySequence( tr( "Alt+U" ) ) );
    downBtn->setText( tr( "&Down" ) );
    downBtn->setAccel( QKeySequence( tr( "Alt+D" ) ) );
    cancelBtn->setText( tr( "&Cancel" ) );
    cancelBtn->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    okBtn->setText( tr( "&OK" ) );
    okBtn->setAccel( QKeySequence( tr( "Alt+O" ) ) );
}



/****************************************************************************
** MergeDialog meta object code from reading C++ file 'mergeform.h'
**
** Created: Sun Aug 20 19:32:31 2006
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "mergeform.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *MergeDialog::className() const
{
    return "MergeDialog";
}

QMetaObject *MergeDialog::metaObj = 0;
static QMetaObjectCleanUp cleanUp_MergeDialog( "MergeDialog", &MergeDialog::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString MergeDialog::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MergeDialog", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString MergeDialog::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MergeDialog", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* MergeDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "item", &static_QUType_ptr, "QListBoxItem", QUParameter::In }
    };
    static const QUMethod slot_0 = {"mergeList_currentChanged", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_ptr, "QListBoxItem", QUParameter::In }
    };
    static const QUMethod slot_1 = {"fileList_currentChanged", 1, param_slot_1 };
    static const QUMethod slot_2 = {"addBtn_clicked", 0, 0 };
    static const QUMethod slot_3 = {"removeBtn_clicked", 0, 0 };
    static const QUMethod slot_4 = {"upBtn_clicked", 0, 0 };
    static const QUMethod slot_5 = {"downBtn_clicked", 0, 0 };
    static const QUMethod slot_6 = {"openBtn_clicked", 0, 0 };
    static const QUMethod slot_7 = {"fileNameBtn_clicked", 0, 0 };
    static const QUMethod slot_8 = {"languageChange", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "mergeList_currentChanged(QListBoxItem*)", &slot_0, QMetaData::Public },
	{ "fileList_currentChanged(QListBoxItem*)", &slot_1, QMetaData::Public },
	{ "addBtn_clicked()", &slot_2, QMetaData::Public },
	{ "removeBtn_clicked()", &slot_3, QMetaData::Public },
	{ "upBtn_clicked()", &slot_4, QMetaData::Public },
	{ "downBtn_clicked()", &slot_5, QMetaData::Public },
	{ "openBtn_clicked()", &slot_6, QMetaData::Public },
	{ "fileNameBtn_clicked()", &slot_7, QMetaData::Public },
	{ "languageChange()", &slot_8, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"MergeDialog", parentObject,
	slot_tbl, 9,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_MergeDialog.setMetaObject( metaObj );
    return metaObj;
}

void* MergeDialog::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "MergeDialog" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool MergeDialog::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: mergeList_currentChanged((QListBoxItem*)static_QUType_ptr.get(_o+1)); break;
    case 1: fileList_currentChanged((QListBoxItem*)static_QUType_ptr.get(_o+1)); break;
    case 2: addBtn_clicked(); break;
    case 3: removeBtn_clicked(); break;
    case 4: upBtn_clicked(); break;
    case 5: downBtn_clicked(); break;
    case 6: openBtn_clicked(); break;
    case 7: fileNameBtn_clicked(); break;
    case 8: languageChange(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool MergeDialog::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool MergeDialog::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool MergeDialog::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
