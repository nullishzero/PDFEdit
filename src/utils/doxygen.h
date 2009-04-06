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
// This header file is not included anywhere,
// but it is parsed by doxygen, so it is used to add documentation

/**
 \mainpage PDF Editor programmer documentation

 \section intro_sec Introduction

 PDF Editor consists of two logical parts - Kernel and GUI.<br>
 Both use a set of utility classes.

 \section kernel_sec Kernel

 Kernel part is responsible for PDF file manipulation and provides API that is used by GUI.

 \section gui_sec GUI

 Graphical user interface.

 \section util_sec Utility classes

 Various utility classes
 <ul> 
 	<li>configuration parser - </li>
	<li>delinearizator - </li>
	<li>object storage - </li>
	<li>iterator - implementation of iterator design pattern</li>
	<li>debugging</li>
</ul>
  
*/

//Namespace documentation

/**
 GUI namespace<br>
 This namespace contain all user-interface related classes
 (widgets, tree items, scripting wrappers and others)
 \brief Graphical user interface namespace
*/
namespace gui {}
/**
 Utility namespace<br>
 Contains various static utility functions, used in GUI
 \brief Utility namespace
*/
namespace util {}

/** Namespace for configuration specific stuff. 
 */
 namespace configuration {}

/** Namespace for debug functions and related stuff. 
 */
 namespace debug {}

/** TODO namespace filters       */
 namespace filters {}
/** TODO namespace iterator      */
 namespace iterator {}
 
/** Namespace for generic observer stuff.
 */
 namespace observer {}
 
/** Namespace for rules manager generic classes.
 */
 namespace rulesmanager {}

/** TODO namespace xpdf          */
 namespace xpdf {}
