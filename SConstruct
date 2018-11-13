ccflags = ['-g', '-O3', '-Wall', '-Werror', '-std=c++14']
libs = ['SDL2']
env = Environment(CCFLAGS=' '.join(ccflags), LIBS=libs)
env.Program('gvm', Glob('*.cc'))
