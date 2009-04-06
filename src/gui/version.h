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
/** @file
 This file hold constants defining version of application.
 These should be changed before release
 \brief Version information
 */
#ifndef __VERSION_H__
#define __VERSION_H__

// WARNING: DO NOT INCLUDE from header files visible outside gui code!!!

/** Application name shown in various About boxes, window titles, etc .... */
#define APP_NAME "PDFedit"

// It is not a macro, because we want to provide this information only 
// during link time to prevevent from many modules rebuilding when we
// change the version string.
/** Version of application. 
 * This will also include release (if defined). 
 */
extern const char *PDFEDIT_VERSION; 

/** Compile time of this program */
#define COMPILE_TIME (__DATE__ ", " __TIME__)

#endif
