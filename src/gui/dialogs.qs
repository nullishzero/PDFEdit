/* === Dialog helper functions === */

/** 
 * Create LineEdit dialog with label and text filled. 
 */
function createLineEdit(label,text) {
 var e = new LineEdit;
 e.label = label;
 e.text = text;
 return e;
}

/** 
 * Create LineEdit dialog with label and text filled and add it 
 * to another widget.
 */
function createLineEditAndDisplay(label,text,widget) {
 var e = new LineEdit;
 e.label = label;
 e.text = text;
 widget.add(e);
 return e;
}

/**
 * Create group box with title and tooltip the same.
 */
function createGroupBox(title) {
 var gb = new GroupBox;
 gb.title = title;
 gb.tooltip = title;
 return gb;
}

/**
 * Create group box with title and tooltip the same.
 */
function createGroupBoxAndDisplay(title,widget) {
 var gb = new GroupBox;
 gb.title = title;
 gb.tooltip = title;
 widget.add(gb);
 return gb;
}

/**
 * Create normal dialog
 */
function createDialog(caption, okButtonText, cancelButtonText, tooltip) {
 var dialog = new Dialog;
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
  var sb = new SpinBox;
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
  var sb = new NumberEdit;
  sb.label = label;
  sb.minimum = min;
  sb.maximum = max;
  sb.tooltip = label;
  sb.whatsThis = label;
  widget.add (sb);
  return sb;
 }

