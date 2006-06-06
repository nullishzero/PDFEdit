/* ==== Update functions ==== */

/** Tries to update scripts. */
function update (text) {
	if (undefined == text)
		text = "";

	//
	// First window
	//	
 	var dg = createDialog (tr("Script updater"), tr("Update"), tr("Cancel"), tr("Script updater"));
	var gb = createGroupBoxAndDisplay (tr("Update process information"),dg);
	var te = new TextEdit;
	te.text = tr("You can update from project site or any other site containin QSA scripts for PdfEditor.\n\n")+
			  tr("First you will be asked for the script directory. Downloaded scripts will be placed into this directory.")+" "+
			  tr("For automatic script loading select the default script directory (see user documentation).\n\n")+
			  tr("Then, you will be asked for executable utility (e.g. wget) which can automatically download files from http server using")+
			  tr("syntax: executable -P directory files.\n\n")+
			  tr("Finally you will be asked to open the script, which will be executed. (Be carefull, QSA can execute processes)");
	gb.add (te);
	dg.width = 1000;
	
	if (!dg.exec()) return;

	//
	// Directory selection
	//
	var dirname = FileDialog.getExistingDirectory( "",tr("Select directory where scripts should be downloaded.jano.script"));
	if (!dirname) return;

	var name = Input.getText(tr("Select script location"),"http://kde.bolo.tam.bolo.html",tr("Select script location"));
	if (!name) return;
	
	var wget = FileDialog.getOpenFileName( "",tr("Select downloading utility.") );
	if (!wget) return;
	
	var cmd = wget+" -P "+dirname+" "+name;
    var ans = MessageBox.warning( tr("Do you really want to execute ")+cmd, MessageBox.Yes, MessageBox.No);
    if (ans != MessageBox.Yes) return;

	Process.execute (wget);
	print(Process.stdout);
	
	// Open the script
	runscript();
	
}

/** Run a script */
function runscript(script) {
		
	if (undefined == script) {
		// Open the script
		script = FileDialog.getOpenFileName( "*.qs",tr("Select script to execute.") );
		if (!script) return;
	}

	var scripttext = File.read(script);
	
 	var dg = createDialog (tr("Script launcher"), tr("Run"), tr("Cancel"), tr("Script launcher"));
	var gb = createGroupBoxAndDisplay (tr("Script"),dg);
	var te = new TextEdit;
	te.text = scripttext;
	gb.add (te);
	dg.width = 1000;
	if (!dg.exec()) return;
	
	eval(te.text);;
}


