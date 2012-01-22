/********************************************************************************
** Form generated from reading UI file 'pdfgui.ui'
**
** Created: Mon Jan 16 10:15:19 2012
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PDFGUI_H
#define UI_PDFGUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <openpdf.h>
#include "colorpicker.h"

QT_BEGIN_NAMESPACE

class Ui_pdfGuiClass
{
public:
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionSaveCopy;
    QAction *actionAnalyze;
    QAction *actionRotate_left;
    QAction *actionRotate_right;
    QAction *actionPage_up;
    QAction *actionPage_down;
    QAction *actionDelete_Page;
    QAction *actionExport_text;
    QAction *actionAbout;
    QAction *actionAdd_page;
    QAction *actionFullScreen;
    QAction *actionExit;
    QAction *actionDocument;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QFrame *fileFrame;
    QHBoxLayout *horizontalLayout_2;
    QWidget *main;
    QHBoxLayout *horizontalLayout;
    QToolButton *openButton;
    QToolButton *saveButton;
    QToolButton *textButton;
    QToolButton *imageButton;
    QToolButton *annotationButton;
    QToolButton *viewButton;
    QToolButton *settingsButton;
    QToolButton *debugButton;
    QSpacerItem *horizontalSpacer;
    QFrame *line;
    QFrame *imageFrame;
    QHBoxLayout *horizontalLayout_5;
    QToolButton *insertImageButton;
    QToolButton *selectImageButton;
    QToolButton *changeImageButton;
    QToolButton *deleteImageButton;
    QToolButton *imagePartButton;
    QToolButton *extractButton;
    QSpacerItem *horizontalSpacer_2;
    QFrame *annotationFrame;
    QHBoxLayout *l1;
    QToolButton *insertAnotation;
    QToolButton *linkAnnotation;
    QToolButton *changeAnnotationButton;
    QToolButton *highlightButton;
    QToolButton *delAnnotation;
    QSpacerItem *horizontalSpacer_3;
    QFrame *textFrame;
    QHBoxLayout *horizontalLayout_4;
    QToolButton *selectTextButton;
    QToolButton *insertTextButton;
    QToolButton *changeButton;
    QToolButton *deleteButton;
    QToolButton *eraseButton;
    QToolButton *textHighlightButton;
    QToolButton *deleteHighlightButton;
    QToolButton *searchButton;
    QToolButton *exportTextButton;
    QSpacerItem *horizontalSpacer_4;
    QFrame *debugFrame;
    QHBoxLayout *horizontalLayout_6;
    QToolButton *refreshButton;
    QToolButton *analyzeButton;
    QToolButton *saveEncodedButton;
    QToolButton *opSelect;
    QSpacerItem *horizontalSpacer_5;
    QSpacerItem *horizontalSpacer_6;
    QToolButton *actualButton;
    QFrame *settingFrame;
    QGridLayout *gridLayout;
    QLabel *label;
    ColorPicker *hcolor;
    QSpacerItem *horizontalSpacer_8;
    QLabel *label_2;
    ColorPicker *color;
    QSpacerItem *horizontalSpacer_7;
    QSpacerItem *verticalSpacer;
    OpenPdf *openedPdfs;
    QFrame *line_5;
    QLabel *Help;
    QFrame *line_6;
    QStatusBar *statusBar;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuView;
    QMenu *menuPages;
    QMenu *menuOpen_recent;
    QMenu *menuAbout;
    QButtonGroup *mainGroup;
    QButtonGroup *buttonGroup;

    void setupUi(QMainWindow *pdfGuiClass)
    {
        if (pdfGuiClass->objectName().isEmpty())
            pdfGuiClass->setObjectName(QString::fromUtf8("pdfGuiClass"));
        pdfGuiClass->resize(878, 640);
        pdfGuiClass->setMinimumSize(QSize(600, 400));
        QFont font;
        font.setFamily(QString::fromUtf8("MV Boli"));
        pdfGuiClass->setFont(font);
        pdfGuiClass->setMouseTracking(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/mainIcon.png"), QSize(), QIcon::Normal, QIcon::Off);
        pdfGuiClass->setWindowIcon(icon);
        pdfGuiClass->setWindowOpacity(1);
        actionOpen = new QAction(pdfGuiClass);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/open.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(icon1);
        actionSave = new QAction(pdfGuiClass);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave->setIcon(icon2);
        actionSaveCopy = new QAction(pdfGuiClass);
        actionSaveCopy->setObjectName(QString::fromUtf8("actionSaveCopy"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/savecopy.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSaveCopy->setIcon(icon3);
        actionAnalyze = new QAction(pdfGuiClass);
        actionAnalyze->setObjectName(QString::fromUtf8("actionAnalyze"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/analyze.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAnalyze->setIcon(icon4);
        actionRotate_left = new QAction(pdfGuiClass);
        actionRotate_left->setObjectName(QString::fromUtf8("actionRotate_left"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/images/derotate.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRotate_left->setIcon(icon5);
        actionRotate_right = new QAction(pdfGuiClass);
        actionRotate_right->setObjectName(QString::fromUtf8("actionRotate_right"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/images/rotate.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRotate_right->setIcon(icon6);
        actionPage_up = new QAction(pdfGuiClass);
        actionPage_up->setObjectName(QString::fromUtf8("actionPage_up"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/images/uppage.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionPage_up->setIcon(icon7);
        actionPage_down = new QAction(pdfGuiClass);
        actionPage_down->setObjectName(QString::fromUtf8("actionPage_down"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/images/downpage.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionPage_down->setIcon(icon8);
        actionDelete_Page = new QAction(pdfGuiClass);
        actionDelete_Page->setObjectName(QString::fromUtf8("actionDelete_Page"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/images/deletepage.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDelete_Page->setIcon(icon9);
        actionExport_text = new QAction(pdfGuiClass);
        actionExport_text->setObjectName(QString::fromUtf8("actionExport_text"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/images/TextConvert.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionExport_text->setIcon(icon10);
        actionAbout = new QAction(pdfGuiClass);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/images/about.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAbout->setIcon(icon11);
        actionAdd_page = new QAction(pdfGuiClass);
        actionAdd_page->setObjectName(QString::fromUtf8("actionAdd_page"));
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/images/addpage.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAdd_page->setIcon(icon12);
        actionFullScreen = new QAction(pdfGuiClass);
        actionFullScreen->setObjectName(QString::fromUtf8("actionFullScreen"));
        actionExit = new QAction(pdfGuiClass);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionDocument = new QAction(pdfGuiClass);
        actionDocument->setObjectName(QString::fromUtf8("actionDocument"));
        actionDocument->setIcon(icon11);
        centralWidget = new QWidget(pdfGuiClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        fileFrame = new QFrame(centralWidget);
        fileFrame->setObjectName(QString::fromUtf8("fileFrame"));
        fileFrame->setFrameShape(QFrame::StyledPanel);
        fileFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout_2 = new QHBoxLayout(fileFrame);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        main = new QWidget(fileFrame);
        main->setObjectName(QString::fromUtf8("main"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(main->sizePolicy().hasHeightForWidth());
        main->setSizePolicy(sizePolicy);
        horizontalLayout = new QHBoxLayout(main);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        openButton = new QToolButton(main);
        openButton->setObjectName(QString::fromUtf8("openButton"));
        openButton->setIcon(icon1);
        openButton->setIconSize(QSize(32, 32));
        openButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout->addWidget(openButton);

        saveButton = new QToolButton(main);
        saveButton->setObjectName(QString::fromUtf8("saveButton"));
        saveButton->setIcon(icon2);
        saveButton->setIconSize(QSize(32, 32));
        saveButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout->addWidget(saveButton);

        textButton = new QToolButton(main);
        mainGroup = new QButtonGroup(pdfGuiClass);
        mainGroup->setObjectName(QString::fromUtf8("mainGroup"));
        mainGroup->addButton(textButton);
        textButton->setObjectName(QString::fromUtf8("textButton"));
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/images/insertTextMarker.png"), QSize(), QIcon::Normal, QIcon::Off);
        textButton->setIcon(icon13);
        textButton->setIconSize(QSize(32, 32));
        textButton->setCheckable(true);
        textButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout->addWidget(textButton);

        imageButton = new QToolButton(main);
        mainGroup->addButton(imageButton);
        imageButton->setObjectName(QString::fromUtf8("imageButton"));
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/images/images.png"), QSize(), QIcon::Normal, QIcon::Off);
        imageButton->setIcon(icon14);
        imageButton->setIconSize(QSize(32, 32));
        imageButton->setCheckable(true);
        imageButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout->addWidget(imageButton);

        annotationButton = new QToolButton(main);
        mainGroup->addButton(annotationButton);
        annotationButton->setObjectName(QString::fromUtf8("annotationButton"));
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/images/comment.png"), QSize(), QIcon::Normal, QIcon::Off);
        annotationButton->setIcon(icon15);
        annotationButton->setIconSize(QSize(32, 32));
        annotationButton->setCheckable(true);
        annotationButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout->addWidget(annotationButton);

        viewButton = new QToolButton(main);
        mainGroup->addButton(viewButton);
        viewButton->setObjectName(QString::fromUtf8("viewButton"));
        QIcon icon16;
        icon16.addFile(QString::fromUtf8(":/images/view.png"), QSize(), QIcon::Normal, QIcon::Off);
        viewButton->setIcon(icon16);
        viewButton->setIconSize(QSize(32, 32));
        viewButton->setCheckable(true);
        viewButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout->addWidget(viewButton);

        settingsButton = new QToolButton(main);
        mainGroup->addButton(settingsButton);
        settingsButton->setObjectName(QString::fromUtf8("settingsButton"));
        QIcon icon17;
        icon17.addFile(QString::fromUtf8(":/images/settings.png"), QSize(), QIcon::Normal, QIcon::Off);
        settingsButton->setIcon(icon17);
        settingsButton->setIconSize(QSize(32, 32));
        settingsButton->setCheckable(true);
        settingsButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout->addWidget(settingsButton);

        debugButton = new QToolButton(main);
        mainGroup->addButton(debugButton);
        debugButton->setObjectName(QString::fromUtf8("debugButton"));
        QIcon icon18;
        icon18.addFile(QString::fromUtf8(":/images/debug.png"), QSize(), QIcon::Normal, QIcon::Off);
        debugButton->setIcon(icon18);
        debugButton->setIconSize(QSize(32, 32));
        debugButton->setCheckable(true);
        debugButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout->addWidget(debugButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        line = new QFrame(main);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line);


        horizontalLayout_2->addWidget(main);

        imageFrame = new QFrame(fileFrame);
        imageFrame->setObjectName(QString::fromUtf8("imageFrame"));
        sizePolicy.setHeightForWidth(imageFrame->sizePolicy().hasHeightForWidth());
        imageFrame->setSizePolicy(sizePolicy);
        imageFrame->setFrameShape(QFrame::StyledPanel);
        imageFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout_5 = new QHBoxLayout(imageFrame);
        horizontalLayout_5->setSpacing(0);
        horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        insertImageButton = new QToolButton(imageFrame);
        buttonGroup = new QButtonGroup(pdfGuiClass);
        buttonGroup->setObjectName(QString::fromUtf8("buttonGroup"));
        buttonGroup->addButton(insertImageButton);
        insertImageButton->setObjectName(QString::fromUtf8("insertImageButton"));
        QIcon icon19;
        icon19.addFile(QString::fromUtf8(":/images/addImage.png"), QSize(), QIcon::Normal, QIcon::Off);
        insertImageButton->setIcon(icon19);
        insertImageButton->setIconSize(QSize(32, 32));
        insertImageButton->setCheckable(true);
        insertImageButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_5->addWidget(insertImageButton);

        selectImageButton = new QToolButton(imageFrame);
        buttonGroup->addButton(selectImageButton);
        selectImageButton->setObjectName(QString::fromUtf8("selectImageButton"));
        QIcon icon20;
        icon20.addFile(QString::fromUtf8(":/images/hand.png"), QSize(), QIcon::Normal, QIcon::Off);
        selectImageButton->setIcon(icon20);
        selectImageButton->setIconSize(QSize(32, 32));
        selectImageButton->setCheckable(true);
        selectImageButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_5->addWidget(selectImageButton);

        changeImageButton = new QToolButton(imageFrame);
        changeImageButton->setObjectName(QString::fromUtf8("changeImageButton"));
        QIcon icon21;
        icon21.addFile(QString::fromUtf8(":/images/change.png"), QSize(), QIcon::Normal, QIcon::Off);
        changeImageButton->setIcon(icon21);
        changeImageButton->setIconSize(QSize(32, 32));
        changeImageButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_5->addWidget(changeImageButton);

        deleteImageButton = new QToolButton(imageFrame);
        deleteImageButton->setObjectName(QString::fromUtf8("deleteImageButton"));
        QIcon icon22;
        icon22.addFile(QString::fromUtf8(":/images/DeleteText.png"), QSize(), QIcon::Normal, QIcon::Off);
        deleteImageButton->setIcon(icon22);
        deleteImageButton->setIconSize(QSize(32, 32));
        deleteImageButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_5->addWidget(deleteImageButton);

        imagePartButton = new QToolButton(imageFrame);
        buttonGroup->addButton(imagePartButton);
        imagePartButton->setObjectName(QString::fromUtf8("imagePartButton"));
        QIcon icon23;
        icon23.addFile(QString::fromUtf8(":/images/snapshot.png"), QSize(), QIcon::Normal, QIcon::Off);
        imagePartButton->setIcon(icon23);
        imagePartButton->setIconSize(QSize(32, 32));
        imagePartButton->setCheckable(true);
        imagePartButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_5->addWidget(imagePartButton);

        extractButton = new QToolButton(imageFrame);
        extractButton->setObjectName(QString::fromUtf8("extractButton"));
        extractButton->setIcon(icon23);
        extractButton->setIconSize(QSize(32, 16));
        extractButton->setCheckable(false);
        extractButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_5->addWidget(extractButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_2);


        horizontalLayout_2->addWidget(imageFrame);

        annotationFrame = new QFrame(fileFrame);
        annotationFrame->setObjectName(QString::fromUtf8("annotationFrame"));
        sizePolicy.setHeightForWidth(annotationFrame->sizePolicy().hasHeightForWidth());
        annotationFrame->setSizePolicy(sizePolicy);
        annotationFrame->setFrameShape(QFrame::StyledPanel);
        annotationFrame->setFrameShadow(QFrame::Raised);
        l1 = new QHBoxLayout(annotationFrame);
        l1->setSpacing(0);
        l1->setContentsMargins(0, 0, 0, 0);
        l1->setObjectName(QString::fromUtf8("l1"));
        insertAnotation = new QToolButton(annotationFrame);
        buttonGroup->addButton(insertAnotation);
        insertAnotation->setObjectName(QString::fromUtf8("insertAnotation"));
        QIcon icon24;
        icon24.addFile(QString::fromUtf8(":/images/addComment.png"), QSize(), QIcon::Normal, QIcon::Off);
        insertAnotation->setIcon(icon24);
        insertAnotation->setIconSize(QSize(32, 32));
        insertAnotation->setCheckable(true);
        insertAnotation->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        l1->addWidget(insertAnotation);

        linkAnnotation = new QToolButton(annotationFrame);
        buttonGroup->addButton(linkAnnotation);
        linkAnnotation->setObjectName(QString::fromUtf8("linkAnnotation"));
        QIcon icon25;
        icon25.addFile(QString::fromUtf8(":/images/addLink.png"), QSize(), QIcon::Normal, QIcon::Off);
        linkAnnotation->setIcon(icon25);
        linkAnnotation->setIconSize(QSize(32, 32));
        linkAnnotation->setCheckable(true);
        linkAnnotation->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        l1->addWidget(linkAnnotation);

        changeAnnotationButton = new QToolButton(annotationFrame);
        buttonGroup->addButton(changeAnnotationButton);
        changeAnnotationButton->setObjectName(QString::fromUtf8("changeAnnotationButton"));
        changeAnnotationButton->setIcon(icon21);
        changeAnnotationButton->setIconSize(QSize(32, 32));
        changeAnnotationButton->setCheckable(true);
        changeAnnotationButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        l1->addWidget(changeAnnotationButton);

        highlightButton = new QToolButton(annotationFrame);
        buttonGroup->addButton(highlightButton);
        highlightButton->setObjectName(QString::fromUtf8("highlightButton"));
        QIcon icon26;
        icon26.addFile(QString::fromUtf8(":/images/highlight.png"), QSize(), QIcon::Normal, QIcon::Off);
        highlightButton->setIcon(icon26);
        highlightButton->setIconSize(QSize(32, 32));
        highlightButton->setCheckable(true);
        highlightButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        l1->addWidget(highlightButton);

        delAnnotation = new QToolButton(annotationFrame);
        buttonGroup->addButton(delAnnotation);
        delAnnotation->setObjectName(QString::fromUtf8("delAnnotation"));
        delAnnotation->setIcon(icon22);
        delAnnotation->setIconSize(QSize(32, 32));
        delAnnotation->setCheckable(true);
        delAnnotation->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        l1->addWidget(delAnnotation);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        l1->addItem(horizontalSpacer_3);


        horizontalLayout_2->addWidget(annotationFrame);

        textFrame = new QFrame(fileFrame);
        textFrame->setObjectName(QString::fromUtf8("textFrame"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(textFrame->sizePolicy().hasHeightForWidth());
        textFrame->setSizePolicy(sizePolicy1);
        textFrame->setFrameShape(QFrame::StyledPanel);
        textFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout_4 = new QHBoxLayout(textFrame);
        horizontalLayout_4->setSpacing(0);
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        selectTextButton = new QToolButton(textFrame);
        buttonGroup->addButton(selectTextButton);
        selectTextButton->setObjectName(QString::fromUtf8("selectTextButton"));
        selectTextButton->setIcon(icon20);
        selectTextButton->setIconSize(QSize(32, 32));
        selectTextButton->setCheckable(true);
        selectTextButton->setChecked(false);
        selectTextButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_4->addWidget(selectTextButton);

        insertTextButton = new QToolButton(textFrame);
        buttonGroup->addButton(insertTextButton);
        insertTextButton->setObjectName(QString::fromUtf8("insertTextButton"));
        QIcon icon27;
        icon27.addFile(QString::fromUtf8(":/images/insert_text.png"), QSize(), QIcon::Normal, QIcon::Off);
        insertTextButton->setIcon(icon27);
        insertTextButton->setIconSize(QSize(32, 32));
        insertTextButton->setCheckable(true);
        insertTextButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_4->addWidget(insertTextButton);

        changeButton = new QToolButton(textFrame);
        changeButton->setObjectName(QString::fromUtf8("changeButton"));
        changeButton->setIcon(icon21);
        changeButton->setIconSize(QSize(32, 32));
        changeButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_4->addWidget(changeButton);

        deleteButton = new QToolButton(textFrame);
        deleteButton->setObjectName(QString::fromUtf8("deleteButton"));
        deleteButton->setIcon(icon22);
        deleteButton->setIconSize(QSize(32, 32));
        deleteButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_4->addWidget(deleteButton);

        eraseButton = new QToolButton(textFrame);
        eraseButton->setObjectName(QString::fromUtf8("eraseButton"));
        QIcon icon28;
        icon28.addFile(QString::fromUtf8(":/images/eraseText.png"), QSize(), QIcon::Normal, QIcon::Off);
        eraseButton->setIcon(icon28);
        eraseButton->setIconSize(QSize(32, 32));
        eraseButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_4->addWidget(eraseButton);

        textHighlightButton = new QToolButton(textFrame);
        textHighlightButton->setObjectName(QString::fromUtf8("textHighlightButton"));
        textHighlightButton->setIcon(icon26);
        textHighlightButton->setIconSize(QSize(32, 32));
        textHighlightButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_4->addWidget(textHighlightButton);

        deleteHighlightButton = new QToolButton(textFrame);
        deleteHighlightButton->setObjectName(QString::fromUtf8("deleteHighlightButton"));
        deleteHighlightButton->setIcon(icon28);
        deleteHighlightButton->setIconSize(QSize(32, 32));
        deleteHighlightButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_4->addWidget(deleteHighlightButton);

        searchButton = new QToolButton(textFrame);
        searchButton->setObjectName(QString::fromUtf8("searchButton"));
        QIcon icon29;
        icon29.addFile(QString::fromUtf8(":/images/search.png"), QSize(), QIcon::Normal, QIcon::Off);
        searchButton->setIcon(icon29);
        searchButton->setIconSize(QSize(32, 32));
        searchButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_4->addWidget(searchButton);

        exportTextButton = new QToolButton(textFrame);
        exportTextButton->setObjectName(QString::fromUtf8("exportTextButton"));
        exportTextButton->setIcon(icon10);
        exportTextButton->setIconSize(QSize(32, 32));
        exportTextButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_4->addWidget(exportTextButton);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_4);


        horizontalLayout_2->addWidget(textFrame);

        debugFrame = new QFrame(fileFrame);
        debugFrame->setObjectName(QString::fromUtf8("debugFrame"));
        sizePolicy1.setHeightForWidth(debugFrame->sizePolicy().hasHeightForWidth());
        debugFrame->setSizePolicy(sizePolicy1);
        debugFrame->setFrameShape(QFrame::StyledPanel);
        debugFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout_6 = new QHBoxLayout(debugFrame);
        horizontalLayout_6->setSpacing(0);
        horizontalLayout_6->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        refreshButton = new QToolButton(debugFrame);
        refreshButton->setObjectName(QString::fromUtf8("refreshButton"));
        QIcon icon30;
        icon30.addFile(QString::fromUtf8(":/images/refresh.png"), QSize(), QIcon::Normal, QIcon::Off);
        refreshButton->setIcon(icon30);
        refreshButton->setIconSize(QSize(32, 32));
        refreshButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_6->addWidget(refreshButton);

        analyzeButton = new QToolButton(debugFrame);
        analyzeButton->setObjectName(QString::fromUtf8("analyzeButton"));
        analyzeButton->setIcon(icon4);
        analyzeButton->setIconSize(QSize(32, 32));
        analyzeButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_6->addWidget(analyzeButton);

        saveEncodedButton = new QToolButton(debugFrame);
        saveEncodedButton->setObjectName(QString::fromUtf8("saveEncodedButton"));
        QIcon icon31;
        icon31.addFile(QString::fromUtf8(":/images/decrypt_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        saveEncodedButton->setIcon(icon31);
        saveEncodedButton->setIconSize(QSize(32, 32));
        saveEncodedButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_6->addWidget(saveEncodedButton);

        opSelect = new QToolButton(debugFrame);
        opSelect->setObjectName(QString::fromUtf8("opSelect"));
        QIcon icon32;
        icon32.addFile(QString::fromUtf8(":/images/operator.png"), QSize(), QIcon::Normal, QIcon::Off);
        opSelect->setIcon(icon32);
        opSelect->setIconSize(QSize(32, 32));
        opSelect->setCheckable(true);
        opSelect->setChecked(false);
        opSelect->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_6->addWidget(opSelect);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_5);


        horizontalLayout_2->addWidget(debugFrame);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_6);

        actualButton = new QToolButton(fileFrame);
        actualButton->setObjectName(QString::fromUtf8("actualButton"));
        actualButton->setIconSize(QSize(32, 32));
        actualButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_2->addWidget(actualButton);


        verticalLayout->addWidget(fileFrame);

        settingFrame = new QFrame(centralWidget);
        settingFrame->setObjectName(QString::fromUtf8("settingFrame"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(settingFrame->sizePolicy().hasHeightForWidth());
        settingFrame->setSizePolicy(sizePolicy2);
        settingFrame->setFrameShape(QFrame::StyledPanel);
        settingFrame->setFrameShadow(QFrame::Raised);
        gridLayout = new QGridLayout(settingFrame);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(settingFrame);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 2, 1, 1);

        hcolor = new ColorPicker(settingFrame);
        hcolor->setObjectName(QString::fromUtf8("hcolor"));

        gridLayout->addWidget(hcolor, 0, 4, 1, 1);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_8, 0, 5, 1, 1);

        label_2 = new QLabel(settingFrame);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 3, 2, 1, 1);

        color = new ColorPicker(settingFrame);
        color->setObjectName(QString::fromUtf8("color"));

        gridLayout->addWidget(color, 3, 4, 1, 1);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_7, 3, 5, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 4, 0, 1, 1);


        verticalLayout->addWidget(settingFrame);

        openedPdfs = new OpenPdf(centralWidget);
        openedPdfs->setObjectName(QString::fromUtf8("openedPdfs"));
        openedPdfs->setAcceptDrops(true);
        openedPdfs->setAutoFillBackground(true);
        openedPdfs->setTabPosition(QTabWidget::North);
        openedPdfs->setTabShape(QTabWidget::Triangular);
        openedPdfs->setTabsClosable(true);
        openedPdfs->setMovable(true);

        verticalLayout->addWidget(openedPdfs);

        line_5 = new QFrame(centralWidget);
        line_5->setObjectName(QString::fromUtf8("line_5"));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_5);

        Help = new QLabel(centralWidget);
        Help->setObjectName(QString::fromUtf8("Help"));
        Help->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(Help);

        line_6 = new QFrame(centralWidget);
        line_6->setObjectName(QString::fromUtf8("line_6"));
        line_6->setFrameShape(QFrame::HLine);
        line_6->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_6);

        pdfGuiClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(pdfGuiClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        pdfGuiClass->setStatusBar(statusBar);
        menuBar = new QMenuBar(pdfGuiClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 878, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuView = new QMenu(menuFile);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuPages = new QMenu(menuFile);
        menuPages->setObjectName(QString::fromUtf8("menuPages"));
        menuOpen_recent = new QMenu(menuFile);
        menuOpen_recent->setObjectName(QString::fromUtf8("menuOpen_recent"));
        menuOpen_recent->setIcon(icon1);
        menuAbout = new QMenu(menuBar);
        menuAbout->setObjectName(QString::fromUtf8("menuAbout"));
        pdfGuiClass->setMenuBar(menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuAbout->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(menuOpen_recent->menuAction());
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSaveCopy);
        menuFile->addAction(menuPages->menuAction());
        menuFile->addAction(menuView->menuAction());
        menuFile->addAction(actionAnalyze);
        menuFile->addAction(actionDocument);
        menuFile->addAction(actionExport_text);
        menuFile->addAction(actionExit);
        menuView->addAction(actionRotate_left);
        menuView->addAction(actionRotate_right);
        menuView->addAction(actionFullScreen);
        menuPages->addAction(actionPage_up);
        menuPages->addAction(actionPage_down);
        menuPages->addAction(actionDelete_Page);
        menuPages->addAction(actionAdd_page);
        menuOpen_recent->addSeparator();
        menuAbout->addAction(actionAbout);

        retranslateUi(pdfGuiClass);
        QObject::connect(openedPdfs, SIGNAL(tabCloseRequested(int)), openedPdfs, SLOT(closeAndRemoveTab(int)));
        QObject::connect(settingsButton, SIGNAL(toggled(bool)), settingFrame, SLOT(setVisible(bool)));
        QObject::connect(settingsButton, SIGNAL(toggled(bool)), openedPdfs, SLOT(setHidden(bool)));

        openedPdfs->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(pdfGuiClass);
    } // setupUi

    void retranslateUi(QMainWindow *pdfGuiClass)
    {
        pdfGuiClass->setWindowTitle(QApplication::translate("pdfGuiClass", "TIAEditor v1.28", 0, QApplication::UnicodeUTF8));
        actionOpen->setText(QApplication::translate("pdfGuiClass", "Open", 0, QApplication::UnicodeUTF8));
        actionOpen->setShortcut(QApplication::translate("pdfGuiClass", "Ctrl+O", 0, QApplication::UnicodeUTF8));
        actionSave->setText(QApplication::translate("pdfGuiClass", "Save", 0, QApplication::UnicodeUTF8));
        actionSave->setShortcut(QApplication::translate("pdfGuiClass", "Ctrl+S", 0, QApplication::UnicodeUTF8));
        actionSaveCopy->setText(QApplication::translate("pdfGuiClass", "SaveCopy", 0, QApplication::UnicodeUTF8));
        actionSaveCopy->setShortcut(QApplication::translate("pdfGuiClass", "Ctrl+Shift+S", 0, QApplication::UnicodeUTF8));
        actionAnalyze->setText(QApplication::translate("pdfGuiClass", "Analyze", 0, QApplication::UnicodeUTF8));
        actionAnalyze->setShortcut(QApplication::translate("pdfGuiClass", "Ctrl+A", 0, QApplication::UnicodeUTF8));
        actionRotate_left->setText(QApplication::translate("pdfGuiClass", "Rotate left", 0, QApplication::UnicodeUTF8));
        actionRotate_left->setShortcut(QApplication::translate("pdfGuiClass", "Ctrl+L", 0, QApplication::UnicodeUTF8));
        actionRotate_right->setText(QApplication::translate("pdfGuiClass", "Rotate right", 0, QApplication::UnicodeUTF8));
        actionRotate_right->setShortcut(QApplication::translate("pdfGuiClass", "Ctrl+R", 0, QApplication::UnicodeUTF8));
        actionPage_up->setText(QApplication::translate("pdfGuiClass", "Page up", 0, QApplication::UnicodeUTF8));
        actionPage_down->setText(QApplication::translate("pdfGuiClass", "Page down", 0, QApplication::UnicodeUTF8));
        actionDelete_Page->setText(QApplication::translate("pdfGuiClass", "Delete page", 0, QApplication::UnicodeUTF8));
        actionExport_text->setText(QApplication::translate("pdfGuiClass", "Export text", 0, QApplication::UnicodeUTF8));
        actionExport_text->setShortcut(QApplication::translate("pdfGuiClass", "Ctrl+E", 0, QApplication::UnicodeUTF8));
        actionAbout->setText(QApplication::translate("pdfGuiClass", "About", 0, QApplication::UnicodeUTF8));
        actionAdd_page->setText(QApplication::translate("pdfGuiClass", "Add page", 0, QApplication::UnicodeUTF8));
        actionFullScreen->setText(QApplication::translate("pdfGuiClass", "FullScreen", 0, QApplication::UnicodeUTF8));
        actionFullScreen->setShortcut(QApplication::translate("pdfGuiClass", "F11", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("pdfGuiClass", "Exit", 0, QApplication::UnicodeUTF8));
        actionDocument->setText(QApplication::translate("pdfGuiClass", "Document info", 0, QApplication::UnicodeUTF8));
        actionDocument->setShortcut(QApplication::translate("pdfGuiClass", "Ctrl+I", 0, QApplication::UnicodeUTF8));
        openButton->setText(QApplication::translate("pdfGuiClass", "Open", 0, QApplication::UnicodeUTF8));
        saveButton->setText(QApplication::translate("pdfGuiClass", "Save", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        textButton->setToolTip(QApplication::translate("pdfGuiClass", "show text operations", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        textButton->setText(QApplication::translate("pdfGuiClass", "Text", 0, QApplication::UnicodeUTF8));
        textButton->setShortcut(QApplication::translate("pdfGuiClass", "T", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        imageButton->setToolTip(QApplication::translate("pdfGuiClass", "show image operation", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        imageButton->setText(QApplication::translate("pdfGuiClass", "&Images", 0, QApplication::UnicodeUTF8));
        imageButton->setShortcut(QApplication::translate("pdfGuiClass", "I", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        annotationButton->setToolTip(QApplication::translate("pdfGuiClass", "show annotation operations", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        annotationButton->setText(QApplication::translate("pdfGuiClass", "&Annotations", 0, QApplication::UnicodeUTF8));
        annotationButton->setShortcut(QApplication::translate("pdfGuiClass", "A", 0, QApplication::UnicodeUTF8));
        viewButton->setText(QApplication::translate("pdfGuiClass", "View", 0, QApplication::UnicodeUTF8));
        viewButton->setShortcut(QApplication::translate("pdfGuiClass", "V", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        settingsButton->setToolTip(QApplication::translate("pdfGuiClass", "show setting dialog", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        settingsButton->setText(QApplication::translate("pdfGuiClass", "Colors", 0, QApplication::UnicodeUTF8));
        settingsButton->setShortcut(QApplication::translate("pdfGuiClass", "S", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        debugButton->setToolTip(QApplication::translate("pdfGuiClass", "show debug options", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        debugButton->setText(QApplication::translate("pdfGuiClass", "Debug", 0, QApplication::UnicodeUTF8));
        debugButton->setShortcut(QApplication::translate("pdfGuiClass", "D", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        insertImageButton->setToolTip(QApplication::translate("pdfGuiClass", "Insert inline image", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        insertImageButton->setText(QApplication::translate("pdfGuiClass", "Add image", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        selectImageButton->setToolTip(QApplication::translate("pdfGuiClass", "select inline image", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        selectImageButton->setText(QApplication::translate("pdfGuiClass", "Select", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        changeImageButton->setToolTip(QApplication::translate("pdfGuiClass", "change inline image", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        changeImageButton->setText(QApplication::translate("pdfGuiClass", "Change", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        deleteImageButton->setToolTip(QApplication::translate("pdfGuiClass", "delete inline image", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        deleteImageButton->setText(QApplication::translate("pdfGuiClass", "Delete", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        imagePartButton->setToolTip(QApplication::translate("pdfGuiClass", "extract part of pdf as image", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        imagePartButton->setText(QApplication::translate("pdfGuiClass", "SnapShot", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        extractButton->setToolTip(QApplication::translate("pdfGuiClass", "extract inline image", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        extractButton->setText(QApplication::translate("pdfGuiClass", "Extract\n"
"(imageOnly)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        insertAnotation->setToolTip(QApplication::translate("pdfGuiClass", "insert text annotation", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        insertAnotation->setText(QApplication::translate("pdfGuiClass", "Insert comment", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        linkAnnotation->setToolTip(QApplication::translate("pdfGuiClass", "insert link annotation", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        linkAnnotation->setText(QApplication::translate("pdfGuiClass", "Insert link", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        changeAnnotationButton->setToolTip(QApplication::translate("pdfGuiClass", "click & change annotation", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        changeAnnotationButton->setText(QApplication::translate("pdfGuiClass", "Change", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        highlightButton->setToolTip(QApplication::translate("pdfGuiClass", "insert highlight annotation", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        highlightButton->setText(QApplication::translate("pdfGuiClass", "Highlight", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        delAnnotation->setToolTip(QApplication::translate("pdfGuiClass", "delete annotation", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        delAnnotation->setText(QApplication::translate("pdfGuiClass", "Delete", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        selectTextButton->setToolTip(QApplication::translate("pdfGuiClass", "select text", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        selectTextButton->setText(QApplication::translate("pdfGuiClass", "Select", 0, QApplication::UnicodeUTF8));
        selectTextButton->setShortcut(QApplication::translate("pdfGuiClass", "S", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        insertTextButton->setToolTip(QApplication::translate("pdfGuiClass", "insert text", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        insertTextButton->setText(QApplication::translate("pdfGuiClass", "Insert", 0, QApplication::UnicodeUTF8));
        insertTextButton->setShortcut(QApplication::translate("pdfGuiClass", "I", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        changeButton->setToolTip(QApplication::translate("pdfGuiClass", "change text", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        changeButton->setText(QApplication::translate("pdfGuiClass", "Change", 0, QApplication::UnicodeUTF8));
        changeButton->setShortcut(QApplication::translate("pdfGuiClass", "C", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        deleteButton->setToolTip(QApplication::translate("pdfGuiClass", "delete text (leaves no empty space)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        deleteButton->setText(QApplication::translate("pdfGuiClass", "Delete ", 0, QApplication::UnicodeUTF8));
        deleteButton->setShortcut(QApplication::translate("pdfGuiClass", "D", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        eraseButton->setToolTip(QApplication::translate("pdfGuiClass", "replace text with blank space", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        eraseButton->setText(QApplication::translate("pdfGuiClass", "Fill with blank", 0, QApplication::UnicodeUTF8));
        eraseButton->setShortcut(QApplication::translate("pdfGuiClass", "E", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        textHighlightButton->setToolTip(QApplication::translate("pdfGuiClass", "highlight text", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        textHighlightButton->setText(QApplication::translate("pdfGuiClass", "Highlight", 0, QApplication::UnicodeUTF8));
        textHighlightButton->setShortcut(QApplication::translate("pdfGuiClass", "H", 0, QApplication::UnicodeUTF8));
        deleteHighlightButton->setText(QApplication::translate("pdfGuiClass", "Delete highlight", 0, QApplication::UnicodeUTF8));
        searchButton->setText(QApplication::translate("pdfGuiClass", "Search", 0, QApplication::UnicodeUTF8));
        searchButton->setShortcut(QApplication::translate("pdfGuiClass", "S", 0, QApplication::UnicodeUTF8));
        exportTextButton->setText(QApplication::translate("pdfGuiClass", "Export to text", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        refreshButton->setToolTip(QApplication::translate("pdfGuiClass", "redraw page", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        refreshButton->setText(QApplication::translate("pdfGuiClass", "Refresh", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        analyzeButton->setToolTip(QApplication::translate("pdfGuiClass", "create analyze tree", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        analyzeButton->setText(QApplication::translate("pdfGuiClass", "Analyze!", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        saveEncodedButton->setToolTip(QApplication::translate("pdfGuiClass", "save decoded", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        saveEncodedButton->setText(QApplication::translate("pdfGuiClass", "&Decode", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        opSelect->setToolTip(QApplication::translate("pdfGuiClass", "select operators", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        opSelect->setText(QApplication::translate("pdfGuiClass", "Operators", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actualButton->setToolTip(QApplication::translate("pdfGuiClass", "Actual mode", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actualButton->setText(QString());
        label->setText(QApplication::translate("pdfGuiClass", "Select default color for highlighting annotation", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("pdfGuiClass", "Select color for selecting text", 0, QApplication::UnicodeUTF8));
        Help->setText(QString());
        menuFile->setTitle(QApplication::translate("pdfGuiClass", "File", 0, QApplication::UnicodeUTF8));
        menuView->setTitle(QApplication::translate("pdfGuiClass", "Rotation", 0, QApplication::UnicodeUTF8));
        menuPages->setTitle(QApplication::translate("pdfGuiClass", "Pages", 0, QApplication::UnicodeUTF8));
        menuOpen_recent->setTitle(QApplication::translate("pdfGuiClass", "Open recent", 0, QApplication::UnicodeUTF8));
        menuAbout->setTitle(QApplication::translate("pdfGuiClass", "About", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class pdfGuiClass: public Ui_pdfGuiClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PDFGUI_H
