ccflags = ['-g', '-O3', '-Wall', '-Werror', '-std=c++14']
libs = ['sfml-graphics', 'sfml-window', 'sfml-system', 'pthread']

env = Environment(CCFLAGS=' '.join(ccflags), LIBS=libs)
env.Program('gvm', ['computer.cc', 'cpu.cc', 'isa.cc', 'main.cc', 'rom.cc', 'video_display.cc'])
env.Program('display', ['display.cc'])
env.Program('texture', ['texture.cc'])

