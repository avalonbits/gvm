#!/bin/bash

# Copyright (C) 2019  Igor Cananea <icc@avalonbits.com>
# Author: Igor Cananea <icc@avalonbits.com>
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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

: "${BENCH:=benchmark.asm}"
: "${CPU:=0}"
: "${DEBUG:=0}"
: "${CXX:=0}
: "${M:=6}
killall -9 gvm; cd $HOME/gvm/gsm && go install && cd .. && time gsm -o bench.rom $BENCH && scons -j$M dstep=$DEBUG handlers=$CPU clang=$CXX  && time ./gvm --prgrom=bench.rom --video_mode=null
