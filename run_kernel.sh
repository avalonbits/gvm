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

: "${VMODE:=450p}"
: "${CPU:=0}"
: "${DISK_FILE:=/tmp/gvm.hd}"
: "${DSTEP:=0}"
: "${KERNEL:=kernel.asm}"
CMD="killall -9 gvm; cd $HOME/gvm/gsm && go install && cd .. && gsm -o kernel.rom $KERNEL && scons -j6 dstep=$DSTEP  && ./gvm --prgrom=kernel.rom --video_mode=$VMODE --disk_file=$DISK_FILE"

echo $CMD
eval $CMD
