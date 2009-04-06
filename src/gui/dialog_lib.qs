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
/* === Dialog helper functions === */

/**
 * Create LineEdit dialog with label and text filled.
 */
function createLineEdit(label,text) {
 e = new LineEdit;
 e.label = label;
 e.text = text;
 return e;
}

/**
 * Create LineEdit dialog with label and text filled and add it
 * to another widget.
 */
function createLineEditAndDisplay(label,text,widget) {
 e = new LineEdit;
 e.label = label;
 e.text = text;
 widget.add(e);
 return e;
}

/**
 * Create NumberEdit dialog with label and value filled and add it
 * to another widget.
 */
function createNumberEditAndDisplay(label,value,widget) {
 e = new NumberEdit;
 e.label = label;
 e.value = value;
 widget.add(e);
 return e;
}

/**
 * Create group box with title and tooltip the same.
 */
function createGroupBox(title) {
 gb = new GroupBox;
 gb.title = title;
 gb.tooltip = title;
 return gb;
}

/**
 * Create group box with title and tooltip the same.
 */
function createGroupBoxAndDisplay(title,widget) {
 gb = new GroupBox;
 gb.title = title;
 gb.tooltip = title;
 widget.add(gb);
 return gb;
}

/**
 * Create normal dialog
 */
function createDialog(caption, okButtonText, cancelButtonText, tooltip) {
 dialog = new Dialog;
 dialog.caption = caption;
 dialog.okButtonText = okButtonText;
 dialog.cancelButtonText = cancelButtonText;
 dialog.tooltip = tooltip;
 return dialog;
}

/**
 * Create spin box with max and min values and display it
 */
 function createSpinboxAndDisplay (label,min,max,widget) {
  sb = new SpinBox;
  sb.label = label;
  sb.minimum = min;
  sb.maximum = max;
  sb.tooltip = label;
  sb.whatsThis = label;
  widget.add (sb);
  return sb;
 }

/**
 * Create number edit box with max and min values and display it
 */
 function createNumbereditAndDisplay (label,min,max,widget) {
  sb = new NumberEdit;
  sb.label = label;
  sb.minimum = min;
  sb.maximum = max;
  sb.tooltip = label;
  sb.whatsThis = label;
  widget.add (sb);
  return sb;
 }

/**
 * Create radio button and display it
 */
function createRadioButtonAndDisplay (text,widget) {
 rb = new RadioButton;
 rb.text = text;
 rb.checked = false;
 rb.tooltip = text;
 widget.add (rb);
 return rb;
}

/**
 * Create check box and display it
 */
function createCheckBoxAndDisplay (text,widget) {
 cb = new CheckBox;
 cb.text = text;
 cb.checked = false;
 cb.tooltip = text;
 widget.add (cb);
 return cb;
}

/**
 * Make x,y edit boxes in a group box.
 */
function xydialogs ( dg, text, units, default_x, default_y ) {
	var units_name = "";
	if (undefined != units)
		units_name = "( "+units+" )";
	var x = 0;
	if (undefined != default_x)
		x = default_x;
	var y = 0;
	if (undefined != default_y)
		y = default_y;

	gb = createGroupBoxAndDisplay (text, dg);
	ex = createNumberEditAndDisplay ("x"+tr("position")+units_name, x, gb);
	ey = createNumberEditAndDisplay ("y"+tr("position")+units_name, y, gb);
	return [ex,ey];
}

/**
 * Make two number edit boxes in a group box.
 */
function twonumdialogs (dg,text,t1,t2) {
	gb = createGroupBoxAndDisplay (text, dg);
	ex = createNumberEditAndDisplay (t1, "0", gb);
	ey = createNumberEditAndDisplay (t2, "0", gb);
	return [ex,ey];
}
