ccflags = ['-g', '-O3', '-Wall', '-Werror', '-std=c++14']
libs = ['sfml-graphics', 'sfml-window', 'sfml-system']
env = Environment(CCFLAGS=' '.join(ccflags), LIBS=libs)
env.Program('gvm', Glob('*.cc'))
