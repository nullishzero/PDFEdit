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

print("PDF Editor "+version());
