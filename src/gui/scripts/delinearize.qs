//Delinearize plugin

/** Function to install the menu/toolbar items that are used by the plugin */
function installPlugin() {
 createMenuItem('toolsmenu','delinearize',tr('Delinearize'),'delinearize_gui()','','delin.png');
}

function delinearize_fail(err) {
 print(tr("Delinearization failed!"));
 print(err);
 message(tr("Delinearization failed!")+err);
}

/** Function to invoke the pluigin functionality */
function delinearize_gui() {
 inFile=fileOpenDialog();
 if (!inFile) return;
 outFile=fileSaveDialog();
 if (!outFile) return;
 if (!exists(inFile)) { delinearize_fail(tr("Input file '%1' does not exist").arg(inFile)); return; }
 if (inFile==outFile) { delinearize_fail(tr("Input and output files must be different")); return; }
// if (exists(outFile)) delinearize_fail(tr("Output file '%1' already exist").arg(outFile));
 if (delinearize(inFile,outFile)) {
  print(tr("Delinearized")+" :"+inFile+" -> "+outFile);
 } else {
  delinearize_fail(error());
 }
}

//Install the plugin
installPlugin();
