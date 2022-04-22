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

LMMS_PKG=lmms-pkg
BUILD_APPIMG_TOOL=./build-appimage.sh
APPIMG_DIR=$(LMMS_PKG).AppDir/
APPIMAGE_PROG=$(LMMS_PKG)-x86_64.AppImage

WFLAGS=-Wall -Wextra

ifeq ($(DEBUG),yes)

	# Debug mode
	CFLAGS=-fPIC $(WFLAGS) -std=c++17 -g -fsanitize=address
	OPTIMIZE=-O0
	OPT_SIZE=

else

	# Release mode
	CFLAGS=-w -std=c++17
	OPTIMIZE=-O3
	OPT_SIZE=-s

endif


.PHONY: clean mrproper appimage

SRCS=$(wildcard */*.cpp) $(wildcard */*/*.cpp) $(wildcard */*/*/*.cpp)
OBJS=$(SRCS:.cpp=.o)

%.o: %.cpp
	@echo $@" - Compiling "$<
	$(CC) -c $< -o $@

$(LMMS_PKG): $(OBJS)
	@echo "Create "$@
	@$(CC) -o $@ $(OBJS)

appimage: $(LMMS_PKG)
	$(BUILD_APPIMG_TOOL) $(LMMS_PKG)
	@chmod 755 $(APPIMAGE_PROG)

clean:
	@find $(SRC_DIR) -name '*.o' -delete

mrproper: clean
	@rm -rf $(LMMS_PKG) $(APPIMG_DIR) $(APPIMAGE_PROG)
