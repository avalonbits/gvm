ccflags = ['-O3', '-g', '-Wall', '-Werror', '-std=c++14', '-march=native']
libs = ['SDL2', 'pthread']

env = Environment(CCFLAGS=' '.join(ccflags), LIBS=libs)
srcs = [
  'computer.cc', 'computer_roms.cc', 'cpu.cc', 'isa.cc', 'main.cc', 'rom.cc',
  'sdl2_video_display.cc', 'video_controller.cc'
]
env.Program('gvm', srcs)

senv = Environment(CCFLAGS=' '.join(ccflags),
                   LIBS=['pthread', 'sfml-graphics', 'sfml-window', 'sfml-system'])
senv.Program('display', ['display.cc'])
