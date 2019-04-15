BENCH=${BENCH:-benchmark.asm}
killall -9 gvm; cd $HOME/gvm/gsm && go install && cd .. && gsm -o bench.rom $BENCH && scons -j6   && ./gvm --prgrom=bench.rom --video_mode=null
