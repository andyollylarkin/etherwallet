CXX = gcc
CXXFLAGS = -shared -fPIC -O3 -Wall -Wextra -pthread -march=native
ifeq ($(shell uname), Darwin)
LDFLAGS = -lcrypto ./lib/libkeccak_osx.a ./lib/libsecpbtc_osx.a -L/opt/homebrew/lib
else
LDFLAGS = -lcrypto ./lib/libsecpbtc_linux.a ./lib/libkeccak_linux.a -L/usr/lib
endif
ifeq ($(shell uname), Darwin)
INCLUDES = -I/opt/homebrew/include -I./include
else
INCLUDES = -I/usr/include -I./include
endif
ifeq ($(shell uname), Darwin)
TARGET = libwallet.dylib
else
TARGET = libwallet.so
endif
SRC = wallet_gen.c secpbtc.c

ifeq ($(shell uname), Darwin)
TARGET_STATIC = libwallet_osx.a
else
TARGET_STATIC = libwallet_linux.a
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDES) $(LDFLAGS)


clean: clean-obj clean-libs

clean-obj:
	rm -f *.o

clean-libs:
	rm -f $(TARGET) $(TARGET_STATIC)

build-test-app: $(TARGET)
	$(CXX) -L. -lwallet -O3 ./walgen.c -o walgen


build-static: $(SRC)
	$(CXX) -c -O3 -Wall -pthread -march=native $(LDFLAGS) $(INCLUDES) $(SRC)
	ar rcs $(TARGET_STATIC) *.o
	$(MAKE) clean-obj