#   LMMS Project Packager
#   Copyright © 2021 Luxon Jean-Pierre
#
#	See LICENSE
#

# You can modify the value of DEBUG
# If you want to use the debug or release mode
DEBUG=yes

CC=g++
BUILD_DIR=build/
SRC_DIR=src/
LIBZIPPP_DIST_LINUX=`find dist/linux -name '*.a'`

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

DEPS=`pkg-config --cflags libzip`
LFLAGS=`pkg-config --libs libzip`

.PHONY: clean mrproper

SRCS=$(wildcard */*.cpp) $(wildcard */*/*.cpp) $(wildcard */*/*/*.cpp)
OBJS=$(SRCS:.cpp=.o)

%.o: %.cpp
	@echo $@" - Compiling "$<
	$(CC) -c $< -o $@ $(DEPS)

$(LMMS_PKG): $(OBJS)
	@echo "Create "$@
	@$(CC) -o $@ $(OBJS) $(LIBZIPPP_DIST_LINUX) $(LFLAGS)

clean:
	@find $(SRC_DIR) -name '*.o' -delete

mrproper: clean
	@rm $(LMMS_PKG)
