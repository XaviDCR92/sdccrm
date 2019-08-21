# Copyright (C) 2019  Xavier Del Campo Romero <xavi.dcr@tutanota.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Make flags
MAKEFLAGS := -j$(NUMBER_OF_PROCESSORS)

# Toolchain definitions
CC = gcc
LD = gcc

MKDIR = mkdir

# Compiler flags
INCLUDE = -Iinc/
CC_FLAGS = -O0 -g $(INCLUDE) -Wall -Wextra -Wpedantic -c
LD_FLAGS = $(INCLUDE)
# Linker flags
LIBS =

PROJECT = sdccrm

# Objects definition
# Compiled objects list
OBJ_DIR = obj
SRC_DIR = src
OBJECTS = $(addprefix $(OBJ_DIR)/, \
	sdccrm.o function_list.o references.o common.o options.o \
	remove_unused.o alloc.o)

# Source dependencies:
DEPS = $(OBJECTS:.o=.d)

# ------------------------------------
# Instructions
# ------------------------------------

$(PROJECT): $(OBJECTS)
	$(LD) $^ -o $@ $(LD_FLAGS) $(LIBS)

clean:
	rm -f $(OBJ_DIR)/*.o

deps: $(DEPS)

-include $(DEPS)

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c
	@$(MKDIR) -p $(OBJ_DIR)
	$(CC) $< $(DEFINE) $(CC_FLAGS) -MM > $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR)/%.d
	@$(MKDIR) -p $(OBJ_DIR)
	$(CC) $< -o $@ $(DEFINE) $(CC_FLAGS) -MMD

# ----------------------------------------
# Phony targets
# ----------------------------------------
.PHONY: deps clean
