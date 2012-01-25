/** \file openpdf.h Defines class for handling all pdfs */
#pragma once
#include <QTabWidget>
#include <QString>
#include <QTimer>
#include "typedefs.h"

/** \brief structure for holding all necessary help indices */
/** this structure is emitted every time  mode is changed. It containt all necessary information mode change \n
 - help text that is possible to show in help row
 - icon to be shown in actual mode
 - descripion as a text under actuaIcon */
struct HelptextIcon
{
	/// text shown in help row
	const char * helpText;
	/// shown icon
	const char * icon;
	/// description of the actual button
	const char * description;
};
/** \brief main class handling multiple opened documents */
/** this class contains and handles multiple opened pdfs in tabWidget tree */
class OpenPdf :
	public QTabWidget
{
	Q_OBJECT
	

public:
	/** \brief constructor */
	OpenPdf(QWidget * widget);

	/** \brief descturor*/
	~OpenPdf(void);

	/** \brief Opend another pdf */
	/** this is used internaly as a result action of some slot */
	void open(QString s);

	/** previous mode. It is useful for teporary mode like ModeEmitPosition */
	Mode _previous;
	/** actual mode */
	Mode _mode;
	/** saved color */
	QColor _color;
	/** saved highlight color */
	QColor _highlightColor;
	/** owner of the account. Its name will be automatically filled within annotation etc. */ 
	std::string _author;

public:
	/** \brief sets mode */
	/** Sets and emits code about change. */  
	void setMode(Mode mode);
	/** \brief gets actual mode */
	Mode getMode() const { return _mode; }

signals:
	/** \brief request for actual highlight signal */
	QColor GetActualHColorSignal();
	/** \brief request for actualselecte text signal */
	QColor GetActualColorSignal();
	/** \brief request for actual highlight signal */
	/** All the widgets can catch the sognal and pick the information */
	void ModeChangedSignal(HelptextIcon);
public slots:
	/** \brief inita abaluzation tree */
	/** load docCatalog into analyzation Item . All other analyzation things will be lazy -load only after request */
	void initAnalyze();
	/** \brief reaction on search sognal from main window */
	/** re-called to child */
	void search(QString s, int flags);
	/** re-called to child for deleting selected text */
	void deleteSelectedText();
	/** \brief delegated to child widget for erasing selected text */
	void eraseSelectedText();
	/**\brief delegates to child widget chor changing text */
	void changeSelectedText();

	/** \brief sets highlight annotatinon for selected */
	void highlightSelected();

	/** \brief set mode for selecting text operators */
	void setModeOperator();

	/** \brief sets viewMode */
	void setModeView();
	/** \brief sets mode for changing annotatinon */
	void setModeChangeAnnotation();
	/** \brief sets mode for highlighting comment without raising dialog */
	void setHighlighCommentText();
	/** \brief sets mode for snapshot */
	void setModeImagePart();
	/** \brief sets mode for inserting text */
	void setModeInsertText();
	/** \brief sets mode for selecting text */
	void setModeSelectText();
	/** \brief sets mode for inserting image */
	void setModeInsertImage();
	/** \brief sets defailt (view) mode */
	void setModeDefault();
	/** \brief sets mode for selecting  annotation */
	void setModeSelectImage();
	/** \brief sets mode for inserting annotation */
	void setModeInsertAnotation();

	/** \brief delegates to child widget for changing selected image */
	void changeSelectedImage();

	/** \brief delegates to child widget for deleting selected image */
	void deleteSelectedImage();
	/** \brief saves in the encoded state */
	void saveEncoded();

	/// \brief extracts text
	void getText();
	/** \brief rotate page in the clock-wise direction */
	void derotate();
	/// \brief rotates active page
	void rotate();

	/// \brief Opens another pdf
	void openAnotherPdf();

	/// \brief deletes tab when closed
	void closeAndRemoveTab(int);

	///saves under original name
	void save();

	///save unser another name
	void saveAs();

	///move the actual page up
	void pageUp();

	///moves the actual page down
	void pageDown();

	///insert range from another pdf
	void insertRange();

	///inserts empty page
	void insertEmpty();

	/** \brief makes the page stop searching */
	void stopSearch();

	/** \brief this is not used */
	void print();
	/** \brief delegates to the child widget */
	void deletePage();

	/** \brief sets mode for deleting only highligh annotation */
	void setModeDeleteHighLight();
	
	/** \brief sets mode for deleting annotation */
	void setModeDeleteAnnotation();

	/** \brief of pdf was changed, adds a star to its name */
	void pdfChanged();
	
	/** \brief redraws actual page */
	void redraw();
	
	/** \brief sets previous mode */
	void setPreviousMode();
	
	/** \brief sets color for select operation*/
	/** this will be mandatory for all subwidgets */
	void setColor(QColor);
	/** \brief sets color for highlight annotation */
	void setHColor(QColor);
	/** \brief get color that should be used when drawing something */
	QColor getColor();
	/** \brief get global color for highlighting annotations */  
	QColor getHColor();
	/** \brief sets mode for inserting link annotation  */
	void setModeInsertLinkAnotation();
	/** \brief sets setting mode */
	void setModeSetting();
	/** \brief re-call to the child widget */
	/** handling request for raising dialog window with information about PDF */
	void about();
	/** \brief re-call to the child widget */
	/** asks actual tabPage aout extracting image */
	void extractImage();

signals:
	/** \brief this is used for emitting history */
	/** every time a file is successfuly opened, it will be added to the recent list*/
	void OpenSuccess(QString);
public:
	/// loginn namethat should be used for annotation
	std::string Author()const;
	/** \brief this check if all opened pdfs were saved before closing */
	/** It there is document that was not closed, the pop-up dialog will as user about what to do */
	void checkClose();
	//void resizeEvent(QResizeEvent *event);
};
