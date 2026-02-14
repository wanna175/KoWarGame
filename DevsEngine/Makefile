export HOMEDIR=/home/ko/Desktop/cStudy/test
SOURCE_DIR = $(HOMEDIR)/src
LIB_SOURCE_DIR = $(HOMEDIR)/lib/src

COMPILE_DIRS = $(SOURCE_DIR)

all:
	cd $(LIB_SOURCE_DIR); make
	cd $(SOURCE_DIR); make
clean:
	cd $(LIB_SOURCE_DIR); rm *.o;
	cd $(HOMEDIR)/lib; rm *.a;
	cd $(SOURCE_DIR); rm *.o;
	cd $(HOMEDIR)/bin; rm *;
