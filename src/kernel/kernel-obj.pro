#
# Path to xpdf object files
#

_PATH = ../xpdf

#
# One specific lib, that must exist
#
_ONELIB = Stream.o

exists( $${_PATH}/xpdf/$${_ONELIB} ) {
	LIBS += -lxpdf -L$$_PATH/xpdf
	LIBS += -lfofi -lGoo -L$$_PATH/fofi -L$$_PATH/goo
}else{
	error( "You do not have xpdf libraries compiled. [../xpdf/xpdf/Object.o, ../xpdf/xpdf/Stream.o, ....]" )
}
