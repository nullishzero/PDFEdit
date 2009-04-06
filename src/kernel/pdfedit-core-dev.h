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
#ifndef __PDFEDIT_CORE_DEV_H__
#define __PDFEDIT_CORE_DEV_H__

#include "xpdf/GlobalParams.h"

/**
 * @param cfgFileName Configuration file name used by xpdf core for parameters
 * 	initialization.
 * @param fontDir Directory with fonts used by xpdf core.
 */
struct pdfedit_core_dev_init
{
	const char *cfgFileName;
	const char *fontDir;
};

/** Initialization routine for pdfedit_core_dev library.
 * @param argc Pointer to the argv elements count (ignored if NULL).
 * @param argv Pointer to arguments array (ignoder if NULL).
 * @param init Additional init parameters (ignored if NULL).
 *
 * This function must be called before any other function from pdfedit_core_dev
 * library is called. All global wide configuration is done here (and only
 * here).
 * <br>
 * At this moment, it contains only xpdf core initialization (globalParams
 * and base fonts).
 * <br>
 * Function can be called with no parameters and all required values for
 * initialization will be set to default values. Nevertheless, it is highly
 * recommended to provide at least first two parameters for future
 * compatibility and possible changes (without need to change also the code
 * which uses this functionality). You can simply use &argc and &argv values
 * given to the main function.
 * \code
 * int main(int argc, char ** argv)
 * {
 * 	...
 * 	if (pdfedit_core_dev_init(&argc, &argv))
 * 	{
 * 		// not able to initialize pdfedit-core-dev
 * 		return 1;
 * 	}
 *
 *	// hadle the rest of argc, argv
 *	// work the rest of the code
 *      // final cleanup
 *      pdfedit_core_dev_destroy();
 * }
 * \endcode
 * <br>
 * If some of argc parameters are used by initialization code, they are
 * removed from the array and argc value is decreased appropriatelly.
 * It is recommended to call this function before any command line parameters
 * are handled by application.
 * <br>
 * If either argc or argv is NULL they must be NULL both of them. Also *argv
 * array must contain at least argc numbers of elements. Any violation of
 * these rules are reported as error and initialization fails.
 * <br>
 * The last init parameter can be used to set configuration not covered
 * by command line parameters. It is required only when very specific
 * configuration is required. Please @see struct pdfedit_core_dev_init.
 *
 * <br>
 * Repeated calls (without pdfedit_core_dev_destroy) are ignored.
 *
 * @return 0 on success, -error code otherwise.
 */
int pdfedit_core_dev_init(int *argc=NULL, char ***argv=NULL, const struct pdfedit_core_dev_init * init=NULL)
	WARN_UNUSED_RESULT;

/** Checks whether global pdfedit_core_dev_init has been called.
 * This method is for internal purposes only and should be called by all
 * methods which could possible deal with global wide configuration (e.g.
 * xpdf globalParams).
 * <br>
 * If this method returns with false, caller should triger PDFedit_devException.
 *
 * @return true on success, false otherwise.
 */
bool pdfedit_core_dev_init_check()WARN_UNUSED_RESULT;

/** Clean up pdfedit_core_dev environment.
 * This function should be called in the final cleanup.
 * Future call to pdfedit_core_dev_init is possible.
 */
void pdfedit_core_dev_destroy();

#endif

