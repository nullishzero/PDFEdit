#ifndef _PROGRESSBAR_H_
#define _PROGRESSBAR_H_

#include <qprogressbar.h>
#include "kernel/pdfwriter.h"

namespace gui
{
class ProgressBar: public pdfobjects::utils::IProgressBar, public QProgressBar
{
public:
       ProgressBar( QWidget* parent=0, const char* name=0, WFlags f=0 )
               :QProgressBar(parent, name, f)
       {
               hide();
       }
       
       ProgressBar( int totalSteps, QWidget* parent=0, const char* name=0, WFlags f=0 )
               :QProgressBar(totalSteps, parent, name, f)
       {
               hide();
       }

       void start()
       {
               show();
       }

       void finish()
       {
               hide();
       }

       void setMaxStep(int maxStep)
       {
               setTotalSteps(maxStep);
       }

       void update(int step)
       {
               setProgress(step);
       }
};

} // gui namespace

#endif
