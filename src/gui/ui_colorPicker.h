/********************************************************************************
** Form generated from reading UI file 'colorPicker.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORPICKER_H
#define UI_COLORPICKER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QToolButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_colorPick
{
public:
    QHBoxLayout *horizontalLayout;
    QToolButton *colors;

    void setupUi(QWidget *colorPick)
    {
        if (colorPick->objectName().isEmpty())
            colorPick->setObjectName(QString::fromUtf8("colorPick"));
        colorPick->resize(94, 56);
        colorPick->setMinimumSize(QSize(0, 0));
        colorPick->setAutoFillBackground(true);
        horizontalLayout = new QHBoxLayout(colorPick);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        colors = new QToolButton(colorPick);
        colors->setObjectName(QString::fromUtf8("colors"));
        colors->setIconSize(QSize(32, 32));

        horizontalLayout->addWidget(colors);


        retranslateUi(colorPick);

        QMetaObject::connectSlotsByName(colorPick);
    } // setupUi

    void retranslateUi(QWidget *colorPick)
    {
        colorPick->setWindowTitle(QApplication::translate("colorPick", "Form", 0, QApplication::UnicodeUTF8));
        colors->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class colorPick: public Ui_colorPick {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORPICKER_H
