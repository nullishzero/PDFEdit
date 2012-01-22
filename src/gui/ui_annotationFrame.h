/********************************************************************************
** Form generated from reading UI file 'annotationFrame.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ANNOTATIONFRAME_H
#define UI_ANNOTATIONFRAME_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AnnotationFrame
{
public:
    QHBoxLayout *horizontalLayout;
    QButtonGroup *buttongroup;

    void setupUi(QWidget *AnnotationFrame)
    {
        if (AnnotationFrame->objectName().isEmpty())
            AnnotationFrame->setObjectName(QString::fromUtf8("AnnotationFrame"));
        AnnotationFrame->resize(338, 73);
        horizontalLayout = new QHBoxLayout(AnnotationFrame);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

        retranslateUi(AnnotationFrame);

        QMetaObject::connectSlotsByName(AnnotationFrame);
    } // setupUi

    void retranslateUi(QWidget *AnnotationFrame)
    {
        AnnotationFrame->setWindowTitle(QApplication::translate("AnnotationFrame", "Annotations tools", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AnnotationFrame: public Ui_AnnotationFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ANNOTATIONFRAME_H
