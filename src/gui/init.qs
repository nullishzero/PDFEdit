//this script will be run at start

function quit() {
 exitApp();
}

function closewindow() {
 closeWindow();
}

function newwindow() {
 createNewWindow();
}

print("Windowstate of main is: "+settings.read("gui/windowstate/main"));
