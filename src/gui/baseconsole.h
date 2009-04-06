/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
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
