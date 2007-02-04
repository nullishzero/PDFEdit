/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __BASECONSOLE_H__
#define __BASECONSOLE_H__

#include "base.h"

namespace gui {

class ConsoleWriterConsole;

/**
 Class that host scripts and contain static script functions<br>
 This class is also responsible for garbage collection of scripting
 objects and interaction of editor and scripts
 \brief Script hosting base class
*/
class BaseConsole : public Base {
 Q_OBJECT
public:
 BaseConsole(const QStringList &_params);
 virtual ~BaseConsole();
 void runInitScript();
public slots: //This will be all exported to scripting
 /*- 
 Terminate the application. You can specify return code of application in parameter
 */
 void exit(int returnCode=0);
 /*-
  Return list of commandline parameters
  (excluding any possible switches processed by pdfeditor itself)
 */
 QStringList parameters();
 /*-
  Return first parameter from list of parameters
  and remove it from the list. Other parameters are shifted to take the empty space.
  If there is no parameter to take, NULL is returned
 */
 QString takeParameter();

/** Function return X position in pdf page from position of viewed page on screen (unapply rotation, viewing dpi, ...)
 @param fromX	X position on viewed page.
 @param fromY	Y position on viewed page.

 @return Return X position in pdf page.

 @see convertPixmapPosToPdfPos_y
 @see convertPixmapPosToPdfPos
 @see convertPdfPosToPixmapPos
*/
double convertPixmapPosToPdfPos_x ( double fromX, double fromY );
/** Function return Y position in pdf page from position of viewed page on screen (unapply rotation, viewing dpi, ...)
 @param fromX	X position on viewed page.
 @param fromY	Y position on viewed page.

 @return Return Y position in pdf page.

 @see convertPixmapPosToPdfPos_x
 @see convertPixmapPosToPdfPos
 @see convertPdfPosToPixmapPos
*/
double convertPixmapPosToPdfPos_y ( double fromX, double fromY );
/** Function return X position on viewed page from position in pdf page (apply rotation, viewing dpi, ...)
 @param fromX	X position in pdf page.
 @param fromY	Y position in pdf page.

 @return Return X position in pdf page.

 @see convertPdfPosToPixmapPos_y
 @see convertPdfPosToPixmapPos
 @see convertPixmapPosToPdfPos
*/
double convertPdfPosToPixmapPos_x ( double fromX, double fromY );
/** Function return Y position on viewed page from position in pdf page (apply rotation, viewing dpi, ...)
 @param fromX	X position in pdf page.
 @param fromY	Y position in pdf page.

 @return Return Y position in pdf page.

 @see convertPdfPosToPixmapPos_x
 @see convertPdfPosToPixmapPos
 @see convertPixmapPosToPdfPos
*/
double convertPdfPosToPixmapPos_y ( double fromX, double fromY );
private:
 /** Console writer class writing to STDOUT */
 ConsoleWriterConsole* consoleWriter;
 /** Commandline parameters */
 QStringList params;
};

} // namespace gui

#endif
