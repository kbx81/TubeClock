##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

# Build directory - relative to project root
BUILD_DIR = ../../build/v1

DEFS	+= -DHARDWARE_VERSION=1
BINARY = $(BUILD_DIR)/TubeClock
SRCS   = $(filter-out TubeClock.cpp, $(wildcard *.cpp))
# CFLAGS += -I./
# CXXFLAGS += -I./

OBJS  += $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))

OPENCM3_DIR=../../../../libopencm3
# LDSCRIPT = $(OPENCM3_DIR)/lib/stm32/f0/stm32f04xz6.ld
LDSCRIPT = $(OPENCM3_DIR)/lib/stm32/f0/stm32f07xzb.ld
# LDFLAGS += -Wl,--Map=$(BINARY).map

CXXSTD	 = -std=c++11

include ../../../../libopencm3.target.mk

# Make .bin the default target
all: $(BINARY).bin

# Generate binary file from ELF
$(BINARY).bin: $(BINARY).elf
	@printf "  OBJCOPY $@\n"
	$(Q)$(OBJCOPY) -O binary $< $@

# Override build rules to use BUILD_DIR
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@#printf "  CC      $<\n"
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -MF $(BUILD_DIR)/$*.d -o $@ -c $<

$(BUILD_DIR)/%.o: %.cxx | $(BUILD_DIR)
	@#printf "  CXX     $<\n"
	$(Q)$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -MF $(BUILD_DIR)/$*.d -o $@ -c $<

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@#printf "  CXX     $<\n"
	$(Q)$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -MF $(BUILD_DIR)/$*.d -o $@ -c $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: clean
clean:
	@#printf "  CLEAN\n"
	$(Q)$(RM) -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)
