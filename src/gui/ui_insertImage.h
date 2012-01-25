/********************************************************************************
** Form generated from reading UI file 'insertImage.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INSERTIMAGE_H
#define UI_INSERTIMAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDial>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Image
{
public:
    QGridLayout *gridLayout_2;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_6;
    QFrame *mainFrame;
    QGridLayout *gridLayout;
    QFrame *frame_2;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QSpinBox *positionX;
    QSpinBox *positionY;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QSpinBox *scaleX;
    QSpinBox *scaleY;
    QFrame *frame_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QSpinBox *sizeX;
    QSpinBox *sizeY;
    QFrame *imageChooseFrame;
    QHBoxLayout *horizontalLayout_5;
    QLineEdit *lineEdit;
    QPushButton *chooseFile;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QLabel *rotationabel;
    QDial *rotation;
    QLabel *angleText;
    QPushButton *apply;

    void setupUi(QWidget *Image)
    {
        if (Image->objectName().isEmpty())
            Image->setObjectName(QString::fromUtf8("Image"));
        Image->resize(509, 197);
        gridLayout_2 = new QGridLayout(Image);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        groupBox = new QGroupBox(Image);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setTitle(QString::fromUtf8(""));
        horizontalLayout_6 = new QHBoxLayout(groupBox);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        mainFrame = new QFrame(groupBox);
        mainFrame->setObjectName(QString::fromUtf8("mainFrame"));
        mainFrame->setFrameShape(QFrame::StyledPanel);
        mainFrame->setFrameShadow(QFrame::Raised);
        gridLayout = new QGridLayout(mainFrame);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        frame_2 = new QFrame(mainFrame);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        horizontalLayout_3 = new QHBoxLayout(frame_2);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(-1, 0, 200, 0);
        label_2 = new QLabel(frame_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_3->addWidget(label_2);

        positionX = new QSpinBox(frame_2);
        positionX->setObjectName(QString::fromUtf8("positionX"));
        positionX->setMaximum(5000);

        horizontalLayout_3->addWidget(positionX);

        positionY = new QSpinBox(frame_2);
        positionY->setObjectName(QString::fromUtf8("positionY"));
        positionY->setMaximum(5000);

        horizontalLayout_3->addWidget(positionY);


        gridLayout->addWidget(frame_2, 3, 0, 1, 1);

        widget = new QWidget(mainFrame);
        widget->setObjectName(QString::fromUtf8("widget"));
        horizontalLayout_2 = new QHBoxLayout(widget);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(-1, 0, 200, 0);
        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        scaleX = new QSpinBox(widget);
        scaleX->setObjectName(QString::fromUtf8("scaleX"));
        scaleX->setMinimum(50);
        scaleX->setMaximum(150);
        scaleX->setValue(100);

        horizontalLayout_2->addWidget(scaleX);

        scaleY = new QSpinBox(widget);
        scaleY->setObjectName(QString::fromUtf8("scaleY"));
        scaleY->setMinimum(50);
        scaleY->setMaximum(150);
        scaleY->setValue(100);

        horizontalLayout_2->addWidget(scaleY);


        gridLayout->addWidget(widget, 2, 0, 1, 1);

        frame_3 = new QFrame(mainFrame);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        frame_3->setFrameShape(QFrame::StyledPanel);
        frame_3->setFrameShadow(QFrame::Raised);
        horizontalLayout_4 = new QHBoxLayout(frame_3);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(9, 0, 200, 0);
        label_3 = new QLabel(frame_3);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_4->addWidget(label_3);

        sizeX = new QSpinBox(frame_3);
        sizeX->setObjectName(QString::fromUtf8("sizeX"));
        sizeX->setMinimum(20);
        sizeX->setMaximum(640);

        horizontalLayout_4->addWidget(sizeX);

        sizeY = new QSpinBox(frame_3);
        sizeY->setObjectName(QString::fromUtf8("sizeY"));
        sizeY->setMinimum(20);
        sizeY->setMaximum(640);

        horizontalLayout_4->addWidget(sizeY);


        gridLayout->addWidget(frame_3, 1, 0, 1, 1);

        imageChooseFrame = new QFrame(mainFrame);
        imageChooseFrame->setObjectName(QString::fromUtf8("imageChooseFrame"));
        imageChooseFrame->setFrameShape(QFrame::StyledPanel);
        imageChooseFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout_5 = new QHBoxLayout(imageChooseFrame);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        lineEdit = new QLineEdit(imageChooseFrame);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout_5->addWidget(lineEdit);

        chooseFile = new QPushButton(imageChooseFrame);
        chooseFile->setObjectName(QString::fromUtf8("chooseFile"));
        chooseFile->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(chooseFile->sizePolicy().hasHeightForWidth());
        chooseFile->setSizePolicy(sizePolicy);
        chooseFile->setAutoDefault(true);

        horizontalLayout_5->addWidget(chooseFile);


        gridLayout->addWidget(imageChooseFrame, 0, 0, 1, 1);

        frame = new QFrame(mainFrame);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, -1, 300, -1);
        rotationabel = new QLabel(frame);
        rotationabel->setObjectName(QString::fromUtf8("rotationabel"));

        horizontalLayout->addWidget(rotationabel);

        rotation = new QDial(frame);
        rotation->setObjectName(QString::fromUtf8("rotation"));
        rotation->setMaximumSize(QSize(30, 30));
        rotation->setMaximum(359);
        rotation->setTracking(false);
        rotation->setOrientation(Qt::Horizontal);
        rotation->setWrapping(true);
        rotation->setNotchesVisible(false);

        horizontalLayout->addWidget(rotation);

        angleText = new QLabel(frame);
        angleText->setObjectName(QString::fromUtf8("angleText"));
        angleText->setEnabled(false);

        horizontalLayout->addWidget(angleText);


        gridLayout->addWidget(frame, 4, 0, 1, 1);


        horizontalLayout_6->addWidget(mainFrame);

        apply = new QPushButton(groupBox);
        apply->setObjectName(QString::fromUtf8("apply"));

        horizontalLayout_6->addWidget(apply);


        gridLayout_2->addWidget(groupBox, 0, 0, 1, 1);


        retranslateUi(Image);
        QObject::connect(apply, SIGNAL(clicked()), Image, SLOT(apply()));
        QObject::connect(chooseFile, SIGNAL(clicked()), Image, SLOT(setImagePath()));

        QMetaObject::connectSlotsByName(Image);
    } // setupUi

    void retranslateUi(QWidget *Image)
    {
        Image->setWindowTitle(QApplication::translate("Image", "Form", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Image", "position", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Image", "Scale", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Image", "Size", 0, QApplication::UnicodeUTF8));
        chooseFile->setText(QApplication::translate("Image", "Choose", 0, QApplication::UnicodeUTF8));
        rotationabel->setText(QApplication::translate("Image", "Rotation", 0, QApplication::UnicodeUTF8));
        angleText->setText(QApplication::translate("Image", "0", 0, QApplication::UnicodeUTF8));
        apply->setText(QApplication::translate("Image", "Ok", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Image: public Ui_Image {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INSERTIMAGE_H
