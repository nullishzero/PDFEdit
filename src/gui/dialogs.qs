/* === Dialog helper functions === */

/**
 * Create warning dialog.
 */
function displayDialogWarning(text) {
	MessageBox.warning( text, MessageBox.Ok );
}

/** 
 * Create LineEdit dialog with label and text filled. 
 */
function createLineEdit(label,text) {
 var e = new LineEdit;
 e.label = label;
 e.text = text;
 return e;
}

