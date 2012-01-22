/********************************************************************************
** Form generated from reading UI file 'search.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SEARCH_H
#define UI_SEARCH_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Search
{
public:
    QGridLayout *gridLayout;
    QLineEdit *text;
    QToolButton *prevButton;
    QToolButton *nextButton;
    QCheckBox *caseSensitive;
    QLabel *searchLabel;
    QCheckBox *regexp;
    QToolButton *stopButton;
    QCheckBox *concate;

    void setupUi(QWidget *Search)
    {
        if (Search->objectName().isEmpty())
            Search->setObjectName(QString::fromUtf8("Search"));
        Search->resize(324, 91);
        Search->setFocusPolicy(Qt::StrongFocus);
        gridLayout = new QGridLayout(Search);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        text = new QLineEdit(Search);
        text->setObjectName(QString::fromUtf8("text"));

        gridLayout->addWidget(text, 0, 2, 1, 1);

        prevButton = new QToolButton(Search);
        prevButton->setObjectName(QString::fromUtf8("prevButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/previousPage.png"), QSize(), QIcon::Normal, QIcon::Off);
        prevButton->setIcon(icon);
        prevButton->setToolButtonStyle(Qt::ToolButtonFollowStyle);

        gridLayout->addWidget(prevButton, 0, 3, 1, 1);

        nextButton = new QToolButton(Search);
        nextButton->setObjectName(QString::fromUtf8("nextButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/nextpage.png"), QSize(), QIcon::Normal, QIcon::Off);
        nextButton->setIcon(icon1);
        nextButton->setToolButtonStyle(Qt::ToolButtonFollowStyle);

        gridLayout->addWidget(nextButton, 0, 4, 1, 1);

        caseSensitive = new QCheckBox(Search);
        caseSensitive->setObjectName(QString::fromUtf8("caseSensitive"));

        gridLayout->addWidget(caseSensitive, 2, 1, 1, 1);

        searchLabel = new QLabel(Search);
        searchLabel->setObjectName(QString::fromUtf8("searchLabel"));

        gridLayout->addWidget(searchLabel, 0, 1, 1, 1);

        regexp = new QCheckBox(Search);
        regexp->setObjectName(QString::fromUtf8("regexp"));

        gridLayout->addWidget(regexp, 2, 2, 1, 1);

        stopButton = new QToolButton(Search);
        stopButton->setObjectName(QString::fromUtf8("stopButton"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/stop.png"), QSize(), QIcon::Normal, QIcon::Off);
        stopButton->setIcon(icon2);

        gridLayout->addWidget(stopButton, 3, 4, 1, 1);

        concate = new QCheckBox(Search);
        concate->setObjectName(QString::fromUtf8("concate"));

        gridLayout->addWidget(concate, 3, 1, 1, 1);


        retranslateUi(Search);

        QMetaObject::connectSlotsByName(Search);
    } // setupUi

    void retranslateUi(QWidget *Search)
    {
        Search->setWindowTitle(QApplication::translate("Search", "Search", 0, QApplication::UnicodeUTF8));
        prevButton->setText(QApplication::translate("Search", "Previous", 0, QApplication::UnicodeUTF8));
        nextButton->setText(QApplication::translate("Search", "Next", 0, QApplication::UnicodeUTF8));
        caseSensitive->setText(QApplication::translate("Search", "Case sensitive", 0, QApplication::UnicodeUTF8));
        searchLabel->setText(QApplication::translate("Search", "Search:", 0, QApplication::UnicodeUTF8));
        regexp->setText(QApplication::translate("Search", "Regular expression (BETA)", 0, QApplication::UnicodeUTF8));
        stopButton->setText(QString());
        concate->setText(QApplication::translate("Search", "Concate split words", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Search: public Ui_Search {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SEARCH_H
