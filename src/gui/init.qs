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
//PDFedit init script
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

/** Validate tree item selection */
function isTreeItemSelected() {
 if (undefined == firstSelectedItem()) {
	return false;
 }else
 	return true;
}

/** Validate item selection */
function isItemInTreeSelected() {
 if (undefined == firstSelected()) {
	return false;
 }else
 	return true;
}

/** Update menus to correctly show certain window shown/hidden state */
function toggleVisItem(item,menuitem) {
 vis=isVisible(item);
 checkItem(menuitem,vis);
// t=getItemText(menuitem);
}

/* ==== Code to run on start ==== */

//Pre-selected font sizes
setPredefs('fontface',getSystemFonts());
//Pre-selected font sizes
setPredefs('fontsize','8,9,10,12,14,16,18,24,36,72');
setNumber('fontsize',10);//default value
//Pre-selected line widths
setPredefs('linewidth','0.5,1,1.5,2,3,4,8,16');
setNumber('linewidth',1);//default value

//Print welcome message
print("PDFedit "+version());
onSelectMode_AllObjects();
checkMenus();

//Item visibility toggleboxes
toggleVisItem('commandline','hideCmd');
toggleVisItem('propertyeditor','hideProp');
toggleVisItem('tree','hideTree');
toggleVisItem('statusbar','hideStatus');

//Set tooltips for items
setTooltip("fontface",tr("Font face"));
setTooltip("fontsize",tr("Font size (in points)"));
setTooltip("linewidth",tr("Line width (in points)"));
