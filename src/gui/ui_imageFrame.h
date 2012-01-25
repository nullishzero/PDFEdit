/********************************************************************************
** Form generated from reading UI file 'imageFrame.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMAGEFRAME_H
#define UI_IMAGEFRAME_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ImageFrame
{
public:
    QHBoxLayout *horizontalLayout;
    QButtonGroup *mainGrup;

    void setupUi(QWidget *ImageFrame)
    {
        if (ImageFrame->objectName().isEmpty())
            ImageFrame->setObjectName(QString::fromUtf8("ImageFrame"));
        ImageFrame->resize(545, 171);
        horizontalLayout = new QHBoxLayout(ImageFrame);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

        retranslateUi(ImageFrame);

        QMetaObject::connectSlotsByName(ImageFrame);
    } // setupUi

    void retranslateUi(QWidget *ImageFrame)
    {
        ImageFrame->setWindowTitle(QApplication::translate("ImageFrame", "Image tools", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ImageFrame: public Ui_ImageFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMAGEFRAME_H
