PROG=testcv.prog
TEST=remap.prog
CXXFLAGS=-Wall -Wextra -pedantic -std=c++11 `pkg-config --cflags opencv`
LDFLAGS=`pkg-config --libs opencv`
OBJS=calib.o

all : test norm

norm : $(PROG)

test : $(TEST)

$(PROG) : $(OBJS) main.o
	g++ $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST) : $(OBJS) test.o
	g++ $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o : %.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

clean :
	rm -vf *.o

rec : clean all

.PHONY:rec clean all test


