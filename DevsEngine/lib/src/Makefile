# 확장자 및 컴파일러 설정
C++_FILE_EXT=cpp
OBJ_FILE_EXT=o
CC=g++

# 디렉토리 및 파일 경로
HOMEDIR=/home/ko/Desktop/cStudy/test
LIBDIR=$(HOMEDIR)/lib
LIBNAME=libKODEVSIM.a
TARGET=$(LIBDIR)/$(LIBNAME)

OBJDIR=.

# include & lib path
INC_PATH=-I. -I/usr/local/include -I$(HOMEDIR)/inc -I$(HOMEDIR)/lib/inc
LIB_PATH=-L. -L$(HOMEDIR)/lib

# 컴파일 플래그
CFLAGS=-fPIC -Wall -g
C++FLAGS=-D_REENTRANT

# 소스 및 오브젝트 파일
CFILES=$(sort $(wildcard *.$(C++_FILE_EXT)))
OFILES=$(CFILES:.$(C++_FILE_EXT)=.$(OBJ_FILE_EXT))

# 기본 규칙: 라이브러리 생성
all: $(TARGET)

# 라이브러리 생성
$(TARGET): $(OFILES)
	@echo
	@echo "++++++++++++++++++++++[ Generating $(TARGET) ]++++++++++++++++++++++"
	@mkdir -p $(LIBDIR)
	ar rcs $@ $(OFILES)
	@echo "++++++++++++++++++++++[ Build Complete ]+++++++++++++++++++++++++++++"

# 오브젝트 파일 생성 규칙
.$(C++_FILE_EXT).o:
	@echo
	@echo "------------------[ Compiling $< ]-------------------"
	$(CC) -w $(CFLAGS) $(C++FLAGS) $(INC_PATH) -c -o $*.o $*.cpp -lrt -pthread

# 클린 명령
clean:
	@rm -f *.o
	@rm -f $(TARGET)
	@echo "[Clean Finished]"

.PHONY: all clean
