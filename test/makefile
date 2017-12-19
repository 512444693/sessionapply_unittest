FRAMECOMMON=../../../framecommon
INCLUDE=-I/usr/include/mysql/ -I$(FRAMECOMMON)/include
LIBS=-L$(FRAMECOMMON)/lib -lframecommon /usr/lib64/mysql/libmysqlclient_r.a -lz -pthread -lssl -luuid 
MACRO=-D_REENTRANT
CFLAGS=-g -Wall -fPIC ${MACRO}
BINARY=$(patsubst %.cpp,%,$(wildcard *.cpp))
all:$(BINARY)

%:%.cpp
	$(CXX) $< -o $@  $(INCLUDE) $(LIBS) $(CFLAGS) 

clean:
	@rm -rf ${BINARY}

