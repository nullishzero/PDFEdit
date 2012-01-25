#ifndef __TYPEDEFS__
#define __TYPEDEFS__

#include <vector>
#include <boost/shared_ptr.hpp>
#include <kernel/pdfoperatorsbase.h>
#include <kernel/pdfoperators.h>
#include <kernel/cannotation.h>

#define BEGIN_ZOOM 50
#define MAX_ZOOM 500
#define ZOOM_STEP 50

#define MODES(XX) \
	XX(ModeDoNothing,"Viewmode ","view.png", "View only")	\
	XX(ModeInsertText,"InsertText mode : click on pdf to insert a text","insert_text.png","Insert")	\
	XX(ModeSelectText,"Select text mode : Click and drag to select a text","hand.png","Select") \
	XX(ModeInsertImage,"InsertImage mode: Select a rectangle to insert an image","addImage.png","Insert") \
	XX(ModeSelectImage,"Select image mode: Click to select inline image","hand.png","Select")	\
	XX(ModeImageSelected, "Image selected. Click on buttons to change or to delete image","hand.png","Select")	\
	XX(ModeImagePartCopied,"Snapshot was copied to the clipboard","snapshot.png","Snapshot")	\
	XX(ModeImagePart,"Extract pdf part as image mode: select a rectangle to be saved as image","snapshot.png","Snapshot")	\
	XX(ModeOperatorSelect,"Operator select mode: click on text operator to highligh it","operator.png","Operators")	\
	XX(ModeInsertAnnotation,"Insert Text Annotation mode: click on pdf to insert an annotation","addComment.png","Insert comment")	\
	XX(ModeInsertLinkAnnotation,"Insert Link Annotation mode: click on pdf to insert a link annotation","addLink.png","Insert Link")	\
	XX(ModeDeleteAnnotation, "Delete annotation mode: click on annotation to delete it","DeleteText.png","Delete annotation")	\
	XX(ModeEmitPosition, "Emit position mode: Pdf is waiting for position. Click somewhere in the pdf","position.png","Wait for position")	\
	XX(ModeSettings,"Set the color that should be used when highlighting text by hand and by annostion. Press setting to get away","settings.png","Settings")\
	XX(ImageMode," Not Implemented","unknown.png","")	\
	XX(DrawMode,"NotImplemented, go away!","unknown.jpg","")	\
	XX(ModeFindLastFont, "Click to select font and font size by text in PDF","font.png","Fetch Font")\
	XX(ModeChangeAnnotation, "Click on anotation to change it","change.png","Change annotation") \
	XX(DefaultMode,"Open an PDF","unknown.jpg","") \
	XX(ModeHighlighComment,"After selecting text, fill please content","highlight.png","Highlight") \
	XX(ModeDeleteHighLight, "Click to delete highlight annotation","deleteText.png","Delete highlight")\
	

#define ENUMS(a,b,c,d) a,

#define EARRAY(a,b,c,d) { b,c,d },

enum Mode
{
	MODES(ENUMS)
	NModes
};
enum PageDrawMode
{
	ModeDrawHighLight,
	ModeDrawPosition,
	ModeTrackCompleteRectangle,
	ModeDrawNothing,
	ModeTrackDrawingRect
};


enum SearchFlags
{
	SearchForward = 1,
	SearchCaseSensitive = 2,
	SearchConcate = 4,
	SearchRegexp = 8
};

typedef boost::shared_ptr<pdfobjects::CAnnotation> PdfAnnot;
typedef boost::shared_ptr<pdfobjects::CPdf> PdfInstance;
typedef boost::shared_ptr<pdfobjects::IProperty> PdfProperty;
typedef std::vector<boost::shared_ptr<pdfobjects::PdfOperator> > Ops;
typedef std::list<boost::shared_ptr<pdfobjects::PdfOperator> > OpsList;
typedef boost::shared_ptr<pdfobjects::PdfOperator> PdfOp;
typedef pdfobjects::PdfOperator::BBox BBox;
typedef boost::shared_ptr<pdfobjects::UnknownCompositePdfOperator> PdfComp;

#endif // __TYPEDEFS__
