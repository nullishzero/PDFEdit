#
# Path to xpdf object files
#
XPDFPATH = ../xpdf

#
# Path to utils
#
UTILSPATH = ../utils

#
# One specific lib, that must exist
#
XPDFLIB = libxpdf.a

#
# Utils library
#
UTILSLIB = libutils.a

exists( $${XPDFPATH}/xpdf/$${XPDFLIB} ) {
	LIBS += -lxpdf -L$$XPDFPATH/xpdf
	LIBS += -lfofi -lGoo -L$$XPDFPATH/fofi -L$$XPDFPATH/goo
	LIBS += -lsplash -L$$XPDFPATH/splash

	exists( ../xpdf/config_xpdf_variables ) {
		include( ../xpdf/config_xpdf_variables )
		LIBS += $$config_xpdf_t1_LIBS
		LIBS += $$config_xpdf_libpaper_LIBS
		LIBS += $$config_xpdf_freetype2_LIBS
	}else{
		error( "You do not have ../config_xpdf_variables created from ../xpdf/config.status" )
	}
}else{
	error( "You do not have xpdf compiled. [../xpdf/xpdf/Object.o, ../xpdf/xpdf/Stream.o, ....]" )
}

exists( $${UTILSPATH}/$${UTILSLIB} ) {
        LIBS += -lutils -L$$UTILSPATH
}else{
        error( "You do not have utils library")
}
