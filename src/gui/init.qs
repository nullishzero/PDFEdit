//this script will be run at start

//todo: do another way
function quit() {
 exitApp();
}

//todo: do another way
function closewindow() {
 closeWindow();
}

function newwindow() {
 createNewWindow();
}

function test() {
 print("This is test function");
 print("All functions from initscript are still alive in QSInterpreter");
}

print("Windowstate of main is: "+settings.read("gui/windowstate/main"));
