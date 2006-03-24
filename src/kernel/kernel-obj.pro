#
# Path to xpdf object files
#

_PATH = ../xpdf

#
# One specific lib, that must exist
#

_ONELIB = Stream.o


exists( $${_PATH}/xpdf/$${_ONELIB} ) {

	LIBS += $$system( find $$_PATH/goo -name "*.o" )
	LIBS += $$system( find $$_PATH/fofi -name "*.o" )
	LIBS += $$system( find $$_PATH/xpdf -name "[^pdf]*.o" )

}else{
	
	# Specific path
	_PATH = /usr/xpdf
	
	exists( $${_PATH}/$${_ONELIB} ) {

		LIBS += $$system( find $$_PATH/ -name "*.o" )
		
	}else{
		error( "You do not have xpdf libraries compiled. [../xpdf/xpdf/Object.o, ../xpdf/xpdf/Stream.o, ....]" )
	}
}
