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

CSTANDARD=gnu99

DEFINES += __DEVICE_VERSION__=\"0.0.1\"
DEFINES += _DIAGNOSTIC_MODE=0
debug: DEFINES += _DEBUG=1
debug: DEBUG=3
debug: OPT=0

