-include ../config.mk

CPP_SRC= 

C_SRC= timestamp.c \
	error_message.c \
	utility.c \

TARGET_NAME:=libwiicarutility

LIB_NAME:=$(TARGET_NAME).a

LIBD_NAME:=$(TARGET_NAME).so.1

all compile build install debug release pcsimapp cboard_test_gui_app: $(LIB_NAME) $(LIBD_NAME)

-include ../makedefs.mk



