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
