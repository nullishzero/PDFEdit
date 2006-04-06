#
# Path to xpdf object files
#

_PATH = ../xpdf

#
# One specific lib, that must exist
#

_ONELIB = Stream.o


exists( $${_PATH}/xpdf/$${_ONELIB} ) {

	LIBS += $$system( find $$_PATH/xpdf -name "[^pdf]*.o" )
	LIBS += -lfofi -lGoo -L$$_PATH/fofi -L$$_PATH/goo

}else{
	
	# Specific path
	_PATH = /usr/xpdf
	
	exists( $${_PATH}/$${_ONELIB} ) {

		LIBS += $$system( find $$_PATH/ -name "*.o" )
		
	}else{
		error( "You do not have xpdf libraries compiled. [../xpdf/xpdf/Object.o, ../xpdf/xpdf/Stream.o, ....]" )
	}
}
