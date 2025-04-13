CXX = gcc
CXXFLAGS = -shared -fPIC -O3 -Wall -Wextra -pthread -march=native
LDFLAGS = -lcrypto -lsecp256k1 -L /opt/homebrew/lib -lkeccak
ifeq ($(shell uname), Darwin)
INCLUDES = -I/opt/homebrew/include
else
INCLUDES = -I/usr/include
endif
ifeq ($(shell uname), Darwin)
TARGET = libwallet.dylib
else
TARGET = libwallet.so
endif
SRC = wallet_gen.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDES) $(LDFLAGS)

clean:
	rm -f $(TARGET)
	rm -f libwallet.a
	rm -f *.
	rm -f *.dylib

build-test-app: $(TARGET)
	$(CXX) -L. -lwallet -O3 ./walgen.c -o walgen


build-static-linux: $(SRC)
	$(CXX) -c -O3 -Wall -pthread -march=native $(INCLUDES) $(SRC)
	ar rcs libwallet.a wallet_gen.o
	mkdir -p ../lib
	mv ./libwallet.a ../lib/libwallet.a
	$(MAKE) clean