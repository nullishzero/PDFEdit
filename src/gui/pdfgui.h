/** \file pdfgui.h define main window */
#ifndef PDFGUI_H
#define PDFGUI_H

#include <QtGui/QMainWindow>
#include <QCloseEvent>
#include <QDialog>
#include <QShortcut>
#include "Search.h"

#include "openpdf.h"
#include "ui_pdfgui.h"
//xpdf, pdfedit -> ktovie ci to nema ist do cppka

#include <kernel/pdfedit-core-dev.h>
#include <kernel/cpdf.h>
#include <kernel/cpage.h>
#include <splash/Splash.h>
#include <splash/SplashBitmap.h>	
#include <xpdf/SplashOutputDev.h>
#define NO_CMAP
#include "xpdf/GlobalParams.h"
#undef NO_CMAP

#include "ui_aboutDialog.h"


/** \brief Main window responsible for distributing all signals to class handling PDF */
/** this class is responsible for :
 - correct destroying class handling all pdfs  documents
 - redistribuding all signals from buttons to c;asshandling odfs
 - loading and distributing config files */
class pdfGui : public QMainWindow
{
	Q_OBJECT

	struct pdfedit_core_dev_init init;

	QShortcut * _searchShortCut;
	QDialog aboutDialog;
	Ui::AboutDialog aboutDialogUI;

	/*QCloseWidget _annotationFrame;
	Ui::AnnotationFrame _annotationFrameUI;

	QCloseWidget _imageFrame;
	Ui::ImageFrame _imageFrameUI;

	QCloseWidget _textFrame;
	Ui::TextFrame _textFrameUI;*/
public:
/** \brief constructor */
	/** in this constructor it is loaded configs and distributed values to the appropriate classes*/
	pdfGui(QWidget *parent = 0, Qt::WFlags flags = 0);
	/** \brief Destructor */
	~pdfGui();
	/** \brief closes all other windows */
	/** this means that application is closing, co we can safely write config file \n
		Config file will contain last opened pdf in form FILE=(name of the pdf), \n
		- colors in form hcolor=(color) \n
		- hcolor = (color) \n
		-searchflags = (flags)
	*/
	void closeEvent( QCloseEvent *event );

private:
	Search * _search;
	QMenu * _lastOpenedButtonMenu;
	Ui::pdfGuiClass ui;
public slots:
	/** \brief opens file from the "recent" menu */
	void openLastFile( );
/** \brief this was used with the custom menu, deprecated */
	void lastOpenedPdfs();
	/** \brief adds opened pdf to the recent list */
	void appendToLast( QString s );
	/** \brief method for changing the help row and helpIcon according to the set pdf */
	void handleModeChange( HelptextIcon s );
	/** \brief sets or disables full screen after request */
	void showFullScreened();
	/** \brief check id there is something open. If there is not, disable main panel */
	/** this methid should be calle every time tab is closed */
	void disableMainPanel( int );
};

#endif // PDFGUI_H
