#####################################################################
##
##  makedefs.mk
##  
##  standard build options for GNU make
##
##  support for building executables, static libraries, dynamic libs
##  and dependencies, output will always be in src directory.
##
##  Rev 0.0.1 08/30/2010
##  Rev 0.0.2 08/30/2010 - added EXT_DEPS to prereqs
##	Rev 0.0.3 09/22/2010 - fixed bug where generating .o file even if error
##  Rev 0.0.4 12/06/2010 - use nm to create map file
##						 - generate eep file (AVR)
##  Rev 0.0.5 12/13/2010 - added support for GTK executables 
##  Rev 0.0.6 12/22/2010 - added LINK, DEBUG and STANDARD tags
##  Rev 0.0.7 12/15/2011 - build all files as subdir, instead of as libraries
##
#####################################################################
	

	

#pull in source files from all packages
C_SRC = 
CPP_SRC = 

PACKAGE_MAKEFILES= $(PACKAGES:%=%/subdir.mk)
-include $(PACKAGE_MAKEFILES)
	
OBJS = $(C_SRC:%.c=%.o)
CPP_OBJS =$(CPP_SRC:%.cpp=%.o)
DEPS = $(OBJS:%.o=%.d) \
	   $(CPP_OBJS:%.o=%.d)
LIB_FLAGS = $(LIBS:%=-l%) \
			$(LIB_DIR:%=-L../%/) 
EXTRA_CFLAGS+= $(PROJ_LIBS:%=-I../%/) 
EXTRA_CFLAGS+= $(PACKAGES:%=-I%)
EXT_DEPS+=$(PACKAGE_MAKEFILES)
CFLAGS+=$(DEBUG:%=-g%)
LDFLAGS+=$(DEBUG:%=-g%)
CFLAGS+=$(OPT:%=-O%)
CFLAGS+=$(CSTANDARD:%=-std=%)
CFLAGS+=$(DEFINES:%=-D%)


clean:
	@echo
	@echo Removing files
	$(RM) $(OBJS) $(DEPS) $(EXECUTABLE_NAME) $(MAP_FILE) $(LIB_NAME) $(LIBD_NAME) $(HEXFILE_NAME) $(EEPFILE_NAME) $(GTK_EXECUTABLE_NAME)


# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)
-include $(CPP_OBJS:.o=.d)

$(EEPFILE_NAME): $(EXECUTABLE_NAME)
	$(OBJCOPY) $(EEP_OBJCOPY_FLAGS) $(EXECUTABLE_NAME) $(EEPFILE_NAME)

$(HEXFILE_NAME): $(EXECUTABLE_NAME)
	$(OBJCOPY) $(OBJCOPY_FLAGS) $(EXECUTABLE_NAME) $(HEXFILE_NAME)

$(EXECUTABLE_NAME): $(OBJS) $(CPP_OBJS) $(EXT_DEPS)
	@echo 
	@echo Linking $(EXECUTABLE_NAME)
	$(CXX) -o $(EXECUTABLE_NAME) $(LDFLAGS) $(EXTRA_LDFLAGS) $(OBJS) $(CPP_OBJS) $(ASMS) $(LIB_FLAGS)
	$(SIZE) $(SIZE_FLAGS) $(EXECUTABLE_NAME)
	$(NM) $(NM_FLAGS) $(EXTRA_NMFLAGS) $(EXECUTABLE_NAME) > $(MAP_FILE)

$(LIB_NAME): $(OBJS) $(CPP_OBJS) $(EXT_DEPS)
	@echo
	@echo Linking $(LIB_NAME)
	$(AR) rs $(LIB_NAME) $(OBJS) $(CPP_OBJS) $(ASMS)
	$(NM) $(NM_FLAGS) $(EXTRA_NMFLAGS) $(LIB_NAME) > $(MAP_FILE)
	
$(LIBD_NAME): $(OBJS) $(CPP_OBJS) $(EXT_DEPS)
	@echo
	@echo Linking $(LIBD_NAME)
	$(CXX) -shared $(LDFLAGS) $(EXTRA_LDFLAGS) -o $(LIBD_NAME) $(OBJS) $(CPP_OBJS) $(ASMS) $(LIB_FLAGS) 
	$(NM) $(NM_FLAGS) $(EXTRA_NMFLAGS) $(LIBD_NAME) > $(MAP_FILE)
	@echo Installing $(LIBD_NAME)

# compile and generate dependency info;
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons
$(CPP_OBJS): %.o : %.cpp $(MAKEFILES)
	@echo
	@echo Compiling $*.cpp
	$(CXX) -c $(CXXFLAGS) $(EXTRA_CXXFLAGS) $< -o $@
	$(CXX) -MM $(CXXFLAGS) $(EXTRA_CXXFLAGS) $*.cpp > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@rm -f $*.d.tmp

$(OBJS): %.o : %.c $(MAKEFILES)
	@echo
	@echo Compiling $*.c
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@
	$(CC) -MM $(CFLAGS) $(EXTRA_CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@rm -f $*.d.tmp
	