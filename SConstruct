ccflags = ['-g', '-O3', '-Wall', '-Werror', '-std=c++14',
           '-fno-builtin-malloc', '-fno-builtin-calloc',
           '-fno-builtin-realloc', '-fno-builtin-free']

libs = ['tcmalloc', 'profiler']
env = Environment(CCFLAGS=' '.join(ccflags), LIBS=libs) 
env.Program('gvm', Glob('*.cc'))
