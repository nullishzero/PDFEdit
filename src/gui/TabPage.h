/** \file tabPage.h defines main class for handling operation with pdf */
#ifndef __TABPAGE__
#define __TABPAGE__

#include <qwidget.h>
#include <QString>
#include <QFileDialog>
#include <QRect>
#include "ui_Tab.h"
#include "page.h"
#include "fontWidget.h"
#include <list>
#include <QTreeWidgetItem>
#include <QTextEdit>
#include "typedefs.h"
#include "debug.h"
#include "Search.h"
#include "comments.h"
#include "tree.h"
#include "insertImage.h"
#include "bookmark.h"


//xpdf, pdfedit -> ktovie ci to nema ist do cppka
#define NO_CMAP
#include "xpdf/GlobalParams.h"
#undef NO_CMAP

#include <kernel/pdfedit-core-dev.h>
#include <kernel/cpdf.h>
#include <kernel/cpage.h>
#include <splash/Splash.h>
#include <splash/SplashBitmap.h>	
#include <xpdf/SplashOutputDev.h>
#include <kernel/factories.h>
#include <kernel/displayparams.h>
#include <kernel/pdfoperatorsbase.h>

//END of PDF
using namespace boost;
using namespace pdfobjects;

//co spravit, ked prepnem na inu stranku


#include "operatorData.h"
#include "ui_aboutDialog.h"

typedef std::list<OperatorData> TextData;

enum AcceptName
{
	OpFontName,
	OpTextName,
	OpImageName,
	OpGraphicName,
	OpAcceptCount
};
/** \brief structure with operator names thast should be accpted for operation */
struct AcceptOperatorName
{
	/// names of the operators
	std::vector<std::string> names;
	/** \brief checks if name is wanted operator name */
	bool isType(std::string name)
	{
		for (size_t i =0; i < names.size(); i++)
		{
			if (name == names[i]) //operator name
				return true;
		}
		return false;
	}
	/** \brief adds name to the recognized operotor name */
	void add(std::string name)
	{
		names.push_back(name);
	}
};
/** \brief helper class handling all operators that TIAEditor can handle */
class IsType
{
	AcceptOperatorName names[OpAcceptCount];

public:
	/** \brief constructor */
	IsType()
	{
		names[OpFontName].add("Tf");
		names[OpFontName].add("tf");
		names[OpTextName].add("TJ");
		names[OpTextName].add("Tj");
		names[OpTextName].add("'");
		names[OpTextName].add("\"");

		names[OpImageName].add("BI");
		/*names[OpImageName].add("ID");
		names[OpImageName].add("Id");*/

		names[OpGraphicName].add("DO");
		names[OpGraphicName].add("Do");
	}
	/** \brief checks if n is accepted operator name accortding to the type */
	bool isType(int type, std::string n)
	{
		return names[type].isType(n);
	}
	/** \brief checks if the application can handle operator with this name */
	bool acceptType(std::string name)
	{
		for ( int i =0 ; i < OpAcceptCount; i++)
		{
			if (isType(i,name))
				return true;
		}
		return false;
	}
};
/** \brief annotation that can be handled by TIAEditor */
enum AnnotType
{
	LinkAnnot,
	TextAnnot,
	HighLighAnnot,
	StrokeAnnot
};
class OpenPdf;

/** \brief main class for handling one concrete opened pdf  document*/
class TabPage : public QWidget
{
	Q_OBJECT

private: //variables
	/** this variable is used when the should be new page visible but still some operation is pending in the old page. It is used for creating link annotation */
	int _oldPage;
	/** thread that start when request of search is handled */
	QThread * _searchThread;
	/** flag determining when searching should stop. User interrupts */
	int _stop;
	/** output device for PDF document */
	SplashOutputDev splash;
	/** what type should operator mode accept */
	int _acceptedType;
	/** ownet of this class */
	OpenPdf * _parent;
	/** generated user interface */
	Ui::TabUI ui; 
	/** Form that will pop-up when signal for inserting or changing image is set */
	InsertImage * _image;
	/** position os the mouse first time it was clicked */
	QPoint _mousePos;
	/** searching engine. It contains all the flags and work that we look for. It need to be constantly fed by tokens */ 
	Tree _searchEngine;
	/** form that will raise up when request for handling annotation is caught */
	Comments * _cmts;

	/** name of the opened file */
	QString _name; 
	/** form that will raise up when request for handling will be caught */
	FontWidget * _font;	
	/** checker for type that TIAEditor can handle */
	IsType typeChecker;
	/** parameters that changes the way the final pdf is disaplyed, like zoot etc */
	pdfobjects::DisplayParams displayparams;	
	/** pdf that is openened */
	boost::shared_ptr<pdfobjects::CPdf> _pdf;
	/** visible page */
	boost::shared_ptr<pdfobjects::CPage> _page;
	/** container with all ordered text tokens */
	TextData _textList;
	/** iterators the border the selection */
	TextData::iterator sTextIt, sTextItEnd, sTextMarker; //kde ten iterator konci
	/** flags that tells it there was fulfilled all condition for continueing the operation. For example if after draggon mouse in image wade it was clicked and aproppriate variable set */
	bool _dataReady;
	/** flag that determines if there was something selected */
	bool _selected;
	/** inline image operator that was selected */
	PdfOp _selectedImage;

	/** user interface of dialog about PDF */
	Ui::AboutDialog aboutDialogUI;
	/** widget handler for displaying info about PDF */
	QDialog aboutDialog;
	/** accepted annotation */
	CPage::Annotations _annots;
	/** widget responsible to display pdf */
	DisplayPage * _labelPage;
	/** fills coordinated that shouls be actine in annotation */
	void fillCoordinates(std::vector<float>& coordinates, float * dim);
	/** inits revision from the scratch */
	void initRevisions();
	
	/** \brief return last td operator that applied to the parameter */
	PdfOperator::Iterator findTdAssOp(PdfOperator::Iterator iter);
	//float findDistance(std::string s,TextData::iterator textIter);
	/** Set next page. If the page is last, go to the first */
	void SetNextPageRotate();
	/* vytvorit textovy list */
	/** fill ops with the operators at the given position */
	void getSelected(int x , int y, Ops ops);
	/**changes position from pixmap position to pdfposition */
	void toPdfPos(int x, int y, double & x1, double &y1);
	/** reverse to toPdfPos */
	void toPixmapPos(double x1, double y1, int & x, int & y);
	/**moves the iterator forward or backward */
	void inDirection(TextData::iterator & it, bool forw);
	/** check if the operator is really first and switches them if it is not so */
	void setSelected(TextData::iterator& first, TextData::iterator& last);
	/** load all supported annnotation */
	void showAnnotation();
public:
/// raise searching thread
	bool performSearch(QString srch, bool forw);
	/// link annotation that should be inserted ( can be from another page )
	PdfAnnot _linkAnnot;
	//static std::string SupportedAnnotations[] = { ANNOTS(CREATE_ARRAY) };
///
	/** if image was selected, it will be deleted */
	void deleteSelectedImage();
	/** show form about changing image */
	void raiseChangeSelectedImage();
	/** recreates list of all text operators and sorts the according to left uppert corner */
	void createList();
	/** creates annotation aroud selected text */
	void highLightAnnSelected();
	/** delected annnptation */
	void delAnnot(int i); //page to u seba upravi, aby ID zodpovedali
///constructor	
	TabPage(OpenPdf *,QString name);
	///destructor
	~TabPage(void);
///sets initializes bookmark according to dictionary
	void setTree(shared_ptr<CDict> d, Bookmark * b );
	///start highlightin - no longer used
	void highLightBegin(int x, int y); //nesprav nic, pretoze to bude robit mouseMove
	///highlights selected text
	void highlightText(); //tu mame convertle  x,y
/** inserts ioperator before another. PdfEdit kernel does not support this, so this method replaces the operator and inserts clone after replaced operator */
	void insertBefore(PdfOp op, PdfOp before);
	/// this was not used
	void createAddMoveString(PdfOp bef, double x, double y, QString name);
	/// deprecated. Thia was moved to FontWidget 
	void insertTextAfter(PdfOp opBehind, double td, double ymax, QString s);
private:
///loads object at specified position according to displayparameters actually used 
	void getAtPosition(Ops& ops, int x, int y );
	/// validates that begin is before end 
	void setTextData(TextData::iterator &begin, TextData::iterator end, shared_ptr<PdfOperator> op);
	///deletes selected text 
	void deleteText( QString text);
	//convert BBox to QRect according to display parameters actually used 
	QRect getRectangle( PdfOp ops );
	//convert qrect to BBox according to display parameters actually used 
	QRect getRectangle( BBox box );

	///adds another revision number to the selectable list 
	void addRevision( int i = -1);
	
	/** sychronizes page number an page count ont he screen with the one in PDF */
	void updatePageInfoBar();

	/// gets file, name is name of dialog
	QString getFile(bool open,QFileDialog::FileMode flags = QFileDialog::AnyFile);

	/** shows text operator at the selected position */
	void showClicked(int x, int y);
public:	
	/// performs delinearization
	void delinearize(QString name);
	/// check if this PDF can be saved
	bool CanBeSaved(bool raisew = true);
	/// check if this pdf is not in read=-only mode or delinearized
	bool CanBeSavedChanges(bool raiseW = true);
	/// completely redraw page. This must be performed when something is inserted or deleted. Those operation could change while PDF display
	void redraw();
	///load first level pof bookmarks
	void getBookMarks(); //LATER, treba actions zisti, ako sa vykoavaju
	///handles going to another page
	void wheelEvent( QWheelEvent * event ); 
	///deletes actual page 
	void deletePage();
	///saves pdf to the new file. If no name is specified, all changes are saved to new hidden revision
	void savePdf(char * name);
	///checks if this file is linearized */
	bool checkLinearization();
	/// checks if this pdf contains certain operator. Used for checking TJ operator
	bool containsOperator(std::string name);
	//rotate page

public slots:
	///move page up
	void pageUp();
	///move page down
	void pageDown();
/// find tha last font in position
	PdfOp getPreviousFontInPosition(libs::Point pdfPos);
	/// sets mode that will wait for position
	void SetModePosition(PdfAnnot a);
	///show contennt of the annotation that is active ( there is cursor )
	void showAnnotation(int i);
	/// save pdf with hidden revision
	void save();
	/// save to another copy
	void saveAs();
	/// save in decoded state
	void saveEncoded();
	/// clear selected image and selected text
	void clearSelected();
	///not used. Text operator are set by default when in operator mode
	void setTextOperator();
	///not used
	void setImageOperator();
	/// gets font from page. If there is no such page, is will try to create one
	std::string addFontToPage(std::string id);
	/// not used
	void replaceText( QString what, QString by);
	/// handle operator with changes selected image
	void changeSelectedImage(PdfOp op);
	//void raiseSearch();
	/// not used
	void closeAnnotDiag();
	/// get changes text
	void changeSelectedText(PdfOp);
	/// delte text an move accordingly
	void deleteSelectedText();
	/// fill text with blans space
	void eraseSelectedText();
	///not implemented
	void replaceSelectedText(QString by);
	///insert image
	void insertImage(PdfOp op);
	/// handle vent when mouse was released
	void mouseReleased(QPoint); //nesprav nic, pretoze to bude robit mouseMove
	///insert higligh annotation, create appropriace appearance stream
	void insertTextMarkup(PdfAnnot annot);
	//void waitForPosition(); //nastao stav taky aby emitovala aktualne kliknitu poziciu
	/// insert annotartion to the page
	void insertAnnotation(PdfAnnot a);
	//void deleteAnnotation(QPoint);
	///search according to the flags
	void search(QString text,int flags);
	/// do to the bookmark page
	void handleBookmark(QTreeWidgetItem* item, int);
	/// handle vlick event
	void clicked(QPoint point);
	/// there was created valid pdf operator, it must be inserted into page
	void insertText( PdfOp op );
	/// raise insert Text dialog
	void raiseInsertText(QPoint);
	/// raise change selected text dialog
	void raiseChangeSelectedText();
	/// raise insert image dialog
	void raiseInsertImage(QRect);
	/// raise annotation dialog for inserting comment
	void raiseAnnotation(QPoint point);

	/// deletes image at selected point 
	void deleteImage(QPoint point);
	///Sets image to previous page
	bool previousPage();
	///Sets image to next page
	bool nextPage();

	///insert range
	void insertRange();

	/// Adds empty page
	void addEmptyPage();

	///prints pdf. this was not implemented due to pdf print specification
	void print();

	/** exports text to the chosen file & opens that file ( txt ) in view */
	void exportText();

	/** rotates page according to angle */
	void rotate(int angle);

private slots:
	/// there was clicked on the annotationlink. Link must be loaded and action performed */
	void handleLink( int annot );
	/// set zoom
	void zoom(int zoomscale);
	/// init pdf-reader to have this revision
	void initRevision(int revision);

	/// Inserts range of file from existing PDF
	void insertPageRangeFromExisting();

	/// Saves actual made changes to new revision
	/** Nothing else happens, no need to  
	 */
	void commitRevision();

	/// Saves revision-specific pdf to new pdf
	void exportRevision();
	/// loads bookmar
	void loadBookmark( QTreeWidgetItem * item );
	/// insert text annotation to page 
	void insertTextAnnot(PdfAnnot a);
	/// set mode to the one that allows findinf font in page 
	void findLastFontMode();
	/// check if this string can be displayed with this font 
	GlyphInfo checkCode(QString s, std::string fontName);
	void getPreviousTmInPosition( libs::Point p, float* size);
	void checkLoadedBookmarks();

signals:
	/// signal for creating varnings
	void SetStateSignal(QString);
	/// signal to information about operation to the history */
	void addHistory(QString);
	/// sets new mode to the page 
	void ChangePageModeSignal(PageDrawMode);
	///marks positon in PDF for inserting page
	void markPosition(QPoint point); //reverted point
	///position for comments are ready to be set when this signal is emitted 
	void parsed(std::vector<float>);
	///	void pdfPosition(float a, float b, int w,int h);
	void pdfText(std::string s);
private:
	/// get destination array from name tree
	void getDest( const char * nameToResolve, Bookmark *b ) ;
	/// when loading bookmark, get destination
	void getDestFromArray( PdfProperty pgl, Bookmark * b );
	//redraw all page 
	void JustDraw();
	/// creates appearance stre
	boost::shared_ptr<pdfobjects::CStream> createAPStream(float * dim);
	///creates stream for highlight 
	pdfobjects::IndiRef createAppearanceHighlight(float * dim);
	///create stream for comment
	pdfobjects::IndiRef createAppearanceComment(float *dim);
	/// from the bunch of operator gets valid text operator 
	PdfOp getValidTextOp( Ops& ops, bool & found);
public slots:
	/// return actual used params
	DisplayParams getDisplayParams();
	///copy text to the clipboard
	void copyTextToClipBoard();
	/// remove all remains from all operations from the page
	void operationDone();
	///init analyzation item 
	void initAnalyze();
	/// load analyze t=item that was clicked 
	void loadAnalyzeItem( QTreeWidgetItem * item );
	/// load first page 
	void setFirstPage();
	/** load last page */
	void setLastPage();
	/// set the information page
	void setPageFromInfo();
	/// raise "about dialog "
	void about();
	/// increate zoom
	void addZoom();
	/// decrase zoom 
	void minusZoom();
	/// stops search 
	void stopSearch();
	/** check the result of the search, since the search in in separate thread that does not allow to report correctly, we are connecting slot for finished with this */ 
	void reportSearchResult();
public:
	/**check iif the odf was changed */
	bool changed();
	/** hande resize event */
	void resizeEvent(QResizeEvent * event);\
	/** highlight selected text */
	void highlight(); 
	/** setPage according to the index */
	void setPage(int index);
	/** sets the warning messages */
	void setState();
	/** estract selected image */
	void extractImage();
	/** get full name of this PDF */
	QString getName();
};

#endif