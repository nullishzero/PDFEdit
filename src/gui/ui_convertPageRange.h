/********************************************************************************
** Form generated from reading UI file 'convertPageRange.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONVERTPAGERANGE_H
#define UI_CONVERTPAGERANGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_PageDialog
{
public:
    QFormLayout *formLayout;
    QSpinBox *begin;
    QSpinBox *end;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *PageDialog)
    {
        if (PageDialog->objectName().isEmpty())
            PageDialog->setObjectName(QString::fromUtf8("PageDialog"));
        PageDialog->resize(141, 108);
        formLayout = new QFormLayout(PageDialog);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        begin = new QSpinBox(PageDialog);
        begin->setObjectName(QString::fromUtf8("begin"));

        formLayout->setWidget(0, QFormLayout::LabelRole, begin);

        end = new QSpinBox(PageDialog);
        end->setObjectName(QString::fromUtf8("end"));

        formLayout->setWidget(0, QFormLayout::FieldRole, end);

        buttonBox = new QDialogButtonBox(PageDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        formLayout->setWidget(3, QFormLayout::FieldRole, buttonBox);


        retranslateUi(PageDialog);
        QObject::connect(buttonBox, SIGNAL(rejected()), PageDialog, SLOT(reject()));
        QObject::connect(buttonBox, SIGNAL(accepted()), PageDialog, SLOT(accept()));

        QMetaObject::connectSlotsByName(PageDialog);
    } // setupUi

    void retranslateUi(QDialog *PageDialog)
    {
        PageDialog->setWindowTitle(QApplication::translate("PageDialog", "Dialog", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class PageDialog: public Ui_PageDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONVERTPAGERANGE_H
