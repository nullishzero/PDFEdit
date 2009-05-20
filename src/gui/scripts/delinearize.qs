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
//Delinearize plugin

function delinearize_fail(err) {
 print(tr("Delinearization failed!"));
 print(err);
 message(tr("Delinearization failed!")+" "+err);
}

/** Function to invoke the plugin functionality */
function delinearize_gui() {
 inFile=fileOpenDialog();
 if (!inFile) return;
 outFile=fileSaveDialogDiffer(inFile);
 if (!outFile) return;
 if (!exists(inFile)) { delinearize_fail(tr("Input file '%1' does not exist").arg(inFile)); return; }
 if (inFile==outFile) { delinearize_fail(tr("Input and output files must be different")); return; }
 if (delinearize(inFile,outFile)) {
  print(tr("Delinearized")+" :"+inFile+" -> "+outFile);
 } else {
  delinearize_fail(error());
 }
}

//Install the plugin

// install the menu/toolbar items that are used by the plugin
createMenuItem('toolsmenu','delinearize',tr('Delinearize'),'delinearize_gui()','','delin.png');
