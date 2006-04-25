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
	LIBS += -lsplash -L$$_PATH/splash

	exists( ../config_xpdf_variables ) {
		include( ../config_xpdf_variables )
		LIBS += $$config_xpdf_t1_LIBS
		LIBS += $$config_xpdf_freetype2_LIBS
	}else{
		error( "You do not have ../config_xpdf_variables created from ../xpdf/config.status" )
	}
}else{
	error( "You do not have xpdf compiled. [../xpdf/xpdf/Object.o, ../xpdf/xpdf/Stream.o, ....]" )
}
