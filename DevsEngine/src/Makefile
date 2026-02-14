C++_FILE_EXT=cpp
OBJ_FILE_EXT=o
CC=g++

HOMEDIR=/home/ko/Desktop/cStudy/test

BIN=$(HOMEDIR)/bin/DataTransfer
OBJDIR=.

# include & lib path 설정
INC_PATH=-I. -I/usr/local/include -I$(HOMEDIR)/inc -I$(HOMEDIR)/lib/inc
LIB_PATH=-L$(HOMEDIR)/lib

# 정적 라이브러리 링크
LIBS=-lKODEVSIM -lpthread -lrt

CFLAGS=-fPIC -Wall -g 
C++FLAGS=-D_REENTRANT

CFILES=$(sort ${wildcard *.$(C++_FILE_EXT)} )
OFILES=$(CFILES:.$(C++_FILE_EXT)=.o)

.SUFFIXES:.$(C++_FILE_EXT)

# 실행 파일 빌드
${BIN} : $(OFILES)
	@echo
	@echo "+++++++++++++++++++++[ run file Generate starting]+++++++++++++++++++++++"
	$(CC) -o $@ $(OFILES) $(LIB_PATH) $(LIBS)
	@echo "++++++++++++++++++++[ run file Generate Finished]++++++++++++++++++++++++"

# 오브젝트 파일 빌드
.$(C++_FILE_EXT).o:$(CFILES)
	@echo
	@echo "------------------[ Main Folder " $< " file compiling ]-------------------"
	$(CC) -w $(CFLAGS) ${C++FLAGS} ${INC_PATH} -c -o ${OBJDIR}/$*.${OBJ_FILE_EXT} $*.$(C++_FILE_EXT)

# 클린 명령
clean:
	@rm -f *.o
	@rm -f $(BIN)
	@echo "[Clean Finished]"
