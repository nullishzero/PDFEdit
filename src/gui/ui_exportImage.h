/********************************************************************************
** Form generated from reading UI file 'exportImage.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXPORTIMAGE_H
#define UI_EXPORTIMAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Dialog
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *frame;
    QHBoxLayout *horizontalLayout_3;
    QFrame *frame_4;
    QVBoxLayout *verticalLayout_2;
    QFrame *frame_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSpinBox *spinBox;
    QSpinBox *spinBox_3;
    QFrame *frame_3;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QSpinBox *spinBox_2;
    QSpinBox *spinBox_4;
    QSpacerItem *verticalSpacer;
    QLabel *image;
    QHBoxLayout *box1;
    QSpacerItem *spacer1;
    QPushButton *okButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QString::fromUtf8("Dialog"));
        Dialog->resize(593, 347);
        verticalLayout = new QVBoxLayout(Dialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        frame = new QFrame(Dialog);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout_3 = new QHBoxLayout(frame);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        frame_4 = new QFrame(frame);
        frame_4->setObjectName(QString::fromUtf8("frame_4"));
        frame_4->setFrameShape(QFrame::StyledPanel);
        frame_4->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(frame_4);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        frame_2 = new QFrame(frame_4);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame_2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(frame_2);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        spinBox = new QSpinBox(frame_2);
        spinBox->setObjectName(QString::fromUtf8("spinBox"));

        horizontalLayout->addWidget(spinBox);

        spinBox_3 = new QSpinBox(frame_2);
        spinBox_3->setObjectName(QString::fromUtf8("spinBox_3"));

        horizontalLayout->addWidget(spinBox_3);


        verticalLayout_2->addWidget(frame_2);

        frame_3 = new QFrame(frame_4);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        frame_3->setFrameShape(QFrame::StyledPanel);
        frame_3->setFrameShadow(QFrame::Raised);
        horizontalLayout_2 = new QHBoxLayout(frame_3);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(frame_3);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        spinBox_2 = new QSpinBox(frame_3);
        spinBox_2->setObjectName(QString::fromUtf8("spinBox_2"));

        horizontalLayout_2->addWidget(spinBox_2);

        spinBox_4 = new QSpinBox(frame_3);
        spinBox_4->setObjectName(QString::fromUtf8("spinBox_4"));

        horizontalLayout_2->addWidget(spinBox_4);


        verticalLayout_2->addWidget(frame_3);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);


        horizontalLayout_3->addWidget(frame_4);

        image = new QLabel(frame);
        image->setObjectName(QString::fromUtf8("image"));

        horizontalLayout_3->addWidget(image);

        horizontalLayout_3->setStretch(1, 100);

        verticalLayout->addWidget(frame);

        box1 = new QHBoxLayout();
#ifndef Q_OS_MAC
        box1->setSpacing(6);
#endif
        box1->setContentsMargins(0, 0, 0, 0);
        box1->setObjectName(QString::fromUtf8("box1"));
        box1->setSizeConstraint(QLayout::SetMinimumSize);
        spacer1 = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        box1->addItem(spacer1);

        okButton = new QPushButton(Dialog);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        box1->addWidget(okButton);

        cancelButton = new QPushButton(Dialog);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        box1->addWidget(cancelButton);


        verticalLayout->addLayout(box1);

        verticalLayout->setStretch(0, 30);
        frame->raise();

        retranslateUi(Dialog);
        QObject::connect(okButton, SIGNAL(clicked()), Dialog, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), Dialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog", "TextLabel", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Dialog", "TextLabel", 0, QApplication::UnicodeUTF8));
        image->setText(QApplication::translate("Dialog", "TextLabel", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("Dialog", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("Dialog", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXPORTIMAGE_H
