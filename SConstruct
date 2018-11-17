ccflags = ['-g', '-O3', '-Wall', '-Werror', '-std=c++14']
libs = ['sfml-graphics', 'sfml-window', 'sfml-system']

envMain = Environment(CCFLAGS=' '.join(ccflags), LIBS=libs)
envMain.Program('gvm', ['cpu.cc', 'isa.cc', 'main.cc', 'rom.cc', 'video_display.cc'])

envDisplay = Environment(CCFLAGS=' '.join(ccflags), LIBS=libs)
envDisplay.Program('display', ['display.cc'])


