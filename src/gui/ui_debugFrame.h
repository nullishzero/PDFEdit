/********************************************************************************
** Form generated from reading UI file 'debugFrame.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEBUGFRAME_H
#define UI_DEBUGFRAME_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DebugFrame
{
public:
    QHBoxLayout *horizontalLayout;

    void setupUi(QWidget *DebugFrame)
    {
        if (DebugFrame->objectName().isEmpty())
            DebugFrame->setObjectName(QString::fromUtf8("DebugFrame"));
        DebugFrame->resize(440, 213);
        horizontalLayout = new QHBoxLayout(DebugFrame);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

        retranslateUi(DebugFrame);

        QMetaObject::connectSlotsByName(DebugFrame);
    } // setupUi

    void retranslateUi(QWidget *DebugFrame)
    {
        DebugFrame->setWindowTitle(QApplication::translate("DebugFrame", "Debugging tools", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DebugFrame: public Ui_DebugFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEBUGFRAME_H
