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
 "Add Annotation" dialog<br>
 This dialog is used to add annotation in PDF document
*/
#include "annotdialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include QGROUPBOX
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include "kernel/cpage.h"
#include "kernel/cannotation.h"
#include "annotdialog.h"

namespace gui {

/*
 *  Constructs a AnnotDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AnnotDialog::AnnotDialog(QSPage & p, QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ), page(p)
{
    if ( !name )
	setName( "AnnotDialog" );
    AnnotDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "AnnotDialogLayout");

    layout8 = new QHBoxLayout( 0, 0, 6, "layout8");
    spacer8 = new QSpacerItem( 50, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout8->addItem( spacer8 );

    okBtn = new QPushButton( this, "okBtn" );
    layout8->addWidget( okBtn );
    spacer7 = new QSpacerItem( 40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    layout8->addItem( spacer7 );

    cancelButton = new QPushButton( this, "cancelButton" );
    layout8->addWidget( cancelButton );
    spacer9 = new QSpacerItem( 50, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout8->addItem( spacer9 );

    AnnotDialogLayout->addLayout( layout8, 1, 0 );

    tabWidget3 = new QTabWidget( this, "tabWidget3" );

    tab = new QWidget( tabWidget3, "tab" );
    tabLayout = new QGridLayout( tab, 1, 1, 11, 6, "tabLayout");

    groupBox1 = new Q_GroupBox( tab, "groupBox1" );
    groupBox1->setColumnLayout(0, Qt::Vertical );
    groupBox1->layout()->setSpacing( 6 );
    groupBox1->layout()->setMargin( 11 );
    groupBox1Layout = new QGridLayout( groupBox1->layout() );
    groupBox1Layout->setAlignment( Qt::AlignTop );

    layout21_2 = new QVBoxLayout( 0, 0, 6, "layout21_2");

    open = new QCheckBox( groupBox1, "open" );
    layout21_2->addWidget( open );

    layout16_2 = new QHBoxLayout( 0, 0, 6, "layout16_2");

    textLabel2 = new QLabel( groupBox1, "textLabel2" );
    layout16_2->addWidget( textLabel2 );

    textContent = new QLineEdit( groupBox1, "textContent" );
    layout16_2->addWidget( textContent );
    layout21_2->addLayout( layout16_2 );

    layout18_2 = new QHBoxLayout( 0, 0, 6, "layout18_2");

    textLabel3 = new QLabel( groupBox1, "textLabel3" );
    textLabel3->setMidLineWidth( -6 );
    layout18_2->addWidget( textLabel3 );

    iconName = new QComboBox( FALSE, groupBox1, "iconName" );
    layout18_2->addWidget( iconName );
    layout21_2->addLayout( layout18_2 );

    layout19_2 = new QHBoxLayout( 0, 0, 6, "layout19_2");

    textLabel4_2 = new QLabel( groupBox1, "textLabel4_2" );
    textLabel4_2->setMargin( 0 );
    layout19_2->addWidget( textLabel4_2 );

    state = new QComboBox( FALSE, groupBox1, "state" );
    layout19_2->addWidget( state );
    layout21_2->addLayout( layout19_2 );

    layout20_2 = new QHBoxLayout( 0, 0, 6, "layout20_2");

    textLabel5_2 = new QLabel( groupBox1, "textLabel5_2" );
    layout20_2->addWidget( textLabel5_2 );

    stateModel = new QComboBox( FALSE, groupBox1, "stateModel" );
    layout20_2->addWidget( stateModel );
    layout21_2->addLayout( layout20_2 );

    groupBox1Layout->addLayout( layout21_2, 0, 0 );

    tabLayout->addWidget( groupBox1, 0, 0 );
    tabWidget3->insertTab( tab, QString::fromLatin1("") );

    tab_2 = new QWidget( tabWidget3, "tab_2" );
    tabLayout_2 = new QGridLayout( tab_2, 1, 1, 11, 6, "tabLayout_2");

    frame3 = new QFrame( tab_2, "frame3" );
    frame3->setFrameShape( QFrame::StyledPanel );
    frame3->setFrameShadow( QFrame::Raised );
    frame3Layout = new QGridLayout( frame3, 1, 1, 11, 6, "frame3Layout");

    layout17 = new QVBoxLayout( 0, 0, 6, "layout17");

    layout12 = new QVBoxLayout( 0, 0, 6, "layout12");

    layout16_2_2 = new QHBoxLayout( 0, 0, 6, "layout16_2_2");

    textLabel2_3 = new QLabel( frame3, "textLabel2_3" );
    layout16_2_2->addWidget( textLabel2_3 );

    linkContent = new QLineEdit( frame3, "linkContent" );
    layout16_2_2->addWidget( linkContent );
    layout12->addLayout( layout16_2_2 );

    layout8_2 = new QHBoxLayout( 0, 0, 6, "layout8_2");

    textLabel1 = new QLabel( frame3, "textLabel1" );
    textLabel1->setFrameShape( QLabel::NoFrame );
    layout8_2->addWidget( textLabel1 );

    destination = new QLineEdit( frame3, "destination" );
    layout8_2->addWidget( destination );
    layout12->addLayout( layout8_2 );

    layout9 = new QHBoxLayout( 0, 0, 6, "layout9");

    textLabel2_2 = new QLabel( frame3, "textLabel2_2" );
    textLabel2_2->setFrameShape( QLabel::NoFrame );
    layout9->addWidget( textLabel2_2 );

    highLight = new QComboBox( FALSE, frame3, "highLight" );
    highLight->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, highLight->sizePolicy().hasHeightForWidth() ) );
    layout9->addWidget( highLight );
    layout12->addLayout( layout9 );
    layout17->addLayout( layout12 );
    spacer4 = new QSpacerItem( 21, 81, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout17->addItem( spacer4 );

    frame3Layout->addLayout( layout17, 0, 0 );

    tabLayout_2->addWidget( frame3, 0, 0 );
    tabWidget3->insertTab( tab_2, QString::fromLatin1("") );

    AnnotDialogLayout->addWidget( tabWidget3, 0, 0 );
    languageChange();
    resize( QSize(477, 341).expandedTo(minimumSizeHint()) );

    // signals and slots connections
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( stateModel, SIGNAL( textChanged(const QString&) ), this, SLOT( fillStates(const QString&) ) );
    connect( okBtn, SIGNAL( clicked() ), this, SLOT( createAnnot() ) );
    connect( okBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( stateModel, SIGNAL( activated(const QString&) ), this, SLOT( fillStates(const QString&) ) );

    // buddies
    textLabel3->setBuddy( iconName );
    textLabel4_2->setBuddy( state );
    textLabel5_2->setBuddy( stateModel );
    textLabel2_3->setBuddy( linkContent );
    textLabel1->setBuddy( destination );
    textLabel2_2->setBuddy( highLight );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AnnotDialog::~AnnotDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AnnotDialog::languageChange()
{
    setCaption(tr("Add new annotation"));
    okBtn->setText(QObject::tr("&Ok"));
    okBtn->setAccel(QKeySequence("Alt+O"));
    cancelButton->setText(QObject::tr("&Cancel"));
    cancelButton->setAccel(QKeySequence("Alt+C"));
    groupBox1->setTitle(tr("Text Annotation"));
    open->setText(tr("&Open"));
    open->setAccel(QKeySequence("Alt+O"));
    textLabel2->setText(tr("Content"));
    textLabel3->setText(tr("Icon &Name"));
    iconName->clear();
    iconName->insertItem(QString::null);
    iconName->insertItem(tr("Comment"));
    iconName->insertItem(tr("Help"));
    iconName->insertItem(tr("Insert"));
    iconName->insertItem(tr("Key"));
    iconName->insertItem(tr("NewParagraph"));
    iconName->insertItem(tr("Note"));
    iconName->insertItem(tr("Paragraph"));
    iconName->setCurrentItem(6);
    textLabel4_2->setText(tr("&State"));
    state->clear();
    state->insertItem(tr("Marked"));
    state->insertItem(tr("Unmarked"));
    textLabel5_2->setText(tr("State &Model"));
    stateModel->clear();
    stateModel->insertItem(tr("Marked"));
    stateModel->insertItem(tr("Review"));
    tabWidget3->changeTab(tab, tr("&Text Annotation"));
    textLabel2_3->setText(tr("Content"));
    textLabel1->setText(tr("&Destination"));
    textLabel2_2->setText(tr("&Highlight mode"));
    highLight->clear();
    highLight->insertItem(tr("N - None"));
    highLight->insertItem(tr("I - Invert"));
    highLight->insertItem(tr("O - Outline"));
    highLight->insertItem(tr("P - Push"));
    tabWidget3->changeTab(tab_2, tr("&Link Annotation"));
}

void AnnotDialog::fillStates(const QString & current)
{
        state->clear();
        if(current == "Marked")
        {
                state->insertItem("Marked");
                state->insertItem("Unmarked");
        }else
                if(current == "Review")
                {
                        state->insertItem("Accepted");
                        state->insertItem("Rejected");
                        state->insertItem("Cancelled");
                        state->insertItem("Completed");
                        state->insertItem("None");
                }
}


void AnnotDialog::createTextAnnot()
{
using namespace pdfobjects::utils;
using namespace std;
using namespace boost;

        // backs up default values
        bool dOpen=TextAnnotInitializer::OPEN;
        string dContents=TextAnnotInitializer::CONTENTS;
        string dName=TextAnnotInitializer::NAME;
        string dState=TextAnnotInitializer::STATE;
        string dStateMode=TextAnnotInitializer::STATEMODEL;
        int dFlags=TextAnnotInitializer::FLAGS;

        // sets default values according form values
        TextAnnotInitializer::OPEN=open->isChecked();
        // FIXME this may be problem with encoding
        TextAnnotInitializer::CONTENTS=textContent->text().ascii();
        TextAnnotInitializer::NAME=iconName->currentText().ascii();
        TextAnnotInitializer::STATE=state->currentText().ascii();
        TextAnnotInitializer::STATEMODEL=stateModel->currentText().ascii();
        // TODO support flags field
        //TextAnnotInitializer::FLAGS=dFlags;

        // creates CAnnotation instance with factory, this should
        // use TextAnnotInitializer with values which we have
        // set above
        shared_ptr<CAnnotation> annotInstance=CAnnotation::createAnnotation(rect, "Text");
        page.get()->addAnnotation(annotInstance);


        // sets back default values
        TextAnnotInitializer::OPEN=dOpen;
        TextAnnotInitializer::CONTENTS=dContents;
        TextAnnotInitializer::NAME=dName;
        TextAnnotInitializer::STATE=dState;
        TextAnnotInitializer::STATEMODEL=dStateMode;
        TextAnnotInitializer::FLAGS=dFlags;
}

void AnnotDialog::createLinkAnnot()
{
using namespace pdfobjects::utils;
using namespace std;
using namespace boost;

        // backs up default values
        string dContents=LinkAnnotInitializer::CONTENTS;
        string dDest=LinkAnnotInitializer::DEST;
        string dH=LinkAnnotInitializer::H;

        // sets default values according form values
        // FIXME this may be problem with encoding
        LinkAnnotInitializer::CONTENTS=linkContent->text().ascii();
        LinkAnnotInitializer::DEST=destination->text().ascii();
        LinkAnnotInitializer::H=highLight->currentText().ascii();

        // creates CAnnotation instance with factory, this should
        // use LinkAnnotInitializer with values which we have
        // set above
        shared_ptr<CAnnotation> annotInstance=CAnnotation::createAnnotation(rect, "Link");
        page.get()->addAnnotation(annotInstance);

        // sets back default values
        LinkAnnotInitializer::CONTENTS=dContents;
        LinkAnnotInitializer::DEST=dDest;
        LinkAnnotInitializer::H=dH;
}

void AnnotDialog::createAnnot()
{
// checks currently set tab and creates annotation instance according
// its setting
// finally creates QSAnnotation and registers it to the page
        int current=tabWidget3->currentPageIndex();
        switch(current)
        {
                // Text annotation
                case 0:
                        createTextAnnot();
                        break;
                // Link annotation
                case 1:
                        createLinkAnnot();
                        break;
                // Annotation is not supported at this moment
                default:
                        break;
        }
}


void AnnotDialog::setRectangle(double xl, double yl, double width, double height)
{
        this->rect.xleft=xl;
        this->rect.yleft=yl;
        this->rect.xright=xl+width;
        this->rect.yright=yl+height;
}

} //namespace gui
