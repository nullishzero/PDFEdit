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
#ifndef __REVISIONTOOL_H__
#define __REVISIONTOOL_H__

#include <qwidget.h>
#include <boost/smart_ptr.hpp>

class QComboBox;
class QResizeEvent;

namespace pdfobjects {
 class CPdf;
}

namespace gui {

/**
 Toolbutton listing revisions of current PDF document, with ability to switch revisions<br>
 Can be placed in toolbar in place of ordinary button
 \brief Toolbar widget for changing revision
*/
class RevisionTool : public QWidget {
Q_OBJECT
public:
 RevisionTool(QWidget *parent=0,const char *name=NULL);
 ~RevisionTool();
 QSize sizeHint() const;
public slots:
 void updateRevision(int revision);
 void selectRevision(int revision);
 void setDocument(boost::shared_ptr<pdfobjects::CPdf> newDocument);
signals:
 /**
  Signal emitted when user changes the revision in this control
  @param revision number of selected revision
 */
 void revisionChanged(int revision);
protected:
 void resizeEvent (QResizeEvent *e);
protected:
 /** Revision selection list */
 QComboBox *revList;
 /** Reference to current PDF document */
 boost::shared_ptr<pdfobjects::CPdf> document;
};

} // namespace gui

#endif
