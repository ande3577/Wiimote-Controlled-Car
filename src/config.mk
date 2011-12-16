#####################################################################
##
##  pc.mk
##  
##  Platform specific configuration options (linux)
##
##  Shared among all build targets
##
##  Rev 0.0.1 12/6/2010
##
#####################################################################

-include pc.mk

debug: DEFINES += _DEBUG=1
debug: DEBUG=3
debug: OPT=0

