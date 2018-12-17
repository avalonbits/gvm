ccflags = ['-O3', '-g', '-Wall', '-Werror', '-std=c++14', '-march=native']
libs = ['sfml-graphics', 'SDL2', 'sfml-window', 'sfml-system', 'pthread']

env = Environment(CCFLAGS=' '.join(ccflags), LIBS=libs)
srcs = [
  'computer.cc', 'computer_roms.cc', 'cpu.cc', 'isa.cc', 'main.cc', 'rom.cc',
  'sdl2_video_display.cc', 'sfml_video_display.cc', 'video_controller.cc'
]
env.Program('gvm', srcs)
env.Program('display', ['display.cc'])
env.Program('texture', ['texture.cc'])

