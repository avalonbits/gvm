ccflags = ['-O3', '-g', '-Wall', '-Werror', '-std=c++14', '-march=native']

dstep = ARGUMENTS.get('dstep', 0)
if int(dstep):
  ccflags.append('-DDEBUG_DISPATCH')

debug = ARGUMENTS.get('debug', 0)
if int(dstep) or int(debug):
  ccflags.remove('-O3')
  ccflags.remove('-g')
  ccflags.append('-g3')
  ccflags.append('-gdwarf')

libs = ['SDL2', 'pthread']

env = Environment(CCFLAGS=' '.join(ccflags), LIBS=libs)
srcs = [
  'computer.cc', 'computer_roms.cc', 'cpu.cc', 'isa.cc', 'main.cc', 'rom.cc',
  'sdl2_video_display.cc', 'ticker.cc', 'video_controller.cc'
]
env.Program('gvm', srcs)

if int(ARGUMENTS.get('display', 0)):
  senv = Environment(CCFLAGS=' '.join(ccflags),
                     LIBS=['pthread', 'sfml-graphics', 'sfml-window', 'sfml-system'])
  senv.Program('display', ['display.cc'])
