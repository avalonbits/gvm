: "${BENCH:=benchmark.asm}"
: "${CPU:=0}"
killall -9 gvm; cd $HOME/gvm/gsm && go install && cd .. && gsm -o bench.rom $BENCH && scons -j6 handlers=$CPU  && ./gvm --prgrom=bench.rom --video_mode=null
