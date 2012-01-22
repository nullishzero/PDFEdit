/********************************************************************************
** Form generated from reading UI file 'insertpagerange.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INSERTPAGERANGE_H
#define UI_INSERTPAGERANGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InsertPageRangeClass
{
public:
    QPushButton *OkButton;
    QPushButton *CancelButton;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLabel *label;
    QSpinBox *beginBox;
    QLabel *label_2;
    QSpinBox *endbox;
    QLabel *preview;

    void setupUi(QDialog *InsertPageRangeClass)
    {
        if (InsertPageRangeClass->objectName().isEmpty())
            InsertPageRangeClass->setObjectName(QString::fromUtf8("InsertPageRangeClass"));
        InsertPageRangeClass->resize(399, 334);
        OkButton = new QPushButton(InsertPageRangeClass);
        OkButton->setObjectName(QString::fromUtf8("OkButton"));
        OkButton->setGeometry(QRect(220, 300, 75, 23));
        CancelButton = new QPushButton(InsertPageRangeClass);
        CancelButton->setObjectName(QString::fromUtf8("CancelButton"));
        CancelButton->setGeometry(QRect(310, 300, 75, 23));
        gridLayoutWidget = new QWidget(InsertPageRangeClass);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(20, 60, 160, 80));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(gridLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        beginBox = new QSpinBox(gridLayoutWidget);
        beginBox->setObjectName(QString::fromUtf8("beginBox"));

        gridLayout->addWidget(beginBox, 0, 1, 1, 1);

        label_2 = new QLabel(gridLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        endbox = new QSpinBox(gridLayoutWidget);
        endbox->setObjectName(QString::fromUtf8("endbox"));

        gridLayout->addWidget(endbox, 1, 1, 1, 1);

        preview = new QLabel(InsertPageRangeClass);
        preview->setObjectName(QString::fromUtf8("preview"));
        preview->setGeometry(QRect(210, 20, 150, 250));
        preview->setMinimumSize(QSize(150, 200));

        retranslateUi(InsertPageRangeClass);
        QObject::connect(OkButton, SIGNAL(clicked()), InsertPageRangeClass, SLOT(accept()));
        QObject::connect(CancelButton, SIGNAL(clicked()), InsertPageRangeClass, SLOT(reject()));

        QMetaObject::connectSlotsByName(InsertPageRangeClass);
    } // setupUi

    void retranslateUi(QDialog *InsertPageRangeClass)
    {
        InsertPageRangeClass->setWindowTitle(QApplication::translate("InsertPageRangeClass", "InsertPageRange", 0, QApplication::UnicodeUTF8));
        OkButton->setText(QApplication::translate("InsertPageRangeClass", "Ok", 0, QApplication::UnicodeUTF8));
        CancelButton->setText(QApplication::translate("InsertPageRangeClass", "Cancel", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("InsertPageRangeClass", "begin", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("InsertPageRangeClass", "end", 0, QApplication::UnicodeUTF8));
        preview->setText(QApplication::translate("InsertPageRangeClass", "TextLabel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class InsertPageRangeClass: public Ui_InsertPageRangeClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INSERTPAGERANGE_H
