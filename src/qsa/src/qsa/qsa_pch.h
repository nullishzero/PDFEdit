/*
 * This is a precompiled header file for use in Xcode / Mac GCC /
 * GCC >= 3.4 / VC to greatly speed the building of Qt. It may also be
 * of use to people developing their own project, but it is probably
 * better to define your own header.  Use of this header is currently
 * UNSUPPORTED.
 */

#if (defined(_WIN32) || defined(__NT__))
#  define QT_UNDEF_MACROS_IN_PCH
#  define _WINSCARD_H_
#  define _POSIX_ 	/* Make sure PATH_MAX et al. are defined    */
#  include <limits.h>
#  undef _POSIX_  	/* Don't polute                             */
#endif

#if defined __cplusplus
#  if defined(__GNUC__)
#    ifndef QT_NO_STL
#      include <ios>
#      undef _GLIBCPP_FULLY_COMPLIANT_HEADERS  // Makes qlocale.cpp compile
#    endif
#  endif
#include <qmap.h>		    // I must be first!
#include <private/qucomextra_p.h>  // All moc genereated code has this include
#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qdatetime.h>
#include <qglobal.h>
#include <qimage.h>
#include <qmetaobject.h>  // All moc genereated code has this include
#include <qobject.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qplatformdefs.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qwidget.h>

#include <limits.h>
#include <stdlib.h>
#if defined(__GNUC__)
#  ifndef QT_NO_STL
#    define _GLIBCPP_FULLY_COMPLIANT_HEADERS
#  endif
#endif
#endif

#if defined(QT_UNDEF_MACROS_IN_PCH)
#  undef max /*  These are defined in windef.h, but                   */
#  undef min /*  we don't want them when building Qt                  */
#  undef _WINSCARD_H_
#endif

#include "qsinterpreter.h"
#include "qsproject.h"
#include "qsargument.h"

#include "../kernel/quickinterpreter.h"
#include "../kernel/quickobjects.h"
#include "../kernel/quickobjects.h"

#include "../engine/qsclass.h"
#include "../engine/qsobject.h"
#include "../engine/qsmember.h"
#include "../engine/qsenv.h"
#include "../engine/qsengine.h"
#include "../engine/qsinternal.h"
#include "../engine/qsnodes.h"

