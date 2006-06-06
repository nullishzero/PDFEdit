//PDF Editor init script
//This script will be run at start, or on creation of new window
//Each window have its own scripting context

/* ==== load various needed functions ==== */
run( "pdfoperator.qs" );	//Functions for manipulation with pdf operators
run( "callback.qs" );			//Callback handlers
run( "dialog_lib.qs" );			//Dialog helper functions
run( "dialogs.qs" );			//Dialogs
run( "menu.qs" );			//Functons activated from menu/toolbars
run( "page.qs" );			//Page manipulation functions
run( "test.qs" );			//Testing functions

/* ==== Other helper functions ==== */

/** Toggle on/off boolean setting with specified key */
function toggle(key) {
 val=settings.readBool(key);
 settings.write(key,!val);
}

/** Get dictionary from page number X */
function pageDict(x) {
 page=document.getPage(x);
 return page().getDictionary();
}

/** === Validate functions === */

/** Is argument a number */
function isNumber(x) {
	if (undefined == x || isNaN(parseFloat(x)))
		return false;
	else
		return true;
}

/** Are all numbers */
function isNumber2(x,y) {
	if (isNumber(x) && isNumber(y))
		return true;
	else
		return false;
}

/** Are all numbers */
function isNumber4(x,y,z,w) {
	if (isNumber(x) && isNumber(y) && isNumber(z) && isNumber(w))
		return true;
	else
		return false;
}

/** Validate current page */
function isPageAvaliable() {
 if (page()) {
	return true;
 }else
 	return false;
}

/** Validate item selection */
function isTreeItemSelected() {
 if (undefined == treeitem) {
	return false;
 }else
 	return true;
}

/* ==== Code to run on start ==== */

//Print welcome message
print("PDF Editor "+version());
checkMenus();
