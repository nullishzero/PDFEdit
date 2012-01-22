/********************************************************************************
** Form generated from reading UI file 'textFrame.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEXTFRAME_H
#define UI_TEXTFRAME_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TextFrame
{
public:
    QHBoxLayout *horizontalLayout;
    QButtonGroup *buttonGroup;

    void setupUi(QWidget *TextFrame)
    {
        if (TextFrame->objectName().isEmpty())
            TextFrame->setObjectName(QString::fromUtf8("TextFrame"));
        TextFrame->resize(542, 247);
        horizontalLayout = new QHBoxLayout(TextFrame);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

        retranslateUi(TextFrame);

        QMetaObject::connectSlotsByName(TextFrame);
    } // setupUi

    void retranslateUi(QWidget *TextFrame)
    {
        TextFrame->setWindowTitle(QApplication::translate("TextFrame", "Text tools", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TextFrame: public Ui_TextFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEXTFRAME_H
