# The GVM Virtual Machine

GVM is a software computer that I created as a way to learn assembly. It emulates a made up 32bit RISC(-ish) CPU with 32 registers and a small instruction set (ISA). The ISA was inspired by different CPUs (Armv7, MIPS, RISC-V) but was created with ease of use and understanding as the main objective.

In order to make the virtual machine useful, I emulated a video controller, video display and have started writing a basic kernel that is currently capable of displaying a text and bitmapped interface. I have also created an assembler called GSM that will output raw binaries for GVM.

This sofware is pre-alpha quality and there are many things missing that I want to support to make it a useful computer for learning and teaching assembly. Check the [list of issues](https://github.com/avalonbits/gvm/issues) if you want to help out.

## Compiling GVM

GVM is written in C++14 and has only been tested to work on Linux, both on Armv8 and x86_64 CPUs. I use [gcc](https://gcc.gnu.org/) 7.4 on [armbian](https://armbian.com) on a NanoPI-M4 as my main development machine, but any gcc version 6.1 and above should work. The CPU emulator uses a gcc extension called ["Labels as Values" or "computed gotos"](https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html) so unless your compiler of choice also supports it, stick with gcc.

Besides a c++ compiler, you will also need
  - [libsdl2](https://www.libsdl.org/download-2.0.php) which is used for user input reading and video display.
  - [SCons](https://scons.org) which is used to build the program.

With those dependencies in place you can compile gvm as follows:

```
$ cd gvm
~/gvm$ scons
```

This should give you a program called `gvm`. In order to do anything useful with it, you need a rom to load on startup.

## Compiling and running GSM

GSM is an assembler that can ouput a raw binary rom usable by GVM. The assembler is written in [Go](https://golang.org) and requires [Go 1.12](https://golang.org/dl/) or above. You can compile the assembler as follows:

```
$ cd gvm/gsm
~/gvm/gsm$ go install
```

This should give you a program called `gsm`, instaled in your $GOPATH/bin directory which you can then use to compile an assembly file. To compile the GVM kernel, do the following:

```
$ cd gvm
~/gvm$ gsm -o kernel.rom kernel.asm
```

This should give you a binary called `kernel.rom`. To use it with GSM:

```
$ cd gvm
~/gvm$ ./gvm --prgrom=kernel.rom --video_mode=720p
```

And now a 1280x720 window should show up with a text mode interface. Take a look at `kernel.asm`. It is heavily commented so should be fairly easy to follow. If you have difficulties or find a bug or what to add/update features, send me a pull request!

### run_kernel.sh
If you have all the dependencies in place (gcc, libsdl2, scons and go), you can do all the above steps in a single command:

```
$ cd gvm
~/gvm$ ./run_kernel.sh
```

## Test programs
In `gvm/perf` you will find a few programs that I use to benchmark or to test new features or bug fixes. To run these programs:

```
$ cd gvm
~/gvm$ BENCH=perf/program.asm ./run_bench.sh
```

This should run the program and output a few stats about it. For example, running the `gvm/perf/array32.asm` program on my NanoPI-M4 I get the following:

```
~/gvm$ BENCH=perf/array32.asm ./run_bench.sh 
gvm: no process found
scons: Reading SConscript files ...
scons: done reading SConscript files.
scons: Building targets ...
scons: `.' is up to date.
scons: done building targets.
[0x0 0x20000c 0x10000c 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0xf 0xf 0x0 0x0 0x0 0x0 0x101f000 0x101f000]
CPU Runtime: 980457us
CPU Instruction count: 83707964
Average per instruction: 11.7128ns
Average clock: 85.3765MHz
Timer elapsed: 980.5ms
```

`run_bench.sh` will use GSM to generate the rom and then use GVM to run the program. From the output, you can see the `scons` build output then a list of values which correspond to the final values of the 32 registers, the CPU runtime, instruction count, average time per instruction, average clock and the total time for the program.
