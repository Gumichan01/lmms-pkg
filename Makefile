#   LMMS Project Packager
#   Copyright © 2021 Luxon Jean-Pierre
#
#	See LICENSE
#

# You can modify the value of DEBUG
# If you want to use the debug or release mode
DEBUG=yes

CC=g++
#CC=x86_64-w64-mingw32-g++
BUILD_DIR=build/
SRC_DIR=src/

LMMS_PKG=lmms-pkg
WFLAGS=-Wall -Wextra

ifeq ($(DEBUG),yes)

	# Debug mode
	CFLAGS=-fPIC $(WFLAGS) -std=c++14 -g -fsanitize=address
	OPTIMIZE=-O0
	OPT_SIZE=

else

	# Release mode
	CFLAGS=-fPIC -w -std=c++14
	OPTIMIZE=-O3
	OPT_SIZE=-s

endif

DEPS=`pkg-config --cflags libzip zlib`
LFLAGS=`pkg-config --libs libzip zlib`

.PHONY: clean mrproper

SRCS=$(wildcard */*.cpp) $(wildcard */*/*.cpp) $(wildcard */*/*/*.cpp)
OBJS=$(SRCS:.cpp=.o)

%.o: %.cpp
	@echo $@" - Compiling "$<
	$(CC) -c $< -o $@ $(DEPS)

$(LMMS_PKG): $(OBJS)
	@echo "Create "$@
	@$(CC) -o $@ $(OBJS) $(LFLAGS)

clean:
	@find $(SRC_DIR) -name '*.o' -delete

mrproper: clean
	@rm $(LMMS_PKG) 2>/dev/null
