//this script will be run at start

function quit() {
 //todo: do another way
 exitApp();
}

function closewindow() {
 //todo: do another way
 closeWindow();
}

function newwindow() {
 createNewWindow();
}

//Toggle given boolean setting
function toggle(key) {
 val=settings.readBool(key);
 settings.write(key,!val);
}

//Save from menu/toolbar
function func_save() {
 save();
}

//Save as from menu/toolbar
function func_saveas() {
 var name=fileSaveDialog(filename());
 if (!name) return;
 print("Saving as "+name);
 saveAs(name);
}

//Load from menu/toolbar
function func_load() {
 if (!closeFile(true,true)) return;
 var name=fileOpenDialog();
 if (name) openFile(name);
}

//Load from menu/toolbar
function func_new() {
 closeFile(true);
}

print("PDF Editor "+version());
