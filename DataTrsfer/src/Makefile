C++_FILE_EXT=cpp

OBJ_FILE_EXT=o

CC=g++

#LIB_EXT=a

HOMEDIR=/home/ko/Desktop/git/DataTrsfer/DataTrsfer

BIN=$(HOMEDIR)/bin/DataTransfer

OBJDIR=.

#INC_PATH=-I. -I/ssw/oracle/product/19c/dbhome_1/rdbms/public -I/usr/local/include -I$(HOMEDIR)/inc
#INC_PATH=$(shell find $(HOMEDIR)/inc -type d)
INC_PATH=-I. -I/usr/local/include -I$(HOMEDIR)/inc -I/usr/include/cppconn

LIB_PATH=-L/usr/lib/x86_64-linux-gnu

LIBS=-lm -lpthread -lmysqlcppconn -fpermissive

#LIB_PATH=-L. -L/ssw/oracle/product/19c/dbhome_1/lib

#LIBS=-lm -lpthread -lclntsh -locilib -fpermissive
#LIBS=-lm -lpthread -locilib -fpermissive

CFLAGS=-fPIC -Wall -g 

C++FLAGS=-D_REENTRANT

CFILES=$(sort ${wildcard *.$(C++_FILE_EXT)} )

OFILES=$(CFILES:.$(C++_FILE_EXT)=.o)

.SUFFIXES:.$(C++_FILE_EXT)
${BIN} : $(OFILES)
	@echo
	@echo "+++++++++++++++++++++[ run file Generate starting]+++++++++++++++++++++++"
	$(CC) -o $@ $(OFILES) $(LIB_PATH) $(LIBS)
	@echo "++++++++++++++++++++[ run file Generate Finished]+++++++++++++++++++++"

.$(C++_FILE_EXT).o:$(CFILES)
	@echo
	@echo "------------------[ Main Folder " $< " file compiling ]-------------------"
	$(CC) -w $(CFLAGS) ${C++FLAGS} ${INC_PATH} -c -o ${OBJDIR}/$*.${OBJ_FILE_EXT} $*.$(C++_FILE_EXT) -lrt -pthread

clean:
	@rm *.o
	@rm ./test
	@touch ./*


