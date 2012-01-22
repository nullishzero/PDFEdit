/********************************************************************************
** Form generated from reading UI file 'Tab.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAB_H
#define UI_TAB_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSpacerItem>
#include <QtGui/QSplitter>
#include <QtGui/QTextEdit>
#include <QtGui/QToolButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TabUI
{
public:
    QGridLayout *TabLayout;
    QProgressBar *progressBar;
    QGroupBox *displayManipulation;
    QHBoxLayout *horizontalLayout_3;
    QComboBox *revision;
    QToolButton *commitButton;
    QToolButton *branchRevision;
    QSpacerItem *horizontalSpacer_4;
    QToolButton *firstPage;
    QToolButton *previous;
    QLineEdit *pageInfo;
    QToolButton *next;
    QToolButton *lastPage;
    QToolButton *minusZoom;
    QComboBox *zoom;
    QToolButton *plusZoom;
    QSpacerItem *horizontalSpacer_3;
    QToolButton *documentInfo;
    QLabel *stateLabel;
    QSplitter *splitter;
    QFrame *frame;
    QGridLayout *gridLayout;
    QPushButton *showAnalyzeButton;
    QPushButton *historyButton;
    QPushButton *showBookmark;
    QSpacerItem *verticalSpacer;
    QTreeWidget *tree;
    QTextEdit *historyText;
    QTreeWidget *analyzeTree;
    QScrollArea *scrollArea;
    QWidget *content;

    void setupUi(QWidget *TabUI)
    {
        if (TabUI->objectName().isEmpty())
            TabUI->setObjectName(QString::fromUtf8("TabUI"));
        TabUI->resize(648, 533);
        TabLayout = new QGridLayout(TabUI);
        TabLayout->setContentsMargins(0, 0, 0, 0);
        TabLayout->setObjectName(QString::fromUtf8("TabLayout"));
        TabLayout->setHorizontalSpacing(0);
        TabLayout->setVerticalSpacing(4);
        progressBar = new QProgressBar(TabUI);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(0);
        progressBar->setOrientation(Qt::Horizontal);
        progressBar->setTextDirection(QProgressBar::TopToBottom);

        TabLayout->addWidget(progressBar, 7, 0, 1, 2);

        displayManipulation = new QGroupBox(TabUI);
        displayManipulation->setObjectName(QString::fromUtf8("displayManipulation"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(displayManipulation->sizePolicy().hasHeightForWidth());
        displayManipulation->setSizePolicy(sizePolicy);
        displayManipulation->setAlignment(Qt::AlignCenter);
        horizontalLayout_3 = new QHBoxLayout(displayManipulation);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        revision = new QComboBox(displayManipulation);
        revision->setObjectName(QString::fromUtf8("revision"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(revision->sizePolicy().hasHeightForWidth());
        revision->setSizePolicy(sizePolicy1);

        horizontalLayout_3->addWidget(revision);

        commitButton = new QToolButton(displayManipulation);
        commitButton->setObjectName(QString::fromUtf8("commitButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/commit.png"), QSize(), QIcon::Normal, QIcon::Off);
        commitButton->setIcon(icon);
        commitButton->setIconSize(QSize(16, 16));
        commitButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_3->addWidget(commitButton);

        branchRevision = new QToolButton(displayManipulation);
        branchRevision->setObjectName(QString::fromUtf8("branchRevision"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/export.png"), QSize(), QIcon::Normal, QIcon::Off);
        branchRevision->setIcon(icon1);
        branchRevision->setIconSize(QSize(16, 16));
        branchRevision->setToolButtonStyle(Qt::ToolButtonIconOnly);

        horizontalLayout_3->addWidget(branchRevision);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);

        firstPage = new QToolButton(displayManipulation);
        firstPage->setObjectName(QString::fromUtf8("firstPage"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/firstPage.png"), QSize(), QIcon::Normal, QIcon::Off);
        firstPage->setIcon(icon2);

        horizontalLayout_3->addWidget(firstPage);

        previous = new QToolButton(displayManipulation);
        previous->setObjectName(QString::fromUtf8("previous"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/previousPage.png"), QSize(), QIcon::Normal, QIcon::Off);
        previous->setIcon(icon3);
        previous->setIconSize(QSize(16, 16));
        previous->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        horizontalLayout_3->addWidget(previous);

        pageInfo = new QLineEdit(displayManipulation);
        pageInfo->setObjectName(QString::fromUtf8("pageInfo"));
        pageInfo->setContextMenuPolicy(Qt::NoContextMenu);
        pageInfo->setAlignment(Qt::AlignCenter);
        pageInfo->setReadOnly(false);

        horizontalLayout_3->addWidget(pageInfo);

        next = new QToolButton(displayManipulation);
        next->setObjectName(QString::fromUtf8("next"));
        next->setBaseSize(QSize(10, 30));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/nextpage.png"), QSize(), QIcon::Normal, QIcon::Off);
        next->setIcon(icon4);
        next->setIconSize(QSize(16, 16));
        next->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        horizontalLayout_3->addWidget(next);

        lastPage = new QToolButton(displayManipulation);
        lastPage->setObjectName(QString::fromUtf8("lastPage"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/images/lastPage.png"), QSize(), QIcon::Normal, QIcon::Off);
        lastPage->setIcon(icon5);

        horizontalLayout_3->addWidget(lastPage);

        minusZoom = new QToolButton(displayManipulation);
        minusZoom->setObjectName(QString::fromUtf8("minusZoom"));

        horizontalLayout_3->addWidget(minusZoom);

        zoom = new QComboBox(displayManipulation);
        zoom->setObjectName(QString::fromUtf8("zoom"));
        zoom->setAcceptDrops(true);

        horizontalLayout_3->addWidget(zoom);

        plusZoom = new QToolButton(displayManipulation);
        plusZoom->setObjectName(QString::fromUtf8("plusZoom"));

        horizontalLayout_3->addWidget(plusZoom);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);

        documentInfo = new QToolButton(displayManipulation);
        documentInfo->setObjectName(QString::fromUtf8("documentInfo"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/images/about.png"), QSize(), QIcon::Normal, QIcon::Off);
        documentInfo->setIcon(icon6);

        horizontalLayout_3->addWidget(documentInfo);


        TabLayout->addWidget(displayManipulation, 2, 1, 1, 1);

        stateLabel = new QLabel(TabUI);
        stateLabel->setObjectName(QString::fromUtf8("stateLabel"));
        sizePolicy.setHeightForWidth(stateLabel->sizePolicy().hasHeightForWidth());
        stateLabel->setSizePolicy(sizePolicy);

        TabLayout->addWidget(stateLabel, 1, 1, 1, 1);

        splitter = new QSplitter(TabUI);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy2);
        splitter->setOrientation(Qt::Horizontal);
        splitter->setChildrenCollapsible(false);
        frame = new QFrame(splitter);
        frame->setObjectName(QString::fromUtf8("frame"));
        sizePolicy2.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy2);
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        gridLayout = new QGridLayout(frame);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, -1, 0, -1);
        showAnalyzeButton = new QPushButton(frame);
        showAnalyzeButton->setObjectName(QString::fromUtf8("showAnalyzeButton"));
        showAnalyzeButton->setMaximumSize(QSize(20, 16777215));
        showAnalyzeButton->setStyleSheet(QString::fromUtf8(""));
        showAnalyzeButton->setCheckable(true);
        showAnalyzeButton->setDefault(true);
        showAnalyzeButton->setFlat(true);

        gridLayout->addWidget(showAnalyzeButton, 2, 1, 1, 1);

        historyButton = new QPushButton(frame);
        historyButton->setObjectName(QString::fromUtf8("historyButton"));
        historyButton->setMaximumSize(QSize(20, 16777215));
        historyButton->setStyleSheet(QString::fromUtf8(""));
        historyButton->setCheckable(true);
        historyButton->setDefault(true);
        historyButton->setFlat(false);

        gridLayout->addWidget(historyButton, 0, 1, 1, 1);

        showBookmark = new QPushButton(frame);
        showBookmark->setObjectName(QString::fromUtf8("showBookmark"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(showBookmark->sizePolicy().hasHeightForWidth());
        showBookmark->setSizePolicy(sizePolicy3);
        showBookmark->setMaximumSize(QSize(20, 16777215));
        showBookmark->setLayoutDirection(Qt::LeftToRight);
        showBookmark->setStyleSheet(QString::fromUtf8(""));
        showBookmark->setCheckable(true);
        showBookmark->setDefault(true);
        showBookmark->setFlat(true);

        gridLayout->addWidget(showBookmark, 1, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 3, 1, 1, 1);

        tree = new QTreeWidget(frame);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        tree->setHeaderItem(__qtreewidgetitem);
        tree->setObjectName(QString::fromUtf8("tree"));
        sizePolicy3.setHeightForWidth(tree->sizePolicy().hasHeightForWidth());
        tree->setSizePolicy(sizePolicy3);
        tree->setProperty("showDropIndicator", QVariant(false));
        tree->setRootIsDecorated(true);
        tree->setAnimated(true);

        gridLayout->addWidget(tree, 0, 6, 4, 1);

        historyText = new QTextEdit(frame);
        historyText->setObjectName(QString::fromUtf8("historyText"));
        sizePolicy3.setHeightForWidth(historyText->sizePolicy().hasHeightForWidth());
        historyText->setSizePolicy(sizePolicy3);
        historyText->setReadOnly(true);

        gridLayout->addWidget(historyText, 0, 5, 4, 1);

        analyzeTree = new QTreeWidget(frame);
        QTreeWidgetItem *__qtreewidgetitem1 = new QTreeWidgetItem();
        __qtreewidgetitem1->setText(0, QString::fromUtf8("1"));
        analyzeTree->setHeaderItem(__qtreewidgetitem1);
        analyzeTree->setObjectName(QString::fromUtf8("analyzeTree"));
        sizePolicy3.setHeightForWidth(analyzeTree->sizePolicy().hasHeightForWidth());
        analyzeTree->setSizePolicy(sizePolicy3);
        analyzeTree->setAnimated(true);

        gridLayout->addWidget(analyzeTree, 0, 4, 4, 1);

        splitter->addWidget(frame);
        scrollArea = new QScrollArea(splitter);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setMouseTracking(true);
        scrollArea->setAlignment(Qt::AlignCenter);
        content = new QWidget();
        content->setObjectName(QString::fromUtf8("content"));
        content->setGeometry(QRect(0, 118, 429, 177));
        content->setMouseTracking(true);
        scrollArea->setWidget(content);
        analyzeTree->raise();
        historyText->raise();
        tree->raise();
        splitter->addWidget(scrollArea);

        TabLayout->addWidget(splitter, 3, 1, 1, 1);


        retranslateUi(TabUI);
        QObject::connect(historyButton, SIGNAL(toggled(bool)), historyText, SLOT(setVisible(bool)));
        QObject::connect(showBookmark, SIGNAL(toggled(bool)), tree, SLOT(setVisible(bool)));
        QObject::connect(showAnalyzeButton, SIGNAL(toggled(bool)), analyzeTree, SLOT(setVisible(bool)));

        QMetaObject::connectSlotsByName(TabUI);
    } // setupUi

    void retranslateUi(QWidget *TabUI)
    {
        TabUI->setWindowTitle(QApplication::translate("TabUI", "Viewer", 0, QApplication::UnicodeUTF8));
        progressBar->setFormat(QApplication::translate("TabUI", "Work done%p%", 0, QApplication::UnicodeUTF8));
        displayManipulation->setTitle(QApplication::translate("TabUI", "Display manipulation", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        revision->setToolTip(QApplication::translate("TabUI", "save revisions(incremental changes)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        commitButton->setToolTip(QApplication::translate("TabUI", "creates new revision", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        commitButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        branchRevision->setToolTip(QApplication::translate("TabUI", "saves this revision to another file", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        branchRevision->setText(QString());
        firstPage->setText(QApplication::translate("TabUI", "First Page", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        previous->setToolTip(QApplication::translate("TabUI", "previous page", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        previous->setText(QString());
        previous->setShortcut(QApplication::translate("TabUI", "P, PgUp, Ctrl+S", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        next->setToolTip(QApplication::translate("TabUI", "next page", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        next->setText(QString());
        next->setShortcut(QApplication::translate("TabUI", "N, PgDown", 0, QApplication::UnicodeUTF8));
        lastPage->setText(QApplication::translate("TabUI", "Last Page", 0, QApplication::UnicodeUTF8));
        minusZoom->setText(QApplication::translate("TabUI", "-", 0, QApplication::UnicodeUTF8));
        plusZoom->setText(QApplication::translate("TabUI", "+", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        documentInfo->setToolTip(QApplication::translate("TabUI", "Information about opened PDF document", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        documentInfo->setText(QApplication::translate("TabUI", "...", 0, QApplication::UnicodeUTF8));
        stateLabel->setText(QString());
#ifndef QT_NO_TOOLTIP
        showAnalyzeButton->setToolTip(QApplication::translate("TabUI", "Analyze pdf document", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        showAnalyzeButton->setText(QApplication::translate("TabUI", "A\n"
"n\n"
"a\n"
"l\n"
"y\n"
"z\n"
"e", 0, QApplication::UnicodeUTF8));
        showAnalyzeButton->setShortcut(QApplication::translate("TabUI", "Ctrl+A", 0, QApplication::UnicodeUTF8));
        historyButton->setText(QApplication::translate("TabUI", "H\n"
"i\n"
"s\n"
"t\n"
"o\n"
"r\n"
"y", 0, QApplication::UnicodeUTF8));
        historyButton->setShortcut(QApplication::translate("TabUI", "Ctrl+H", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        showBookmark->setToolTip(QApplication::translate("TabUI", "Show/hide bookmarks", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        showBookmark->setText(QApplication::translate("TabUI", "B\n"
"o\n"
"o\n"
"k\n"
"m\n"
"a\n"
"r\n"
"k\n"
"s", 0, QApplication::UnicodeUTF8));
        showBookmark->setShortcut(QApplication::translate("TabUI", "Ctrl+B", 0, QApplication::UnicodeUTF8));
        historyText->setDocumentTitle(QApplication::translate("TabUI", "History", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TabUI: public Ui_TabUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB_H
