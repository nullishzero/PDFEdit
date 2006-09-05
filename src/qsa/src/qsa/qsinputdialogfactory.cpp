/****************************************************************************
** $Id$
**
** Copyright (C) 2001-2006 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Script for Applications framework (QSA).
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding a valid Qt Script for Applications license may use
** this file in accordance with the Qt Script for Applications License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about QSA Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
*****************************************************************************/

#include "qsinputdialogfactory.h"
#include "qsinterpreter.h"
#include <qapplication.h>
#include <qwidget.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qhbox.h>
#include <qdialog.h>
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <qtabwidget.h>
#include <qbuttongroup.h>
#include <qdatetimeedit.h>
#include <qvalidator.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
#include <qthread.h>
extern Q_EXPORT Qt::HANDLE qt_get_application_thread_id();
#endif


/*!
  \class QSInputDialogFactory qsinputdialogfactory.h

  \brief The QSInputDialogFactory class extends QSA with a basic and
  easy-to-use input dialog framework.

  To extend the interpreter with the Input Dialog Framework you need
  to create an instance of the QSInputDialogFactory and add it to the
  interpreter using the function
  QSInterpreter::addObjectFactory(). For example:

  \code
  QSProject proj;
  QSInterpreter *ip = proj->interpreter();
  ip->addObjectFactory( new QSInputDialogFactory );
  \endcode

  The script extensions provided by the QSInputDialogFramework are
  described in detail in \link qsa-6.book Input Dialog Framework
  \endlink

  Note on threading; If the Input Dialog Factory is used with an
  interpreter running in the non GUI thread, all functionallity is
  disabled.
*/


/*!
  \fn QSInputDialogFactory::QSInputDialogFactory()

  Creates the input dialog factory. Use the function
  QSInterpreter::addObjectFactory to make the objects this factory
  represents to an interpreter.
*/


/*!
  \fn QObject *QSInputDialogFactory::create( const QString &,
  const QSArgumentList &, QObject * )
  \internal
*/


class QSInput : public QObject
{
    Q_OBJECT
    Q_OVERRIDE(QCString name SCRIPTABLE false )
public:
    QSInput():QObject(0,"Input"){};
public slots:
    QVariant getText(const QString &label = QString::null,
		     const QString &text = QString::null,
		     const QString &title = QString::null,
		     QWidget *parent = 0);
    QVariant getNumber(const QString &label = QString::null,
		       double value = 0,
		       int decimals = 0,
		       double minValue = -2147483647,
		       double maxValue = 2147483647,
		       const QString &title = QString::null,
		       QWidget *parent = 0);
    QVariant getItem(const QString &label,
		     const QStringList &itemList,
		     const QString &currentItem = QString::null,
		     bool editable = FALSE,
		     const QString &title = QString::null,
		     QWidget *parent = 0);
    QVariant getItem( const QStringList &itemList,
		      const QString &currentItem = QString::null,
		      bool editable = FALSE,
		      QWidget *parent = 0) {
	return getItem( QString::null, itemList, currentItem, editable, QString::null, parent );
    }
};

QVariant QSInput::getText(const QString &label, const QString &text, const QString &title,
			  QWidget *parent )
{
    bool ok;
    QString lab = label;
    if (lab.isEmpty())
	lab = tr("Text:");
    QString res = QInputDialog::getText(title, lab, QLineEdit::Normal, text, &ok,
					parent ? parent : qApp->mainWidget());
    return ( ok ? QVariant( res ) : QVariant() );
}

QVariant QSInput::getNumber(const QString &label, double value,
			    int decimals, double minValue, double maxValue,
			    const QString &title, QWidget *parent )
{
    QString lab = label;
    if (lab.isEmpty())
	lab = tr("Number:");
    bool ok;
    if (decimals > 0) {
	double d = QInputDialog::getDouble( title, lab, value,
					    minValue, maxValue, decimals, &ok,
					    parent ? parent : qApp->mainWidget());
	return ( ok ? QVariant( d ) : QVariant() );
    }
    int i = QInputDialog::getInteger( title, lab, int( value ),
				      int( minValue ), int( maxValue ),
				      1, &ok,
				      parent ? parent : qApp->mainWidget());
    return ( ok ? QVariant( i ) : QVariant() );
}


QVariant QSInput::getItem(const QString &label,
			  const QStringList &itemList,
			  const QString &currentItem,
			  bool editable,
			  const QString &title,
			  QWidget *parent )
{
    QString lab = label;
    if (lab.isEmpty())
	lab = tr("Item:");
    int idx = 0;
    if ( !currentItem.isEmpty() )
	idx = itemList.findIndex(currentItem);
    bool ok;
    QString res = QInputDialog::getItem(title, lab, itemList, QMAX(idx,0), editable, &ok,
					parent ? parent : qApp->mainWidget());
    return ( ok ?  QVariant( res ) : QVariant() );
}


class QSFileDialog : public QObject
{
    Q_OBJECT
    Q_OVERRIDE( QCString name SCRIPTABLE false )

public:
    QSFileDialog() : QObject( 0, "FileDialog" ) { }

public slots:
    QVariant getOpenFileName( const QString &filter = QString::null,
			      const QString &title = QString::null,
			      QWidget *parent = 0);
    QVariant getSaveFileName( const QString &filter = QString::null,
			      const QString &title = QString::null,
			      QWidget *parent = 0 );
    QVariant getExistingDirectory( const QString &dir = QString::null,
				   const QString &title = QString::null,
				   QWidget *parent = 0 );
    QStringList getOpenFileNames( const QString &dir = QString::null,
				  const QString &filter = QString::null,
				  const QString &title = QString::null,
				  QWidget *parent = 0 );
private:
    QString cap;
};


QVariant QSFileDialog::getOpenFileName( const QString &filter, const QString &title,
					QWidget *parent )
{
    QString str = QFileDialog::getOpenFileName( QString::null, filter,
						parent ? parent : qApp->mainWidget(),
						0, title );
    return str.isNull() ? QVariant() : QVariant( str );
}

QVariant QSFileDialog::getSaveFileName( const QString &filter, const QString &title,
					QWidget *parent )
{
    QString str = QFileDialog::getSaveFileName( QString::null, filter,
						parent ? parent : qApp->mainWidget(),
						0, title );
    return str.isNull() ? QVariant() : QVariant( str );
}

QVariant QSFileDialog::getExistingDirectory( const QString &dir, const QString &title,
					     QWidget *parent )
{
    QString str =  QFileDialog::getExistingDirectory( dir, parent ? parent : qApp->mainWidget(),
						      0, title );
    return str.isNull() ? QVariant() : QVariant( str );
}

QStringList QSFileDialog::getOpenFileNames( const QString &dir,
					    const QString &filter,
					    const QString &title,
					    QWidget *parent )
{
    return QFileDialog::getOpenFileNames( filter, dir, parent ? parent : qApp->mainWidget(),
					  0, title );
}


class QSMessageBox : public QObject
{
    Q_OBJECT
    Q_OVERRIDE( QCString name SCRIPTABLE false )
    Q_ENUMS( ButtonType )
public:
    enum ButtonType { NoButton, Ok, Cancel, Yes, No, Abort,
		      Retry, Ignore };

    QSMessageBox() : QObject( 0, "MessageBox" ) { }

public slots:
    int information( const QString &text,
		     ButtonType button0 = Ok,
		     ButtonType button1 = NoButton,
		     ButtonType button2 = NoButton,
		     const QString &title = QString::null,
		     QWidget *parent = 0);
// QMessageBox::question is not part of Qt 3.1
/*
    QSMessageBox::ButtonType question( const QString &text,
				       ButtonType button0 = Yes,
				       ButtonType button1 = No,
				       ButtonType button2 = NoButton );
*/
    int warning( const QString &text,
		 ButtonType button0 = Retry,
		 ButtonType button1 = Abort,
		 ButtonType button2 = NoButton,
		 const QString &title = QString::null,
		 QWidget *parent = 0);

    int critical( const QString &text,
		  ButtonType button0 = Retry,
		  ButtonType button1 = NoButton,
		  ButtonType button2 = NoButton,
		  const QString &title = QString::null,
		  QWidget *parent = 0);

private:
    QString cap;
};


int QSMessageBox::information( const QString &text,
			       ButtonType button0,
			       ButtonType button1,
			       ButtonType button2,
			       const QString &title,
			       QWidget *parent )
{
    return QMessageBox::information( parent ? parent : qApp->mainWidget(),
				     title, text, button0, button1, button2 );
}
/*
QSMessageBox::ButtonType QSMessageBox::question( const QString &text,
						 ButtonType button0,
						 ButtonType button1,
						 ButtonType button2 )
{
    return (QSMessageBox::ButtonType) QMessageBox::question( qApp->mainWidget(),
							     cap, text,
							     button0, button1, button2 );
}
*/
int QSMessageBox::warning( const QString &text,
			   ButtonType button0,
			   ButtonType button1,
			   ButtonType button2,
			   const QString &title,
			   QWidget *parent)
{
    return QMessageBox::warning( parent ? parent : qApp->mainWidget(), title, text, button0, button1, button2 );
}

int QSMessageBox::critical( const QString &text,
			    ButtonType button0,
			    ButtonType button1,
			    ButtonType button2,
			    const QString &title,
			    QWidget *parent)
{
    return QMessageBox::critical( parent ? parent : qApp->mainWidget(), title, text, button0, button1, button2 );
}

class QSWidget : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString whatsThis READ whatsThis WRITE setWhatsThis)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)
    Q_OVERRIDE(QCString name SCRIPTABLE false )
public:
    QSWidget(QWidget *w = 0) : widget(w) {}
    QWidget *widget;
    QString whatsThis() const;
    void setWhatsThis(const QString &text) const;
    QString toolTip() const;
    void setToolTip(const QString &text) const;
};


QString QSWidget::whatsThis() const
{
    return QWhatsThis::textFor(widget);
}
void QSWidget::setWhatsThis(const QString &text) const
{
    QWhatsThis::add(widget, text);
}

QString QSWidget::toolTip() const
{
    return QToolTip::textFor(widget);
}
void QSWidget::setToolTip(const QString &text) const
{
    QToolTip::add(widget, text);
}

#define QS_WIDGET( Class ) \
 inline Q##Class *d() const { return (Q##Class*)widget; }

class QSLabel : public QSWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    QS_WIDGET( Label )
public:
    QSLabel() : QSWidget(new QLabel(0)) {}
    QString text() const
    { return d()->text(); }
    void setText( const QString &txt )
    { d()->setText(txt); }
};

class QSLabeled : public QSWidget
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel)
public:
    QSLabeled(const QString& label, QWidget *w );
    inline QString label() const { return labelWidget->text(); }
    void setLabel(const QString &txt);
    QLabel *labelWidget;
};

void QSLabeled::setLabel(const QString &txt)
{
    labelWidget->setText(txt);
    if( txt.isEmpty() )
	labelWidget->hide();
}

QSLabeled::QSLabeled(const QString& label, QWidget *w )
    : QSWidget(w), labelWidget( new QLabel(label,0) )
{
    labelWidget->setBuddy(w);
}

class QSLineEdit : public QSLabeled
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    QS_WIDGET( LineEdit )
public:
    QSLineEdit() : QSLabeled( QSInput::tr("Text:"),  new QLineEdit(0)) {}
    QString text() const
    { return d()->text(); }
    void setText( const QString &txt )
    { d()->setText(txt); }
};

class QSNumberEdit : public QSLabeled
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue)
    Q_PROPERTY(double decimals READ decimals WRITE setDecimals)
    Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
    QS_WIDGET( LineEdit )
public:
    QSNumberEdit() : QSLabeled( QSInput::tr("Value:"), new QLineEdit(0)), validator(new QDoubleValidator(widget))
    {
	d()->setValidator(validator);
	validator->setDecimals(0);
    }
    double value() const
    { return d()->text().toDouble(); }
    void setValue(double val)
    { d()->setText(QString::number(val)); }
    double decimals() const
    { return validator->decimals(); }
    void setDecimals(double decimals)
    { validator->setDecimals( int( decimals ) ); }
    double minimum() const
    { return validator->bottom(); }
    void setMinimum(double minimum)
    { validator->setBottom(minimum); }
    double maximum() const
    { return validator->top(); }
    void setMaximum(double maximum)
    { validator->setBottom(maximum); }
private:
    QDoubleValidator *validator;
};


class QSDateEditEnums : public QObject
{
    Q_OBJECT
    Q_ENUMS( Order )
    Q_OVERRIDE( QCString name SCRIPTABLE false )
public:
    enum Order { DMY, MDY, YMD, YDM };
};


class QSDateEdit : public QSLabeled
{
    Q_OBJECT
    Q_ENUMS(Order)
    Q_PROPERTY(int order READ order WRITE setOrder)
    Q_PROPERTY(QDate date READ date WRITE setDate)
    Q_PROPERTY(QDate minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(QDate maximum READ maximum WRITE setMaximum)
    QS_WIDGET( DateEdit )
public:
    enum Order { DMY, MDY, YMD, YDM };
    QSDateEdit() : QSLabeled( tr("Date:"), new QDateEdit(0))
    { d()->setAutoAdvance(TRUE); }
    int order() const
    { return (Order)d()->order(); }
    void setOrder(int order)
    { d()->setOrder((QDateEdit::Order)order); }
    QDate date() const
    { return d()->date(); }
    void setDate(QDate date)
    { d()->setDate(date); }
    QDate minimum() const
    { return d()->minValue(); }
    void setMinimum(const QDate &minimum)
    { d()->setMaxValue(minimum); }
    QDate maximum() const
    { return d()->maxValue(); }
    void setMaximum(const QDate &maximum)
    { d()->setMaxValue(maximum); }
};

class QSTimeEdit : public QSLabeled
{
    Q_OBJECT
    Q_PROPERTY(bool showSeconds READ seconds WRITE setSeconds)
    Q_PROPERTY(bool showAMPM READ ampm WRITE setAmpm)
    Q_PROPERTY(QTime time READ time WRITE setTime)
    Q_PROPERTY(QTime minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(QTime maximum READ maximum WRITE setMaximum)
    QS_WIDGET( TimeEdit )
public:
    QSTimeEdit() : QSLabeled( tr("Time:"), new QTimeEdit(0))
    { d()->setAutoAdvance(TRUE); }
    bool seconds() const
    { return d()->display() & QTimeEdit::Seconds; }
    void setSeconds(bool b)
    {
	uint disp = d()->display();
	d()->setDisplay( b ? (disp|QTimeEdit::Seconds):(disp&~QTimeEdit::Seconds));
    }
    bool ampm() const
    { return d()->display() & QTimeEdit::AMPM; }
    void setAmpm(bool b)
    {
	uint disp = d()->display();
	d()->setDisplay( b ? (disp|QTimeEdit::AMPM):(disp&~QTimeEdit::AMPM));
    }
    QTime time() const
    { return d()->time(); }
    void setTime(QTime time)
    { d()->setTime(time); }
    QTime minimum() const
    { return d()->minValue(); }
    void setMinimum(const QTime &minimum)
    { d()->setMaxValue(minimum); }
    QTime maximum() const
    { return d()->maxValue(); }
    void setMaximum(const QTime &maximum)
    { d()->setMaxValue(maximum); }
};

class QSTextEdit : public QSWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    QS_WIDGET( TextEdit )
public:
    QSTextEdit() : QSWidget(new QTextEdit(0))
    { d()->setTextFormat(QTextEdit::PlainText); }
    QString text() const
    { return d()->text(); }
    void setText( const QString &txt )
    { d()->setText(txt); }
};

class QSSpinBox : public QSLabeled
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    QS_WIDGET( SpinBox )
public:
    QSSpinBox() : QSLabeled( QSInput::tr("Value:"), new QSpinBox(0)) {}
    int value() const
    { return d()->value(); }
    void setValue( int val )
    { d()->setValue( val ); }
    int minimum() const
    { return d()->value(); }
    void setMinimum(int minimum)
    { d()->setMinValue(minimum); }
    int maximum() const
    { return d()->value(); }
    void setMaximum(int maximum)
    { d()->setMaxValue(maximum); }
};

class QSCheckBox : public QSWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    QS_WIDGET( CheckBox )
public:
    QSCheckBox() : QSWidget(new QCheckBox(0)) {}
    QString text() const
    { return d()->text(); }
    void setText( const QString &txt )
    { d()->setText(txt); }

    bool isChecked() const
    { return d()->isChecked(); }
    void setChecked(bool b)
    { d()->setChecked(b); }
};


class QSRadioButton : public QSWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    QS_WIDGET( RadioButton )
public:
    QSRadioButton() : QSWidget(new QRadioButton(0)) {}
    QString text() const
    { return d()->text(); }
    void setText( const QString &txt )
    { d()->setText(txt); }

    bool isChecked() const
    { return d()->isChecked(); }
    void setChecked(bool b)
    { d()->setChecked(b); }
};

class QSComboBox : public QSLabeled
{
    Q_OBJECT
    Q_PROPERTY(QStringList itemList READ itemList WRITE setItemList)
    Q_PROPERTY(QString currentItem READ currentItem WRITE setCurrentItem)
    Q_PROPERTY(bool editable READ isEditable WRITE setEditable)
    QS_WIDGET( ComboBox )
public:
    QSComboBox() : QSLabeled( QSInput::tr("Item:"), new QComboBox((bool)FALSE, 0)) {}
    QString currentItem() const
    { return d()->currentText(); }
    void setCurrentItem( const QString &txt )
    {
        for (int i=0; i<d()->count(); ++i) {
            if (d()->text(i) == txt) {
                d()->setCurrentItem(i);
                break;
            }
        }
    }
    bool isEditable() const
    { return d()->editable(); }
    void setEditable(bool b)
    { d()->setEditable(b); }
    QStringList itemList() const;
    void setItemList( const QStringList &l )
    {
	d()->clear();
	d()->insertStringList( l );
    }
};

QStringList QSComboBox::itemList() const
{
    QStringList l;
    for (int i = 0; i < d()->count(); ++i)
	l << d()->text(i);
    return l;
}


class QSGroupBox : public QSWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    QS_WIDGET(GroupBox)
public:
    QSGroupBox();
    QString title() const
    { return d()->title(); }
    void setTitle( const QString &title )
    { d()->setTitle(title); }
public slots:
    void add(QSWidget *widget);
    void newColumn();
    void addSpace(int space);
private:
    QButtonGroup *invisibleButtonGroup;
    QHBoxLayout *hbox;
    QGridLayout *grid;
};

QSGroupBox::QSGroupBox()
    : QSWidget( new QGroupBox ), invisibleButtonGroup(0)
{
    d()->setColumnLayout(0, Qt::Horizontal);
    hbox = new QHBoxLayout(d()->layout() );
    grid = new QGridLayout(hbox, 0, 0, 5);
    grid->setAlignment( AlignTop );
}

void QSGroupBox::add(QSWidget *widget)
{
    QWidget *w = widget->widget;
    if (w->parentWidget() != d() )
	w->reparent(d(), QPoint(), TRUE);
    int row = grid->numRows();
    if ( widget->inherits("QSLabeled") ) {
	QLabel *label = ((QSLabeled*)widget)->labelWidget;
	if (label->parentWidget() != d())
	    label->reparent(d(), QPoint() );
	label->setShown( !label->text().isEmpty() );
 	grid->addWidget(label, row, 0);
 	grid->addWidget(w, row, 1);
    } else {
	grid->addMultiCellWidget(w, row, row, 0, 1);
    }

    if ( w && w->isA("QRadioButton") ) {
	if (!invisibleButtonGroup )
	    (invisibleButtonGroup = new QButtonGroup(d()))->hide();
	invisibleButtonGroup->insert((QButton*)w);
    }

}

void QSGroupBox::addSpace(int space)
{
    QSpacerItem *spacer = new QSpacerItem(0, QMAX(space,0), QSizePolicy::Fixed, QSizePolicy::Fixed);
    grid->addItem(spacer, grid->numRows(), 1);
    invisibleButtonGroup = 0;
}

void QSGroupBox::newColumn() {
    if (grid->numRows()) {
	hbox->addSpacing(17);
	grid = new QGridLayout(hbox, 0, 0, 5);
	grid->setAlignment( AlignTop );
    }
}

class QSDialogPrivate;
class QSDialog : public QSWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(int width READ width WRITE setWidth)
    Q_PROPERTY(QString okButtonText READ okButtonText WRITE setOkButtonText)
    Q_PROPERTY(QString cancelButtonText READ cancelButtonText WRITE setCancelButtonText)
    QSDialogPrivate *d;
public:
    QSDialog(const QString &caption = QString::null, QWidget *parent=0);
    ~QSDialog();

    void setTitle(const QString &title);
    QString title() const;

    void setWidth(int width);
    int width() const;

    void setOkButtonText(const QString &text);
    QString okButtonText() const;

    void setCancelButtonText(const QString &text);
    QString cancelButtonText() const;

public slots:
    void newTab(const QString &label);
    void newColumn();
    void addSpace(int space);
    void add(QSWidget *widget);
    bool exec();

};

class QSDialogPrivate : public QDialog
{
public:
    QSDialogPrivate(QWidget *parent=0);
    int width;
    QSize sizeHint() const
    { return QDialog::sizeHint().expandedTo( QSize(width, 0) ); }
    QWidget *lastWidget;

    QHBoxLayout *hbox;
    QGridLayout *grid;

    QButtonGroup *invisibleButtonGroup;
    QTabWidget *tabWidget;
    QGroupBox *groupBox;

    QHBoxLayout *buttonRow;
    void fixTabOrder();

    QPushButton *okButton, *cancelButton;
    QWidget *parent;

    void addSpace( int space);
    void add(QSWidget *widget);
};


QSDialogPrivate::QSDialogPrivate(QWidget *parent)
    : QDialog(parent),
      width(0), lastWidget(0),
     invisibleButtonGroup(0), tabWidget(0), groupBox(0)
{
    this->parent = this;
    QLayout *vbox = new QVBoxLayout(this, 11, 7);
    hbox = new QHBoxLayout(vbox);
    grid = new QGridLayout(hbox, 0, 0);
    grid->setAlignment( AlignTop );
    okButton = new QPushButton(QMessageBox::tr("OK"), this);
    okButton->setDefault(TRUE);
    QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    cancelButton = new QPushButton(QMessageBox::tr("Cancel"), this);
    QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonRow = new QHBoxLayout(vbox);
    buttonRow->addStretch(10);
    buttonRow->addWidget(okButton);
    buttonRow->addWidget(cancelButton);
}

void QSDialogPrivate::fixTabOrder()
{
    if (lastWidget) {
	if ( lastWidget ) {
	    QWidget::setTabOrder(lastWidget, okButton);
	    QWidget::setTabOrder(okButton, cancelButton);
	}
	layout()->activate();
	lastWidget = 0;
    }
}

void QSDialogPrivate::addSpace(int space)
{
    QSpacerItem *spacer = new QSpacerItem(0, QMAX(space,0), QSizePolicy::Fixed, QSizePolicy::Fixed);
    grid->addItem(spacer, grid->numRows(), 1);
    invisibleButtonGroup = 0;
}

void QSDialogPrivate::add(QSWidget *widget)
{
    QWidget *w = widget->widget;

    if (w->parentWidget() != parent)
	w->reparent(parent, QPoint(), TRUE);

    int row = grid->numRows();
    if ( widget->inherits("QSLabeled") ) {
	QLabel *label = ((QSLabeled*)widget)->labelWidget;
	if (label->parentWidget() != parent)
	    label->reparent(parent, QPoint() );
	label->setShown( !label->text().isEmpty() );
 	grid->addWidget(label, row, 0);
 	grid->addWidget(w, row, 1);
    } else {
	grid->addMultiCellWidget(w, row, row, 0, 1);
    }

    if ( w && w->isA("QRadioButton") ) {
	if (!invisibleButtonGroup )
	    (invisibleButtonGroup = new QButtonGroup(this))->hide();
	invisibleButtonGroup->insert((QButton*)w);
    }

    lastWidget = w;
}

QSDialog::QSDialog(const QString &title, QWidget *parent)
{
    d = new QSDialogPrivate(parent);
    widget = d;
    if (!title.isEmpty())
	setTitle(title);
}

QSDialog::~QSDialog()
{
    delete d;
}

void QSDialog::setTitle(const QString &title) { d->setCaption(title); }
QString QSDialog::title() const { return d->caption(); }
void QSDialog::setWidth(int width){ d->width = width; }
int QSDialog::width() const { return d->width; }
void QSDialog::setOkButtonText( const QString &text ) { d->okButton->setText(text); }
QString QSDialog::okButtonText() const { return d->okButton->text(); }
void QSDialog::setCancelButtonText( const QString &text ) { d->cancelButton->setText(text); }
QString QSDialog::cancelButtonText() const { return d->cancelButton->text(); }

void QSDialog::newTab(const QString &label)
{
    if ( !d->tabWidget ) {
	d->tabWidget = new QTabWidget(d);
	int row = d->grid->numRows();
	d->grid->addMultiCellWidget(d->tabWidget, row, row, 0, 1);
    }
    QWidget *w = new QWidget;
    d->tabWidget->addTab(w, label);
    d->parent = w;
    d->hbox = new QHBoxLayout(w, 11, 7);
    d->grid = new QGridLayout(d->hbox, 0, 0);
    d->grid->setAlignment(AlignTop);
}

void QSDialog::newColumn()
{
    if (d->grid->numRows()) {
	d->hbox->addSpacing(17);
	d->grid = new QGridLayout(d->hbox, 0, 0);
	d->grid->setAlignment(AlignTop);
    }
}

void QSDialog::addSpace(int space)
{
    d->addSpace(space);
}

void QSDialog::add( QSWidget *widget )
{
    d->add(widget);
}

bool QSDialog::exec()
{
    d->fixTabOrder();
    int res = d->exec();
    return (res == QDialog::Accepted);
}


QSInputDialogFactory::QSInputDialogFactory()
{
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    if (qt_get_application_thread_id() != QThread::currentThread()) {
	qWarning("QSInputDialogfactory::QSInputDialogFactory(), "
		 "factory cannot be used in non GUI thread");
	return;
    }
#endif
    registerClass( QString::fromLatin1("Dialog"), QString::fromLatin1("QSDialog") );
    registerClass( QString::fromLatin1("Label"), QString::fromLatin1("QSLabel") );
    registerClass( QString::fromLatin1("LineEdit"), QString::fromLatin1("QSLineEdit") );
    registerClass( QString::fromLatin1("NumberEdit"), QString::fromLatin1("QSNumberEdit") );
    registerClass( QString::fromLatin1("DateEdit"),
                   QString::fromLatin1("QSDateEdit"), new QSDateEditEnums );
    registerClass( QString::fromLatin1("TimeEdit"), QString::fromLatin1("QSTimeEdit") );
    registerClass( QString::fromLatin1("TextEdit"), QString::fromLatin1("QSTextEdit") );
    registerClass( QString::fromLatin1("SpinBox"), QString::fromLatin1("QSSpinBox") );
    registerClass( QString::fromLatin1("CheckBox"), QString::fromLatin1("QSCheckBox") );
    registerClass( QString::fromLatin1("RadioButton"), QString::fromLatin1("QSRadioButton") );
    registerClass( QString::fromLatin1("ComboBox"), QString::fromLatin1("QSComboBox") );
    registerClass( QString::fromLatin1("GroupBox"), QString::fromLatin1("QSGroupBox") );
    registerClass( QString::fromLatin1("MessageBox"), QString::null, new QSMessageBox() );
    registerClass( QString::fromLatin1("FileDialog"), QString::null, new QSFileDialog() );
    registerClass( QString::fromLatin1("Input"), QString::null, new QSInput() );
}

QObject *QSInputDialogFactory::create( const QString &className,
				       const QSArgumentList &arguments,
				       QObject * /*context*/ )
{
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    if (qt_get_application_thread_id() != QThread::currentThread()) {
	qWarning("QSInputDialogfactory::create(), "
		 "factory cannot be used in non GUI thread");
	return 0;
    }
#endif
    if (className == QString::fromLatin1("Dialog")) {
	QString title = arguments.size() >= 1 ? arguments[0].variant().toString() : QString::null;
	QObject *obj = arguments.size() >= 2 ? arguments[1].qobject() : 0;
	QWidget *widget = obj && obj->inherits("QWidget") ? (QWidget*) obj : 0;
	return new QSDialog(title, widget);
    } else if ( className == QString::fromLatin1("Label") ) {
	return new QSLabel;
    } else if ( className == QString::fromLatin1("LineEdit") ) {
	return new QSLineEdit;
    } else if ( className == QString::fromLatin1("NumberEdit") ) {
	return new QSNumberEdit;
    } else if ( className == QString::fromLatin1("DateEdit") ) {
	return new QSDateEdit;
    } else if ( className == QString::fromLatin1("TimeEdit") ) {
	return new QSTimeEdit;
    } else if ( className == QString::fromLatin1("TextEdit") ) {
	return new QSTextEdit;
    } else if ( className == QString::fromLatin1("SpinBox") ) {
	return new QSSpinBox;
    } else if ( className == QString::fromLatin1("CheckBox") ) {
	return new QSCheckBox;
    } else if ( className == QString::fromLatin1("RadioButton") ) {
	return new QSRadioButton;
    } else if ( className == QString::fromLatin1("ComboBox") ) {
	return new QSComboBox;
    } else if ( className == QString::fromLatin1("GroupBox") ) {
	return new QSGroupBox;
    }
    return 0;
}

#include "qsinputdialogfactory.moc"
