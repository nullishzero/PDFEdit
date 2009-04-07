// Console: Flattener
// Description: Flatten input file (remove all revisions except the last one)
// Parameters: [input file] [output file]
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

/** Print help for flattener */
function flatten_help() {
 print(tr("Usage:"));
 print("flatten ["+tr("input file")+"] ["+tr("output file")+"]");
 print(" "+tr("Input file must exist"));
 print(" "+tr("Output file must not exist"));
 exit(1);
}

function flatten_fail(err) {
 print(tr("Flattening failed!"));
 print(err);
 exit(2);
}

p=parameters();
if (p.length!=2) {
 flatten_help("Flattener "+tr("is expecting two parameters"));
}
inFile=p[0];
outFile=p[1];
if (!exists(inFile)) flatten_fail(tr("Input file '%1' does not exist").arg(inFile));
if (exists(outFile)) flatten_fail(tr("Output file '%1' already exist").arg(outFile));
if (inFile==outFile) flatten_fail(tr("Input and output files must be different"));
if (flatten(inFile,outFile)) {
 print(tr("Flattened")+" :"+inFile+" -> "+outFile);
} else {
 flatten_fail(error());
}
