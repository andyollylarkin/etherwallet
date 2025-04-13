CXX = gcc
CXXFLAGS = -shared -fPIC -O3 -Wall -Wextra -pthread -march=native
ifeq ($(shell uname), Darwin)
LDFLAGS = -lcrypto -lsecp256k1 -lkeccak -L/opt/homebrew/lib
else
LDFLAGS = -lcrypto -lsecp256k1 ./libkeccak.a -L/usr/lib
endif
ifeq ($(shell uname), Darwin)
INCLUDES = -I/opt/homebrew/include
else
INCLUDES = -I/usr/include -I.
endif
ifeq ($(shell uname), Darwin)
TARGET = libwallet.dylib
else
TARGET = libwallet.so
endif
SRC = wallet_gen.c

ifeq ($(shell uname), Darwin)
TARGET_STATIC = libwallet_osx.a
else
TARGET_STATIC = libwallet_linux.a
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDES) $(LDFLAGS)


clean-all:
	rm -f $(TARGET)
	rm -f *.o
	rm -f ./*.a

clean:
	rm -f $(TARGET)
	rm -f *.o

build-test-app: $(TARGET)
	$(CXX) -L. -lwallet -O3 ./walgen.c -o walgen


build-static: $(SRC)
	$(CXX) -c -O3 -Wall -pthread -march=native $(INCLUDES) $(SRC)
	ar rcs $(TARGET_STATIC) wallet_gen.o