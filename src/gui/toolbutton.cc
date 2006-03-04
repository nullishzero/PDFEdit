/** @file
 ToolButton - Class representing a single button on the toolbar
*/
#include "toolbutton.h"
#include <qiconset.h>

/** Constructor of ToolButton
 @param icon Icon for this button
 @param tooltip Text to display as tooltip
 @param id Button Id that will be emitted : clicked(id)
 @param parent Parent widget
 @param name Name of button
 */
ToolButton::ToolButton(QPixmap *icon, const QString tooltip, int id, QWidget *parent/*=0*/, const char *name/*=""*/) : QToolButton(parent,name) {
 connect(this,SIGNAL(clicked()),this,SLOT(slotClicked()));
 b_id=id;
 setTextLabel(tooltip);
 if (icon) {
  QIconSet ico=QIconSet(*icon);
  setIconSet(ico);
 }
} 

/** slot that will emit clicked with ID of whis button */
void ToolButton::slotClicked() {
 emit clicked(b_id);
}
