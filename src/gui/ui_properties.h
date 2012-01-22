/********************************************************************************
** Form generated from reading UI file 'properties.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROPERTIES_H
#define UI_PROPERTIES_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDial>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QWidget>
#include "colorpicker.h"

QT_BEGIN_NAMESPACE

class Ui_Properties
{
public:
    QGridLayout *fLayout;
    QFrame *propFrame;
    QGridLayout *gridLayout;
    QLabel *Props;
    ColorPicker *colorN;
    QLabel *borderLabel;
    ColorPicker *colorS;
    QLabel *strokeLabel;
    QDial *rotation;
    QLabel *rotateLabel;
    QComboBox *fontsize;
    QLabel *label;
    QComboBox *fonts;
    QLabel *label_2;
    QComboBox *shape;
    QLabel *shapeLabel;
    QLabel *insertTextLabel;
    QLineEdit *text;
    QPushButton *apply;
    QToolButton *selectFont;
    QLabel *angleLabel;

    void setupUi(QWidget *Properties)
    {
        if (Properties->objectName().isEmpty())
            Properties->setObjectName(QString::fromUtf8("Properties"));
        Properties->resize(316, 258);
        fLayout = new QGridLayout(Properties);
        fLayout->setObjectName(QString::fromUtf8("fLayout"));
        propFrame = new QFrame(Properties);
        propFrame->setObjectName(QString::fromUtf8("propFrame"));
        propFrame->setFrameShape(QFrame::StyledPanel);
        propFrame->setFrameShadow(QFrame::Raised);
        gridLayout = new QGridLayout(propFrame);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        Props = new QLabel(propFrame);
        Props->setObjectName(QString::fromUtf8("Props"));

        gridLayout->addWidget(Props, 1, 0, 1, 1);

        colorN = new ColorPicker(propFrame);
        colorN->setObjectName(QString::fromUtf8("colorN"));

        gridLayout->addWidget(colorN, 8, 2, 1, 3);

        borderLabel = new QLabel(propFrame);
        borderLabel->setObjectName(QString::fromUtf8("borderLabel"));

        gridLayout->addWidget(borderLabel, 8, 0, 1, 1);

        colorS = new ColorPicker(propFrame);
        colorS->setObjectName(QString::fromUtf8("colorS"));
        colorS->setEnabled(true);

        gridLayout->addWidget(colorS, 7, 2, 1, 3);

        strokeLabel = new QLabel(propFrame);
        strokeLabel->setObjectName(QString::fromUtf8("strokeLabel"));

        gridLayout->addWidget(strokeLabel, 7, 0, 1, 1);

        rotation = new QDial(propFrame);
        rotation->setObjectName(QString::fromUtf8("rotation"));
        rotation->setMaximumSize(QSize(50, 30));
        rotation->setMaximum(359);
        rotation->setInvertedAppearance(false);
        rotation->setInvertedControls(false);

        gridLayout->addWidget(rotation, 6, 2, 1, 1);

        rotateLabel = new QLabel(propFrame);
        rotateLabel->setObjectName(QString::fromUtf8("rotateLabel"));

        gridLayout->addWidget(rotateLabel, 6, 0, 1, 1);

        fontsize = new QComboBox(propFrame);
        fontsize->setObjectName(QString::fromUtf8("fontsize"));
        fontsize->setInsertPolicy(QComboBox::InsertAlphabetically);
        fontsize->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        gridLayout->addWidget(fontsize, 5, 2, 1, 3);

        label = new QLabel(propFrame);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 5, 0, 1, 1);

        fonts = new QComboBox(propFrame);
        fonts->setObjectName(QString::fromUtf8("fonts"));
        fonts->setInsertPolicy(QComboBox::InsertAtTop);

        gridLayout->addWidget(fonts, 4, 2, 1, 1);

        label_2 = new QLabel(propFrame);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 4, 0, 1, 1);

        shape = new QComboBox(propFrame);
        shape->setObjectName(QString::fromUtf8("shape"));

        gridLayout->addWidget(shape, 3, 2, 1, 3);

        shapeLabel = new QLabel(propFrame);
        shapeLabel->setObjectName(QString::fromUtf8("shapeLabel"));

        gridLayout->addWidget(shapeLabel, 3, 0, 1, 1);

        insertTextLabel = new QLabel(propFrame);
        insertTextLabel->setObjectName(QString::fromUtf8("insertTextLabel"));

        gridLayout->addWidget(insertTextLabel, 2, 0, 1, 1);

        text = new QLineEdit(propFrame);
        text->setObjectName(QString::fromUtf8("text"));
        text->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        text->setPlaceholderText(QString::fromUtf8(""));

        gridLayout->addWidget(text, 2, 2, 1, 3);

        apply = new QPushButton(propFrame);
        apply->setObjectName(QString::fromUtf8("apply"));

        gridLayout->addWidget(apply, 9, 2, 1, 1);

        selectFont = new QToolButton(propFrame);
        selectFont->setObjectName(QString::fromUtf8("selectFont"));

        gridLayout->addWidget(selectFont, 4, 4, 1, 1);

        angleLabel = new QLabel(propFrame);
        angleLabel->setObjectName(QString::fromUtf8("angleLabel"));
        angleLabel->setEnabled(false);

        gridLayout->addWidget(angleLabel, 6, 4, 1, 1);


        fLayout->addWidget(propFrame, 0, 1, 1, 1);


        retranslateUi(Properties);
        QObject::connect(apply, SIGNAL(clicked()), Properties, SLOT(apply()));

        QMetaObject::connectSlotsByName(Properties);
    } // setupUi

    void retranslateUi(QWidget *Properties)
    {
        Properties->setWindowTitle(QApplication::translate("Properties", "Text properties", 0, QApplication::UnicodeUTF8));
        Props->setText(QApplication::translate("Properties", "Properties", 0, QApplication::UnicodeUTF8));
        borderLabel->setText(QApplication::translate("Properties", "border color", 0, QApplication::UnicodeUTF8));
        strokeLabel->setText(QApplication::translate("Properties", "Fill color", 0, QApplication::UnicodeUTF8));
        rotateLabel->setText(QApplication::translate("Properties", "Rotation", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Properties", "Font size", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        fonts->setToolTip(QApplication::translate("Properties", "If \"select font\" is selected chhose a font from pae, otherwise choose predefined fonts", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("Properties", "Font", 0, QApplication::UnicodeUTF8));
        shapeLabel->setText(QApplication::translate("Properties", "Drawing mode", 0, QApplication::UnicodeUTF8));
        insertTextLabel->setText(QApplication::translate("Properties", "Text to insert", 0, QApplication::UnicodeUTF8));
        apply->setText(QApplication::translate("Properties", "Apply", 0, QApplication::UnicodeUTF8));
        selectFont->setText(QApplication::translate("Properties", "Select font from page", 0, QApplication::UnicodeUTF8));
        angleLabel->setText(QApplication::translate("Properties", "0", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Properties: public Ui_Properties {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROPERTIES_H
