##############################################
# OpenWrt Makefile for helloworld program
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to 
# specify a basic description of the package, 
# where to build our program, where to find 
# the source files, and where to install the 
# compiled program on the router. 
# 
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and 
# there should be no trailing whitespace in
# lines that are not commented.
# 
##############################################

include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:=cwiimotecar
PKG_RELEASE:=1

CONFIGURE_ARGS += \
	CFLAGS='-Wall -Werror'

# This specifies the directory where we're going to build the program.  
# The root build directory, $(BUILD_DIR), is by default the build_mipsel 
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

# Specify package information for this program. 
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION 
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/cwiimotecar
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=CWiid Test -- example project for Wiimote driver
	DEPENDS+= +libcwiid
endef


# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/cwiimotecar/description
	Control a remote control car via a Nintendo Wiimote (R).
endef



# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef



#########################################################################################
# The Build/Compile directive needs to be specified in order to customize compilation
# and linking of our program.  We need to link to uClibc++ and to specify that we 
# do NOT want to link to the standard template library.
#
# To do this we define the LIBS variable.  To prevent linking to the standard libraries we 
# add "-nodefaultlibs" to the $(LIBS) variable and then specify "-lgcc -lc" to ensure that 
# there are no unresolved references to internal GCC library subroutines. Finally 
# "-luClibc++" to link to the  uClibc++ library.  Also, we need to specify "-nostdinc++" 
# in the compiler flags to tell the compiler that c++ standard template library functions
# and data structures will be linked to in specified external libraries and not the 
# standard libraries.
#########################################################################################
#define Build/Compile
#	$(MAKE) -C $(PKG_BUILD_DIR) \
#		LIBS= \
#		LDFLAGS="$(EXTRA_LDFLAGS)" \
#		CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
#		$(TARGET_CONFIGURE_OPTS) $(CPPFLAGS) \
#		CROSS="$(TARGET_CROSS)" \
#		ARCH="$(ARCH)" \
#		$(1) \
#		CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CPPFLAGS)" \
#		$(TARGET_CONFIGURE_OPTS)$( \
#		CROSS="$(TARGET_CROSS)" \
#		ARCH="$(ARCH)" \
#		$(1);
#endef


# Specify where and how to install the program. Since we only have one file, 
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running 
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install 
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the 
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/cwiimotecar/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wiimotecar/wiimotecarapp $(1)/bin/
endef


# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,cwiimotecar))

