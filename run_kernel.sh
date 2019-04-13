echo "killall -9 gvm; cd $HOME/gvm/gsm && go install && cd .. && gsm -o kernel.rom kernel.asm && scons -j6   && ./gvm --prgrom=kernel.rom --video_mode=720p"

killall -9 gvm; cd $HOME/gvm/gsm && go install && cd .. && gsm -o kernel.rom kernel.asm && scons -j6   && ./gvm --prgrom=kernel.rom --video_mode=720p
