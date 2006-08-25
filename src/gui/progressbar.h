#ifndef __PROGRESSBAR_H__
#define __PROGRESSBAR_H__

#include <qprogressbar.h>
#include "kernel/pdfwriter.h"

namespace gui {

class ProgressBar: public pdfobjects::utils::IProgressBar, public QProgressBar {
public:
 ProgressBar( QWidget* parent=0, const char* name=0, WFlags f=0);
 ProgressBar( int totalSteps, QWidget* parent=0, const char* name=0, WFlags f=0);
 void start();
 void finish();
 void setMaxStep(int maxStep);
 void update(int step);
};

} // gui namespace

#endif
