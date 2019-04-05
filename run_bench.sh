killall -9 gvm; cd $HOME/gvm/gsm && go install && cd .. && gsm -o bench.rom benchmark.asm && scons -j6   && ./gvm --prgrom=bench.rom --video_mode=null
