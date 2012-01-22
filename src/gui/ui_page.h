/********************************************************************************
** Form generated from reading UI file 'page.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_H
#define UI_PAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_page
{
public:
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QLabel *label;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QWidget *page)
    {
        if (page->objectName().isEmpty())
            page->setObjectName(QString::fromUtf8("page"));
        page->resize(400, 300);
        page->setMouseTracking(true);
        horizontalLayout = new QHBoxLayout(page);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label = new QLabel(page);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMouseTracking(true);
        label->setScaledContents(true);
        label->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(label);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        retranslateUi(page);

        QMetaObject::connectSlotsByName(page);
    } // setupUi

    void retranslateUi(QWidget *page)
    {
        page->setWindowTitle(QApplication::translate("page", "Form", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("page", "fff", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class page: public Ui_page {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_H
