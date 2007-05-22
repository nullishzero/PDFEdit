/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech AS. All rights reserved.
**
** This file is part of the QSA of the Qt Toolkit.
**
** For QSA Commercial License Holders (non-open source):
** 
** Licensees holding a valid Qt Script for Applications (QSA) License Agreement
** may use this file in accordance with the rights, responsibilities and
** obligations contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of the Licensing Agreement are
** not clear to you.
** 
** Further information about QSA licensing is available at:
** http://www.trolltech.com/products/qsa/licensing.html or by contacting
** info@trolltech.com.
** 
** 
** For Open Source Edition:  
** 
** This file may be used under the terms of the GNU General Public License
** version 2 as published by the Free Software Foundation and appearing in the
** file LICENSE.GPL included in the packaging of this file.  Please review the
** following information to ensure GNU General Public Licensing requirements
** will be met:  http://www.trolltech.com/products/qt/opensource.html 
** 
** If you are unsure which license is appropriate for your use, please review
** the following information:
** http://www.trolltech.com/products/qsa/licensing.html or contact the 
** sales department at sales@trolltech.com.

**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsinputdialogfactory.h"
#include "qsinterpreter.h"
#include <qapplication.h>
#include <qwidget.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qdialog.h>
#include <qinputdialog.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qvalidator.h>
#include <qtooltip.h>

#include <QDateEdit>
#include <QFileDialog>
#include <QTextEdit>
#include <QGroupBox>
#include <QButtonGroup>

#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
extern bool qsa_is_non_gui_thread();


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
  ip->addObjectFactory(new QSInputDialogFactory);
  \endcode

  The script extensions provided by the QSInputDialogFramework are
  described in detail in \l {QSA Input Dialog Framework}.

  Note on threading; If the Input Dialog Factory is used with an
  interpreter running in the non GUI thread, all functionality is
  disabled.
*/


/*!
  \fn QSInputDialogFactory::QSInputDialogFactory()

  Creates the input dialog factory. Use the function
  QSInterpreter::addObjectFactory to make the objects this factory
  represents to an interpreter.
*/


/*!
  \fn QObject *QSInputDialogFactory::create(const QString &,
  const QVariantList &, QObject *)
  \internal
*/


class QSInput : public QObject
{
    Q_OBJECT
    Q_OVERRIDE(QCString name SCRIPTABLE false)
public:
    QSInput() : QObject(0)
    {
        setObjectName(QLatin1String("Input"));
    }

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
		     bool editable = false,
		     const QString &title = QString::null,
		     QWidget *parent = 0);
    QVariant getItem(const QStringList &itemList,
		      const QString &currentItem = QString::null,
		      bool editable = false,
		      QWidget *parent = 0) {
	return getItem(QString::null, itemList, currentItem, editable, QString::null, parent);
    }
};

QVariant QSInput::getText(const QString &label, const QString &text, const QString &title,
			  QWidget *parent)
{
    bool ok;
    QString lab = label;
    if (lab.isEmpty())
	lab = tr("Text:");
    QString res = QInputDialog::getText(parent ? parent : qApp->activeWindow(), title,
                                        lab, QLineEdit::Normal, text, &ok);

    return (ok ? QVariant(res) : QVariant());
}

QVariant QSInput::getNumber(const QString &label, double value,
			    int decimals, double minValue, double maxValue,
			    const QString &title, QWidget *parent)
{
    QString lab = label;
    if (lab.isEmpty())
	lab = tr("Number:");
    bool ok;
    if (decimals > 0) {
	double d = QInputDialog::getDouble(parent ? parent : qApp->activeWindow(), title, lab,
                                       value, minValue, maxValue, decimals, &ok);
	return (ok ? QVariant(d) : QVariant());
    }
    int i = QInputDialog::getInteger(parent ? parent : qApp->activeWindow(), title, lab, int(value),
				      int(minValue), int(maxValue), 1, &ok);
    return (ok ? QVariant(i) : QVariant());
}


QVariant QSInput::getItem(const QString &label,
			  const QStringList &itemList,
			  const QString &currentItem,
			  bool editable,
			  const QString &title,
			  QWidget *parent)
{
    QString lab = label;
    if (lab.isEmpty())
	lab = tr("Item:");
    int idx = 0;
    if (!currentItem.isEmpty())
	idx = itemList.indexOf(currentItem);
    bool ok;
    QString res = QInputDialog::getItem(parent ? parent : qApp->activeWindow(), title,
                                        lab, itemList, qMax(idx,0), editable, &ok);
    return (ok ?  QVariant(res) : QVariant());
}


class QSFileDialog : public QObject
{
    Q_OBJECT
    Q_OVERRIDE(QCString name SCRIPTABLE false)

public:
    QSFileDialog() : QObject(0)
    {
        setObjectName(QLatin1String("FileDialog"));
    }

public slots:
    QVariant getOpenFileName(const QString &filter = QString::null,
			      const QString &title = QString::null,
			      QWidget *parent = 0);
    QVariant getSaveFileName(const QString &filter = QString::null,
			      const QString &title = QString::null,
			      QWidget *parent = 0);
    QVariant getExistingDirectory(const QString &dir = QString::null,
				   const QString &title = QString::null,
				   QWidget *parent = 0);
    QStringList getOpenFileNames(const QString &dir = QString::null,
				  const QString &filter = QString::null,
				  const QString &title = QString::null,
				  QWidget *parent = 0);
private:
    QString cap;
};


QVariant QSFileDialog::getOpenFileName(const QString &filter, const QString &title,
					QWidget *parent)
{
    QString str = QFileDialog::getOpenFileName(parent ? parent : qApp->activeWindow(), title, QString::null, filter,
						                       0);
    return str.isNull() ? QVariant() : QVariant(str);
}

QVariant QSFileDialog::getSaveFileName(const QString &filter, const QString &title,
					QWidget *parent)
{
    QString str = QFileDialog::getSaveFileName(parent ? parent : qApp->activeWindow(), title, QString::null, filter,
                                               0);
    return str.isNull() ? QVariant() : QVariant(str);
}

QVariant QSFileDialog::getExistingDirectory(const QString &dir, const QString &title,
					     QWidget *parent)
{
    QString str =  QFileDialog::getExistingDirectory(parent ? parent : qApp->activeWindow(), title, dir, 0);
    return str.isNull() ? QVariant() : QVariant(str);
}

QStringList QSFileDialog::getOpenFileNames(const QString &dir,
					    const QString &filter,
					    const QString &title,
					    QWidget *parent)
{
    return QFileDialog::getOpenFileNames(parent ? parent : qApp->activeWindow(), title, filter, dir, 0);
}


class QSMessageBox : public QObject
{
    Q_OBJECT
    Q_OVERRIDE(QCString name SCRIPTABLE false)
    Q_ENUMS(ButtonType)
public:
    enum ButtonType { NoButton, Ok, Cancel, Yes, No, Abort,
		      Retry, Ignore };

    QSMessageBox() : QObject(0)
    {
        setObjectName(QLatin1String("MessageBox"));
    }

public slots:
    int information(const QString &text,
		     ButtonType button0 = Ok,
		     ButtonType button1 = NoButton,
		     ButtonType button2 = NoButton,
		     const QString &title = QString::null,
		     QWidget *parent = 0);
// QMessageBox::question is not part of Qt 3.1
/*
    QSMessageBox::ButtonType question(const QString &text,
				       ButtonType button0 = Yes,
				       ButtonType button1 = No,
				       ButtonType button2 = NoButton);
*/
    int warning(const QString &text,
		 ButtonType button0 = Retry,
		 ButtonType button1 = Abort,
		 ButtonType button2 = NoButton,
		 const QString &title = QString::null,
		 QWidget *parent = 0);

    int critical(const QString &text,
		  ButtonType button0 = Retry,
		  ButtonType button1 = NoButton,
		  ButtonType button2 = NoButton,
		  const QString &title = QString(),
		  QWidget *parent = 0);

private:
    QString cap;
};


int QSMessageBox::information(const QString &text,
			       ButtonType button0,
			       ButtonType button1,
			       ButtonType button2,
			       const QString &title,
			       QWidget *parent)
{
    return QMessageBox::information(parent ? parent : qApp->activeWindow(),
				     title, text, button0, button1, button2);
}
/*
QSMessageBox::ButtonType QSMessageBox::question(const QString &text,
						 ButtonType button0,
						 ButtonType button1,
						 ButtonType button2)
{
    return (QSMessageBox::ButtonType) QMessageBox::question(qApp->activeWindow(),
							     cap, text,
							     button0, button1, button2);
}
*/
int QSMessageBox::warning(const QString &text,
			   ButtonType button0,
			   ButtonType button1,
			   ButtonType button2,
			   const QString &title,
			   QWidget *parent)
{
    return QMessageBox::warning(parent ? parent : qApp->activeWindow(), title, text, button0, button1, button2);
}

int QSMessageBox::critical(const QString &text,
			    ButtonType button0,
			    ButtonType button1,
			    ButtonType button2,
			    const QString &title,
			    QWidget *parent)
{
    return QMessageBox::critical(parent ? parent : qApp->activeWindow(), title, text, button0, button1, button2);
}

class QSWidget : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString whatsThis READ whatsThis WRITE setWhatsThis)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)
    Q_OVERRIDE(QCString name SCRIPTABLE false)
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
    return widget->whatsThis();
}

void QSWidget::setWhatsThis(const QString &text) const
{
    widget->setWhatsThis(text);
}

QString QSWidget::toolTip() const
{
    return widget->toolTip();
}

void QSWidget::setToolTip(const QString &text) const
{
    widget->setToolTip(text);
}

#define QS_WIDGET(Class) \
 inline Q##Class *d() const { return (Q##Class*)widget; }

class QSLabel : public QSWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    inline QLabel *d() const { return (QLabel *) widget; }
public:
    QSLabel() : QSWidget(new QLabel(0)) {}
    QString text() const
    { return d()->text(); }
    void setText(const QString &txt)
    { d()->setText(txt); }
};

class QSLabeled : public QSWidget
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel)
public:
    QSLabeled(const QString& label, QWidget *w);
    inline QString label() const { return labelWidget->text(); }
    void setLabel(const QString &txt);
    QLabel *labelWidget;
};

void QSLabeled::setLabel(const QString &txt)
{
    labelWidget->setText(txt);
    if(txt.isEmpty())
	labelWidget->hide();
}

QSLabeled::QSLabeled(const QString& label, QWidget *w)
    : QSWidget(w), labelWidget(new QLabel(label,0))
{
    labelWidget->setBuddy(w);
}

class QSLineEdit : public QSLabeled
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    QS_WIDGET(LineEdit)
public:
    QSLineEdit() : QSLabeled(QSInput::tr("Text:"),  new QLineEdit(0)) {}
    QString text() const
    { return d()->text(); }
    void setText(const QString &txt)
    { d()->setText(txt); }
};

class QSNumberEdit : public QSLabeled
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue)
    Q_PROPERTY(double decimals READ decimals WRITE setDecimals)
    Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
    QS_WIDGET(LineEdit)
public:
    QSNumberEdit() : QSLabeled(QSInput::tr("Value:"), new QLineEdit(0)), validator(new QDoubleValidator(widget))
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
    { validator->setDecimals(int(decimals)); }
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
    Q_ENUMS(Order)
    Q_OVERRIDE(QCString name SCRIPTABLE false)
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
    QS_WIDGET(DateEdit)
public:
    enum Order { DMY, MDY, YMD, YDM };
    QSDateEdit() : QSLabeled(tr("Date:"), new QDateEdit(0))
    { }

    int order() const
    {
        QString format = d()->displayFormat();
        if (format == "dd.MM.yyyy") return DMY;
        else if (format == "MM.dd.yyyy") return MDY;
        else if (format == "yyyy.MM.dd") return YMD;
        else if (format == "yyyy.dd.MM") return YDM;
        return -1;;
    }

    void setOrder(int order)
    {
        if (order == DMY) d()->setDisplayFormat("dd.MM.yyyy");
        else if (order == MDY) d()->setDisplayFormat("MM.dd.yyyy");
        else if (order == YMD) d()->setDisplayFormat("yyyy.MM.dd");
        else if (order == YDM) d()->setDisplayFormat("yyyy.dd.MM");
    }

    QDate date() const { return d()->date(); }
    void setDate(QDate date) { d()->setDate(date); }
    QDate minimum() const { return d()->minimumDate(); }
    void setMinimum(const QDate &minimum) { d()->setMinimumDate(minimum); }
    QDate maximum() const { return d()->maximumDate(); }
    void setMaximum(const QDate &maximum) { d()->setMaximumDate(maximum); }
};

class QSTimeEdit : public QSLabeled
{
    Q_OBJECT
    Q_PROPERTY(bool showSeconds READ seconds WRITE setSeconds)
    Q_PROPERTY(bool showAMPM READ ampm WRITE setAmpm)
    Q_PROPERTY(QTime time READ time WRITE setTime)
    Q_PROPERTY(QTime minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(QTime maximum READ maximum WRITE setMaximum)
    QS_WIDGET(TimeEdit)
public:
    QSTimeEdit() : QSLabeled(tr("Time:"), new QTimeEdit(0))
    { }

    void updateDisplayFormat(bool ampm, bool secs)
    {
        QString baseFormat = "HH::mm";
        if (ampm)
            baseFormat = "AP" + baseFormat;
        if (secs)
            baseFormat += ":ss";
        d()->setDisplayFormat(baseFormat);
    }

    bool seconds() const { return d()->displayFormat().contains("ss"); }
    void setSeconds(bool b) { updateDisplayFormat(ampm(), b); }

    bool ampm() const { return d()->displayFormat().toLower().contains("ap"); }
    void setAmpm(bool b) { updateDisplayFormat(b, seconds()); }

    QTime time() const { return d()->time(); }
    void setTime(QTime time) { d()->setTime(time); }

    QTime minimum() const { return d()->minimumTime(); }
    void setMinimum(const QTime &minimum) { d()->setMinimumTime(minimum); }

    QTime maximum() const { return d()->maximumTime(); }
    void setMaximum(const QTime &maximum) { d()->setMaximumTime(maximum); }
};

class QSTextEdit : public QSWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    QS_WIDGET(TextEdit)
public:
    QSTextEdit() : QSWidget(new QTextEdit(0))
    { }
    QString text() const
    { return d()->toPlainText(); }
    void setText(const QString &txt)
    { d()->setPlainText(txt); }
};

class QSSpinBox : public QSLabeled
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    QS_WIDGET(SpinBox)
public:
    QSSpinBox() : QSLabeled(QSInput::tr("Value:"), new QSpinBox(0)) {}
    int value() const
    { return d()->value(); }
    void setValue(int val)
    { d()->setValue(val); }
    int minimum() const
    { return d()->value(); }
    void setMinimum(int minimum)
    { d()->setMinimum(minimum); }
    int maximum() const
    { return d()->value(); }
    void setMaximum(int maximum)
    { d()->setMaximum(maximum); }
};

class QSCheckBox : public QSWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    QS_WIDGET(CheckBox)
public:
    QSCheckBox() : QSWidget(new QCheckBox(0)) {}
    QString text() const
    { return d()->text(); }
    void setText(const QString &txt)
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
    QS_WIDGET(RadioButton)
public:
    QSRadioButton() : QSWidget(new QRadioButton(0)) {}
    QString text() const
    { return d()->text(); }
    void setText(const QString &txt)
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
    QS_WIDGET(ComboBox)
public:
    QSComboBox() : QSLabeled(QSInput::tr("Item:"), new QComboBox(0))
    {
        setEditable(false);
    }
    QString currentItem() const
    { return d()->currentText(); }
    void setCurrentItem(const QString &txt)
    { 
        int idx = d()->findText(txt);
        if (idx >= 0)
            d()->setCurrentIndex(idx);
    }
    bool isEditable() const
    { return d()->isEditable(); }
    void setEditable(bool b)
    { d()->setEditable(b); }
    QStringList itemList() const;
    void setItemList(const QStringList &l)
    {
	d()->clear();
	d()->insertItems(0, l);
    }
};

QStringList QSComboBox::itemList() const
{
    QStringList l;
    for (int i = 0; i < d()->count(); ++i)
	l << d()->itemText(i);
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
    void setTitle(const QString &title)
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
    : QSWidget(new QGroupBox), invisibleButtonGroup(0)
{
    hbox = new QHBoxLayout(d());
    grid = new QGridLayout();

    hbox->addLayout(grid);
    grid->setAlignment(Qt::AlignTop);
}

void QSGroupBox::add(QSWidget *widget)
{
    QWidget *w = widget->widget;

    int row = grid->rowCount();
    if (qobject_cast<QSLabeled *>(widget)) {
	QLabel *label = ((QSLabeled*)widget)->labelWidget;
 	grid->addWidget(label, row, 0);
 	grid->addWidget(w, row, 1);
	label->setShown(!label->text().isEmpty());
    } else {
	grid->addWidget(w, row, 0, 1, 2);
    }

    if (w && qobject_cast<QRadioButton *>(w)) {
	if (!invisibleButtonGroup)
	    invisibleButtonGroup = new QButtonGroup(d());
	invisibleButtonGroup->addButton(qobject_cast<QRadioButton *>(w));
    }

}

void QSGroupBox::addSpace(int space)
{
    QSpacerItem *spacer = new QSpacerItem(0, qMax(space,0), QSizePolicy::Fixed, QSizePolicy::Fixed);
    grid->addItem(spacer, grid->rowCount(), 1);
    invisibleButtonGroup = 0;
}

void QSGroupBox::newColumn() {
    if (grid->rowCount()) {
	hbox->addSpacing(17);
	grid = new QGridLayout;
    grid->setAlignment(Qt::AlignTop);
    hbox->addLayout(grid);
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
    QSize sizeHint() const { return QDialog::sizeHint().expandedTo(QSize(width, 0)); }
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

    void addSpace(int space);
    void add(QSWidget *widget);
};


QSDialogPrivate::QSDialogPrivate(QWidget *parent)
    : QDialog(parent),
      width(0), lastWidget(0),
     invisibleButtonGroup(0), tabWidget(0), groupBox(0)
{
    this->parent = this;

    QVBoxLayout *vbox = new QVBoxLayout(this);
    setLayout(vbox);

    hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    grid = new QGridLayout;
    hbox->addLayout(grid);
    grid->setAlignment(Qt::AlignTop);

    okButton = new QPushButton(QMessageBox::tr("OK"), this);
    okButton->setDefault(true);
    QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    cancelButton = new QPushButton(QMessageBox::tr("Cancel"), this);
    QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    buttonRow = new QHBoxLayout;
    vbox->addLayout(buttonRow);
    buttonRow->addStretch(10);
    buttonRow->addWidget(okButton);
    buttonRow->addWidget(cancelButton);
}

void QSDialogPrivate::fixTabOrder()
{
    if (lastWidget) {
	if (lastWidget) {
	    QWidget::setTabOrder(lastWidget, okButton);
	    QWidget::setTabOrder(okButton, cancelButton);
	}
	layout()->activate();
	lastWidget = 0;
    }
}

void QSDialogPrivate::addSpace(int space)
{
    QSpacerItem *spacer = new QSpacerItem(0, qMax(space,0), QSizePolicy::Fixed, QSizePolicy::Fixed);
    grid->addItem(spacer, grid->rowCount(), 1);
    invisibleButtonGroup = 0;
}

void QSDialogPrivate::add(QSWidget *widget)
{
    QWidget *w = widget->widget;

    int row = grid->rowCount();
    if (qobject_cast<QSLabeled *>(widget)) {
	QLabel *label = ((QSLabeled*)widget)->labelWidget;
	label->setShown(!label->text().isEmpty());
 	grid->addWidget(label, row, 0);
 	grid->addWidget(w, row, 1);
    } else {
	grid->addWidget(w, row, 0,  1, 2);
    }

    if (w && qobject_cast<QRadioButton *>(w)) {
	if (!invisibleButtonGroup)
	    invisibleButtonGroup = new QButtonGroup(this);
	invisibleButtonGroup->addButton(qobject_cast<QRadioButton *>(w));
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

void QSDialog::setTitle(const QString &title) { d->setWindowTitle(title); }
QString QSDialog::title() const { return d->windowTitle(); }
void QSDialog::setWidth(int width){ d->width = width; }
int QSDialog::width() const { return d->width; }
void QSDialog::setOkButtonText(const QString &text) { d->okButton->setText(text); }
QString QSDialog::okButtonText() const { return d->okButton->text(); }
void QSDialog::setCancelButtonText(const QString &text) { d->cancelButton->setText(text); }
QString QSDialog::cancelButtonText() const { return d->cancelButton->text(); }

void QSDialog::newTab(const QString &label)
{
    if (!d->tabWidget) {
	d->tabWidget = new QTabWidget(d);
	int row = d->grid->rowCount();
	d->grid->addWidget(d->tabWidget, row, 0, 1, 2);
    }
    QWidget *w = new QWidget;
    d->tabWidget->addTab(w, label);
    d->parent = w;
    d->hbox = new QHBoxLayout(w);
    d->grid = new QGridLayout(0);
    d->hbox->addLayout(d->grid);
    d->grid->setAlignment(Qt::AlignTop);
}

void QSDialog::newColumn()
{
    if (d->grid->rowCount()) {
	d->hbox->addSpacing(17);
	d->grid = new QGridLayout(0);
    d->hbox->addLayout(d->grid);
	d->grid->setAlignment(Qt::AlignTop);
    }
}

void QSDialog::addSpace(int space)
{
    d->addSpace(space);
}

void QSDialog::add(QSWidget *widget)
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
    if (qsa_is_non_gui_thread()) {
	qWarning("QSInputDialogfactory::QSInputDialogFactory(), "
		 "factory cannot be used in non GUI thread");
	return;
    }
    registerClass(QString::fromLatin1("Dialog"), &QSDialog::staticMetaObject);
    registerClass(QString::fromLatin1("Label"), &QSLabel::staticMetaObject);
    registerClass(QString::fromLatin1("LineEdit"), &QSLineEdit::staticMetaObject);
    registerClass(QString::fromLatin1("NumberEdit"), &QSNumberEdit::staticMetaObject);
    registerClass(QString::fromLatin1("DateEdit"), &QSDateEdit::staticMetaObject,
                  new QSDateEditEnums);
    registerClass(QString::fromLatin1("TimeEdit"), &QSTimeEdit::staticMetaObject);
    registerClass(QString::fromLatin1("TextEdit"), &QSTextEdit::staticMetaObject);
    registerClass(QString::fromLatin1("SpinBox"), &QSSpinBox::staticMetaObject);
    registerClass(QString::fromLatin1("CheckBox"), &QSCheckBox::staticMetaObject);
    registerClass(QString::fromLatin1("RadioButton"), &QSRadioButton::staticMetaObject);
    registerClass(QString::fromLatin1("ComboBox"), &QSComboBox::staticMetaObject);
    registerClass(QString::fromLatin1("GroupBox"), &QSGroupBox::staticMetaObject);
    registerClass(QString::fromLatin1("MessageBox"), 0, new QSMessageBox());
    registerClass(QString::fromLatin1("FileDialog"), 0, new QSFileDialog());
    registerClass(QString::fromLatin1("Input"), 0, new QSInput());
}

QObject *QSInputDialogFactory::create(const QString &className,
                                      const QVariantList &arguments,
                                      QObject * /*context*/)
{
    if (qsa_is_non_gui_thread()) {
	qWarning("QSInputDialogfactory::create(), "
		 "factory cannot be used in non GUI thread");
	return 0;
    }

    if (className == QString::fromLatin1("Dialog")) {
	QString title = arguments.size() >= 1 ? qvariant_cast<QString>(arguments[0]) : QString();
	QObject *obj = arguments.size() >= 2 ? qvariant_cast<QObject *>(arguments[1]) : 0;
	QWidget *widget = obj && qobject_cast<QWidget *>(obj) ? (QWidget*) obj : 0;
	return new QSDialog(title, widget);
    } else if (className == QString::fromLatin1("Label")) {
	return new QSLabel;
    } else if (className == QString::fromLatin1("LineEdit")) {
	return new QSLineEdit;
    } else if (className == QString::fromLatin1("NumberEdit")) {
	return new QSNumberEdit;
    } else if (className == QString::fromLatin1("DateEdit")) {
	return new QSDateEdit;
    } else if (className == QString::fromLatin1("TimeEdit")) {
	return new QSTimeEdit;
    } else if (className == QString::fromLatin1("TextEdit")) {
	return new QSTextEdit;
    } else if (className == QString::fromLatin1("SpinBox")) {
	return new QSSpinBox;
    } else if (className == QString::fromLatin1("CheckBox")) {
	return new QSCheckBox;
    } else if (className == QString::fromLatin1("RadioButton")) {
	return new QSRadioButton;
    } else if (className == QString::fromLatin1("ComboBox")) {
	return new QSComboBox;
    } else if (className == QString::fromLatin1("GroupBox")) {
	return new QSGroupBox;
    }
    return 0;
}

#include "qsinputdialogfactory.moc"
