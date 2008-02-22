/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
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
 * File with common configuration for entire program
 * \brief Configuration header
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef CONFIG_DATA_PATH
#error CONFIG_DATA_PATH not defined
#else

/* Ugly macros to force compiler to transform CONFIG_DATA_PATH
 * given by command line define to the string.
 * Note that we can't do
 * #define DATA_PATH S_DATA_PATH(CONFIG_DATA_PATH)
 * directly, because # operator doesn't expand parameter so we
 * would get "CONFIG_DATA_PATH" as the result. So one more level is necessary
 * to force expansion (this is done by CONFIG_PATH.
 *
 * FIXME: consolidate this code - don't use macros if possible
 */
#define S_DATA_PATH(path) #path
#define EXPAND_CONFIG_PATH(path) S_DATA_PATH(path)

/** Directory where all data files of pdfedit would be stored
 *  (icons, default config, etc ...).
 *  Value is set from CONFIG_DATA_PATH configuration value.
 */
#define DATA_PATH EXPAND_CONFIG_PATH(CONFIG_DATA_PATH)

#endif

/** config directory name (will be used relative to $HOME) */
#define CONFIG_DIR ".pdfedit"

#endif

