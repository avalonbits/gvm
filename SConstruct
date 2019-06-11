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

ccflags = ['-O3', '-g', '-Wall', '-Werror', '-std=c++14', '-march=native']

dstep = ARGUMENTS.get('dstep', '0')
if int(dstep):
  ccflags.append('-DDEBUG_DISPATCH')

handlers = ARGUMENTS.get('handlers', '0')
if handlers and int(handlers):
  ccflags.append('-DCPU_HANDLERS')

debug = ARGUMENTS.get('debug', '0')
if int(debug):
  ccflags.remove('-O3')
  ccflags.remove('-g')
  ccflags.append('-g3')
  ccflags.append('-gdwarf')

libs = ['SDL2', 'pthread']

env = Environment(CCFLAGS=' '.join(ccflags), LIBS=libs)
srcs = [
  'computer.cc', 'cpu.cc', 'disk.cc', 'disk_controller.cc', 'gfs.cc',
  'input_controller.cc', 'isa.cc', 'main.cc', 'rom.cc', 'sdl2_video_display.cc',
  'timer.cc', 'video_controller.cc'
]
env.Program('gvm', srcs)

if int(ARGUMENTS.get('display', 0)):
  senv = Environment(CCFLAGS=' '.join(ccflags),
                     LIBS=['pthread', 'sfml-graphics', 'sfml-window', 'sfml-system'])
  senv.Program('display', ['display.cc'])
