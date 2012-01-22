/********************************************************************************
** Form generated from reading UI file 'comments.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMMENTS_H
#define UI_COMMENTS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>
#include "colorpicker.h"

QT_BEGIN_NAMESPACE

class Ui_Comments
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QFrame *hFrame;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    ColorPicker *hColor;
    QPushButton *apply;
    QLabel *linkLabel;
    QTextEdit *content;
    QLineEdit *author;
    QLabel *label_3;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *Comments)
    {
        if (Comments->objectName().isEmpty())
            Comments->setObjectName(QString::fromUtf8("Comments"));
        Comments->resize(307, 147);
        gridLayout = new QGridLayout(Comments);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(Comments);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        hFrame = new QFrame(Comments);
        hFrame->setObjectName(QString::fromUtf8("hFrame"));
        hFrame->setFrameShape(QFrame::StyledPanel);
        hFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(hFrame);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(hFrame);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        hColor = new ColorPicker(hFrame);
        hColor->setObjectName(QString::fromUtf8("hColor"));

        horizontalLayout->addWidget(hColor);


        gridLayout->addWidget(hFrame, 4, 0, 1, 3);

        apply = new QPushButton(Comments);
        apply->setObjectName(QString::fromUtf8("apply"));

        gridLayout->addWidget(apply, 4, 3, 1, 1);

        linkLabel = new QLabel(Comments);
        linkLabel->setObjectName(QString::fromUtf8("linkLabel"));

        gridLayout->addWidget(linkLabel, 0, 1, 1, 1);

        content = new QTextEdit(Comments);
        content->setObjectName(QString::fromUtf8("content"));

        gridLayout->addWidget(content, 1, 0, 2, 3);

        author = new QLineEdit(Comments);
        author->setObjectName(QString::fromUtf8("author"));

        gridLayout->addWidget(author, 2, 3, 1, 1);

        label_3 = new QLabel(Comments);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_3, 1, 3, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 3, 3, 1, 1);


        retranslateUi(Comments);
        QObject::connect(apply, SIGNAL(clicked()), Comments, SLOT(apply()));

        QMetaObject::connectSlotsByName(Comments);
    } // setupUi

    void retranslateUi(QWidget *Comments)
    {
        Comments->setWindowTitle(QApplication::translate("Comments", "Add Annotation", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Comments", "Comment:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Comments", "Highlight color", 0, QApplication::UnicodeUTF8));
        apply->setText(QApplication::translate("Comments", "Apply", 0, QApplication::UnicodeUTF8));
        linkLabel->setText(QApplication::translate("Comments", "Write link title", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Comments", "Autor", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Comments: public Ui_Comments {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMMENTS_H
