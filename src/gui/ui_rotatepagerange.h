/********************************************************************************
** Form generated from reading UI file 'rotatepagerange.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ROTATEPAGERANGE_H
#define UI_ROTATEPAGERANGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDial>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_rotatePageRangeClass
{
public:
    QGridLayout *gridLayout;
    QSpinBox *beginPage;
    QDial *dial;
    QSpinBox *endPage;
    QPushButton *OkButton;

    void setupUi(QWidget *rotatePageRangeClass)
    {
        if (rotatePageRangeClass->objectName().isEmpty())
            rotatePageRangeClass->setObjectName(QString::fromUtf8("rotatePageRangeClass"));
        rotatePageRangeClass->resize(141, 97);
        gridLayout = new QGridLayout(rotatePageRangeClass);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        beginPage = new QSpinBox(rotatePageRangeClass);
        beginPage->setObjectName(QString::fromUtf8("beginPage"));

        gridLayout->addWidget(beginPage, 0, 0, 1, 1);

        dial = new QDial(rotatePageRangeClass);
        dial->setObjectName(QString::fromUtf8("dial"));

        gridLayout->addWidget(dial, 0, 1, 2, 1);

        endPage = new QSpinBox(rotatePageRangeClass);
        endPage->setObjectName(QString::fromUtf8("endPage"));

        gridLayout->addWidget(endPage, 1, 0, 1, 1);

        OkButton = new QPushButton(rotatePageRangeClass);
        OkButton->setObjectName(QString::fromUtf8("OkButton"));

        gridLayout->addWidget(OkButton, 2, 1, 1, 1);


        retranslateUi(rotatePageRangeClass);

        QMetaObject::connectSlotsByName(rotatePageRangeClass);
    } // setupUi

    void retranslateUi(QWidget *rotatePageRangeClass)
    {
        rotatePageRangeClass->setWindowTitle(QApplication::translate("rotatePageRangeClass", "rotatePageRange", 0, QApplication::UnicodeUTF8));
        OkButton->setText(QApplication::translate("rotatePageRangeClass", "Done", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class rotatePageRangeClass: public Ui_rotatePageRangeClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ROTATEPAGERANGE_H
