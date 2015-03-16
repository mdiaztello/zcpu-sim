

#------OUTPUTS-------#
COMPONENT_NAME = custom_sim
#Set this to keep the makefile quiet
SILENCE = @

#-----INPUTS---------#
PROJECT_HOME_DIR = .
CPPUTEST_HOME = /home/zorak/projects/CppUTest

CPP_PLATFORM = Gcc

SRC_DIRS = src
TEST_SRC_DIRS = tests
MOCKS_SRC_DIRS = mocks

SDL_LIB = -L/usr/local/lib -lSDL2 -Wl,-rpath=/usr/local/lib
#SDL_INCLUDE = -I/usr/local/include
SDL_INCLUDE = /usr/local/include

INCLUDE_DIRS =\
  .\
  include \
  $(CPPUTEST_HOME)/include/ \
  $(CPPUTEST_HOME)/include/Platforms/Gcc\
  mocks \
  $(SDL_INCLUDE)


CPPUTEST_WARNINGFLAGS = -Wall -Wswitch-default -Wno-write-strings

include $(CPPUTEST_HOME)/build/MakefileWorker.mk







# Simple makefile that will compile all the .c files in the current directory
# and make an executable called "main.exe" in the same directory
# to build the project, type "make" in the terminal
# to remove all the non-source files, type "make clean" in the terminal
CC := gcc

# catch issue additional warnings and make them into hard errors
#CFLAGS := -Wall -Wextra -Werror -std=c99
CFLAGS := -Wall -Wextra -std=c99

LDFLAGS = $(SDL_LIB)

simulator: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)


.PHONY: my_clean

my_clean:
	rm simulator

ctags:
	ctags src/*.c include/*.h



