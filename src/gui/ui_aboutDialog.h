/********************************************************************************
** Form generated from reading UI file 'aboutDialog.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG_H
#define UI_ABOUTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *info;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
        AboutDialog->resize(400, 300);
        horizontalLayout = new QHBoxLayout(AboutDialog);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        info = new QLabel(AboutDialog);
        info->setObjectName(QString::fromUtf8("info"));
        info->setAlignment(Qt::AlignCenter);
        info->setTextInteractionFlags(Qt::TextBrowserInteraction);

        horizontalLayout->addWidget(info);


        retranslateUi(AboutDialog);

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About Pdf Editor", 0, QApplication::UnicodeUTF8));
        info->setText(QApplication::translate("AboutDialog", "Pdf Editor v1.12\n"
"\n"
" Created by Eva Peskova \n"
"\n"
"peva@matfyz.cz", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG_H
