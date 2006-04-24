#
# Path to xpdf object files
#

_PATH = ../xpdf

#
# One specific lib, that must exist
#
_ONELIB = libxpdf.a

exists( $${_PATH}/xpdf/$${_ONELIB} ) {
	LIBS += -lxpdf -L$$_PATH/xpdf
	LIBS += -lfofi -lGoo -L$$_PATH/fofi -L$$_PATH/goo
	LIBS += -lsplash -lt1 -L$$_PATH/splash
}else{
	error( "You do not have xpdf compiled. [../xpdf/xpdf/Object.o, ../xpdf/xpdf/Stream.o, ....]" )
}
