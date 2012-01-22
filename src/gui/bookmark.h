/** \file bookmark.h Defines class for picking colors */
#ifndef __BOOKMARK__
#define __BOOKMARK__

#include <typedefs.h>
#include <QTreeWidgetItem>
#include <kernel/static.h>
#include <kernel/indiref.h>
#include <kernel/cpdf.h>
#include <kernel/iproperty.h>

/** \brief main class handling analysis */
/** This class handles all properties of the PDF document \n
	It is used for displaying information according to the property it contains. It works in the lazy/loading regime. If user clicks on the item the represents AnalyzeItem, it checks if it is loaded. If it is not, it loads all the properties */
class AnalyzeItem : public QTreeWidgetItem
{
	/**\brief Initializes analysation item */
	/** This method is used usually in constructor \n
		It fill all necessary information needed for lazy-loading
	*/
	void Init();
	/** flag determining if this item has child */ 
	bool _hasChild;
	/** flag determining if this item handles  property of pdf operator */
	bool _isProp;
	/** type of the property. Possible value of the type are: \n
		- pNull \n
		- pString \n
		- pArray	\n
		- pName \n
		- pStream \n
		- pRef \n
		*/
	pdfobjects::PropertyType _type;
	/**pdf that this property belong to */
	PdfInstance _pdfs;
	/** property that should be analyzed */
	/** this is exclusive with the operation analysation */
	PdfProperty _prop;
	/** operator that is to be analyzed */
	PdfOp _op;
	/** flags if this is loaded */
	bool _loaded; //name, type, value, indiref

	/** fully load operator */
	void loadOperator();
	/** fully loads property */
	void loadProperty();
public:
	/** \brief contructor for DOC catalog */
	AnalyzeItem(QTreeWidget * parent, PdfProperty prop);

	/** \brief contructor for Subitems with property type */
	AnalyzeItem(QTreeWidgetItem * parent, PdfProperty prop);

	/** \brief contructor for Subitems with operator type */
	AnalyzeItem(QTreeWidgetItem * parent, PdfOp prop);

	/** \brief Checks if the analyzation item was loaded */
	bool loaded();

	/** \brief Loads analysation item */
	/** This load will later call operator loader or property loader */
	void load();
};

/** \brief class for Bookmark handling */
/** This class is similar to AnalyzationItem. It works in lazy-load regime. When it loads, it contains valid page, zoom and X,Y coordinates. If it contains page=0 it means that the bookmark is invalid and its parent should reload it */
class Bookmark : public QTreeWidgetItem
{
	int _page; 
	bool _hasNext;
	bool _loaded;
	pdfobjects::IndiRef _ref;
	double _y;
	double _x;
	int _zoom;
public:
	/** \brief constructor for main bookmark */
	Bookmark(QTreeWidgetItem * parent): QTreeWidgetItem(parent),_page(0),_loaded(true),_y(0),_x(0),_zoom(1) {} 
	/** \brief Constructor for bookmark in item */
	/** Initializes item to invalid values */
	Bookmark(QTreeWidget * parent): QTreeWidgetItem(parent),_page(0),_loaded(true),_y(0),_x(0),_zoom(1) {} 

	/** \brief gets destination page */
	/** \return page number. If this page is less or equal to zero, the bookmark is invalid */
	int getDest()const;

	/** \brief Sets the beginning of the nej subsection */
	/** Sets the bookmark reference where start its subsection. */
	void setSubsection( pdfobjects::IndiRef ref );

	/** \brief creates new subsection */
	/** After the item was requested for getting bookmark subsection, this method will be called. */
	void addSubsection(QTreeWidgetItem * item);

	/** \brief gets indirect reference to the subsection's first page */
	/** This is here due to PDF specification, every next page is set via indirect reference to pages dictionary */
	pdfobjects::IndiRef getIndiRef()const;

	/** \brief sets the destination page */
	void setPage( int i );

	/** \brief checks if the bookmarks was loaded */
	bool loaded();

	/** \brief sets X coordinate */
	void setX( double t );

	/** \brief sets Y coordinate */
	void setY( double t );

	/** \brief sets zoom */
	void setZoom( double t );

	/** \brief gets X coordinate */
	double getX();

	/** \brief gets Y coordinate */
	double getY();

	/** \brief gets zoom */
	double getZoom();
};

#endif //__BOOKMARK__
