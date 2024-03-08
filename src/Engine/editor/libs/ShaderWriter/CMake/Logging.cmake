#--------------------------------------------------------------------------------------------------
#	Macro :	msg_debug
#	Used to print debug messages
#--------------------------------------------------------------------------------------------------
set( SHOW_DEBUG_LOGS OFF )
macro( msg_debug msg )
	if (${SHOW_DEBUG_LOGS} )
		message( STATUS "[DEBUG] ${msg}")
	endif ()
endmacro( msg_debug )
