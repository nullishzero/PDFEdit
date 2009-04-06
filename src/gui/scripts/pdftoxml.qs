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
//
// pdftoxml
//  jmisutka (06/12/03)
//

/**
 * Invoke pdf to xml.
 */
function pdftoxml_gui() {

	// Get input file name
	inFile = fileOpenDialog ();
	if (!inFile) return;
	if (!exists(inFile)) { print (tr("Input file '%1' does not exist.").arg(inFile)); return; }
	// Select pages
	selection = selectPagesDialog (inFile);
	if (!selection) return;
	// Get output file name
	outFile = fileSaveDialogXml();
	if (!outFile) return;
	if (inFile == outFile) { print (tr("Input and output file must differ.")); return; }

	// Do the job
	var xml = pdftoxml (inFile, selection, outFile);
	if (xml) {
		print (tr("Xml produced")+" :"+inFile+" -> "+outFile);
	}else{
		print (tr(error()));
	 	message (tr("Xml conversion failed!") + tr(error()));
		exit (1);
	}

	//
	// Display the page
	//
	dg = createDialog (tr("Xml from ") + inFile + " " + tr("file"),tr("Ok"),tr("Cancel"));
	var gb = createGroupBoxAndDisplay (tr("Xml"),dg);
	te = new TextEdit;
	te.text = xml;
	dg.width = 700;
	te.tooltip = tr("Xml conversion.");
	gb.add (te);
	dg.exec();
	print (tr("Xml displayed."));
}

//
// Install the plugin
//

// Install the menu/toolbar items that are used by the plugin
createMenuItem ('toolsmenu','pdftoxml',tr('Pdf to xml'),'pdftoxml_gui()','','pdftoxml.png');
