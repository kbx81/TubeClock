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

DEFS	+= -DHARDWARE_VERSION=1
BINARY = TubeClock
SRCS   = $(filter-out TubeClock.cpp, $(wildcard *.cpp))
# CFLAGS += -I./
# CXXFLAGS += -I./

OBJS  += $(SRCS:.cpp=.o)

OPENCM3_DIR=../../../../libopencm3
# LDSCRIPT = $(OPENCM3_DIR)/lib/stm32/f0/stm32f04xz6.ld
LDSCRIPT = $(OPENCM3_DIR)/lib/stm32/f0/stm32f07xzb.ld
# LDFLAGS += -Wl,--Map=$(BINARY).map

CXXSTD	 = -std=c++11

include ../../../../libopencm3.target.mk
