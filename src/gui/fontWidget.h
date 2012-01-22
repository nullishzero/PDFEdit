/** \file fontWidget.h that is responsible for correct inserting and changing values */
#ifndef __FONTE_WIDGET__
#define __FONTE_WIDGET__

#include "ui_properties.h"
#include <QWidget>
#include <QString>
#include <vector>
//PDF
#include <kernel/pdfedit-core-dev.h>
#include <kernel/factories.h>
#include <kernel/pdfoperators.h>
#include "typedefs.h"
//misc

/** \brief temporary structure for glyph checking */
/** This structure is used when application is checking if font is able to draw the character */
struct GlyphInfo
{
	/** string that should be inserted */
	std::string name;

	/** size that would occupy the font in pdf units */
	float size;
};

/** \brief temporarily structure used for saving font names */
/** This structure is used for storing the names of the fonts together with it representation inf the PDF document */
class TextFont
{
	std::string _name;
	std::string _fontId;
public:
	/** \brief Contructor */
	TextFont(std::string name) : _name(name),_fontId("") {}
	/** \brief creates valid font operator with the defined size */
	/** this operator will be valid only for PDF that created the name for it \n
	 name must not be empty */
	boost::shared_ptr<pdfobjects::PdfOperator> getFontOper(int size)
	{
		assert(!_fontId.empty());
		pdfobjects::PdfOperator::Operands fontOperands;//TODO check poradie
		fontOperands.push_back(boost::shared_ptr<pdfobjects::IProperty>(new pdfobjects::CName (_fontId)) );
		fontOperands.push_back(boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CRealFactory::getInstance(size)));//velkost pismeno
		return createOperator("Tf", fontOperands);
	}
	/** \brief get recognizable name - for example Times New Roman */
	/** this is the name that is user- friendly - has recognizable name. This name can be different for the same fonts in the differemt PDF pages */
	std::string getName() const{ return _name; }

	/** \brief sets Id of the font. This id is then used for createing valid operator */
	void setId(std::string id) { _fontId = id; } 
};

/** \brief main class for creating and changing  text operator located in PDF document. */
class FontWidget : public QWidget
{
	Q_OBJECT

	enum Shapes
	{
		fill,stroke,both,invisible,NumberOfShapes
	};
	///indicated if the added font in embedded
	bool _embededFont;
	//indicates if the 
	bool _change;
	//bool set[NumberOfOptions];
	Ui::Properties ui;
	std::vector<TextFont> _fonts;
	PdfComp _q;
	PdfComp _BT;
	float _pdfPosX,_pdfPosY;
	float _scale[2];
	PdfOp createMatrix(std::string op);
	QPalette palette;
protected:
	/** \brief sets default title*/
	/** when widget is in changing mode, title should be set separatedly */
	void showEvent(QShowEvent *);

	/** \brief creates the correct PDFoperator */
	/** this method will check all thenecessities like correct string to input. It emits the the signals to check out is insertin operator will be vald -> fint will shou at leaset one characted. I t return empty operator in case something wen wrong */
	PdfOp addText(QString s);
public:
/** \brief this was indesded to changed visual style of QWidget, however it was not used */
	void paintEvent(QPaintEvent * event);
	/** \brief adds font heigth to the combobox */
	/** this method is called mainly when choosing font from page */
	void setHeight(float h)
	{
		QVariant data(h);
		data=data.toInt();
		this->ui.fontsize->setCurrentIndex(this->ui.fontsize->findData(data.toInt()));
	}
	/** \brief gets text as user set it */
	QString getText();

	/** \brief creates translation text matrix */
	static PdfOp createTranslationTd(double x, double y);

	/** \brief sets text tha should be replaced */
	void setText(QString s);
	//void reset();
	/** \brief constructor */
	FontWidget(QWidget * parent);
	/** \brief Desctructor */ 
	~FontWidget();

	/** \brief created net BT operator to the existing q operator */
	/** the q operator that it append to, must be created before this method is called */
	void createBT();

	/** \brief created all set parameters */
	/** \return id of the font that was selected. This name will be later used to determine string that will ne shown in the PDF */
	std::string addParameters();

	/** \brief add to BT operator */
	/** \param op operator to be added to the BT */
	void addToBT(PdfOp op);

	/** \brief ends BT operator */
	/** create BT method must be called before this one. Otherwise the structure will get corrupted */
	PdfOp createET();

	/** \brief adds font to the font list according to the name*/
	/** this method will create some TextFont data. Id tod the data is at first equal to the name */
	void addFont(std::string name, std::string value);

	/** \brief adds font to the font list according to the pdfoperator */
	/** font will be extracted from the operator and added correct TextFont data with the ID set */
	void addFont(PdfOp op);
	/** \brief get rotation as was set */
	int getRotation();
	/** \brief gets scale as it was set */
	int getScale();
	/** \brief get color of the font */
	QColor getColor();

	/** \brief gets X position of the inserted text */
	int getX();

	/** \brief gets Y position of the inserted text */
	int getY();

	/** \brief emits change selected signal */
	void change();

	/// creates TM from page. this is used when getting font from page
	void addTm( float w, float h );

signals:
	/** \brief emits closed signal */
	void FontClosedSignal();

	/** \brief emits request for last Tm at given position */
	/** \param p is point from the the search should begin 
		\param size will after successful perform contain information about width and height of the found TH.*/
	void getLastTm(libs::Point p,float * size);

	/** \brief checks the text againt requested font */
	/** \return unformation about string that can be displayed */
	GlyphInfo convertTextFromUnicode(QString, std::string);
	
	/** \brief signal that signifies that part of the text that was requested to ched, is changed */
	void changeTextSignal(PdfOp op);

	/** \brief signal that is emitted after valid operator is created */
	void text(PdfOp op);
	//void changeSelected();
	/** signal that reguests last font from given point */
	PdfOp getLastFontSignal(libs::Point);
	/** \brief checks inf the font is in page */
	/** if the font is not in page, it is added to page and it representation id is returned */
	std::string fontInPage(std::string id);

	/** \brief requests font from page */
	/** this count on that if any widget catches this, it will later emit the fignal with the font */ 
	void FindLastFontSignal();

public slots:
	/** \brief sets the state to insert state */
	void setInsert();
	
	/** \brief sets the state to change state */
	void setChange();

	/** \brief sets position */
	/** parameters are inf the PDF  units */
	void setPosition(float pdfx, float pdfy);
	
	//void setTm() { set[OptionTm] = true; }
	//void setFont() { set[OptionFont] = true; }
	//void setShape() { set[OptionShape] = true; }
	//void setGray() { set[OptionGray] = true; } //TODO macro

	/** \brief this method will insert or change the text and closes the window */
	void apply(); //on clicked
	/** \brief inserts this operators to new BT with new parameters */
	/** \note this method was not tested, so it is nore released */
	void createFromMulti( std::vector<PdfOp>& operators );
	/** \brief reaction to angle change */
	/** if angle is changed, the value should show in the label the to the widget */  
	void sliderChanged(int value);
	/** \brief clears all fonts and sets them from scratch */
	/** some fonts are present only on one particular page. We have to clrear all the fonts every time this widget will be used */
	void clearTempFonts();

	/** \brief sets this widget to the state that waits only fot font */
	/** this method will disable clear templ when this widget will show up */
	void waitForFont( );
	/** \brief cleans after everything was ionserted and done */
	/** after close event, all temporary folders will be delete */
	void closeEvent ( QCloseEvent * event );

	/** \brief checks which color should be visible */
	void setDrawType( int index );
	//void setAngle(int angle);
};

#endif
