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
#include <errno.h>
#include "kernel/pdfedit-core-dev.h"
#include "kernel/pdfwriter.h"

static bool initialized = false;
using namespace pdfobjects;
using namespace utils;

/** Parse command line parameters relevant for pdfedit-core-dev.
 * @param argc Pointer to the argv elements count (ignored if NULL).
 * @param argv Pointer to arguments array (ignoder if NULL).
 *
 * Removes all such relevant parameters from argv and decreases argc
 * appropriately.
 * <br>
 * argc and argv must be either both NULL (then they are ignored) or
 * nonNULL and argv must contain at least argc number of elements.
 *
 * @return 0 on success or -error otherwise.
 */
static int parse_command_line(int *argc, char ***argv)
{
	// argc, argv have to be either both NULL or nonNULL
	if((!argc && argv) || (!argv && argc))
		return -EINVAL;
	// Nothing to do at the moment.
	return 0;
}

/** Initializes all xpdf core related stuff.
 * @param init Initialization structure (use default when NULL).
 *
 * Initializes GlobalParams global structure and base fonts.
 *
 * @return 0 on success, -error otherwise.
 */
static int init_xpdf_core(const struct pdfedit_core_dev_init *init)
{
	// initializes global parameters for xpdf code - TODO use
	// configuration file - from parameters
	const char * cfgFileName = (init)?init->cfgFileName:NULL;
	GlobalParams::initGlobalParams(cfgFileName);
	if(!globalParams)
		return -ENOMEM;
	const char * fontDir = (init)?init->fontDir:NULL;
	globalParams->setupBaseFonts(fontDir);
	return 0;
}

/** Registers all global filtewriters.
 */
static void init_stream_filterwriters()
{
	// NullFilterStreamWriter doesn't have to be registered as it is default 
	// and we will fallback to it anyway
	FilterStreamWriter::registerFilterStreamWriter(ZlibFilterStreamWriter::getInstance());

	utils::FilterStreamWriter::setDefaultStreamWriter(NullFilterStreamWriter::getInstance());
}

int pdfedit_core_dev_init(int *argc, char ***argv, const struct pdfedit_core_dev_init * init)
{
	if(initialized)
		return 0;

	int ret = parse_command_line(argc, argv);
	if(ret)
		return ret;

	if((ret = init_xpdf_core(init)))
		return ret;

	init_stream_filterwriters();
	initialized = true;
	return 0;
}

bool pdfedit_core_dev_init_check()
{
	return initialized;
}

void pdfedit_core_dev_destroy()
{
	GlobalParams::destroyGlobalParams();
	initialized = false;
	FilterStreamWriter::unregisterFilterStreamWriter(ZlibFilterStreamWriter::getInstance());
}
