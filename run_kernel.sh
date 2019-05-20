: "${VMODE:=900p}"
: "${CPU:=0}"
: "${DISK_FILE:=/tmp/gvm.hd}"
CMD="killall -9 gvm; cd $HOME/gvm/gsm && go install && cd .. && gsm -o kernel.rom kernel.asm && scons -j6   && ./gvm --prgrom=kernel.rom --video_mode=$VMODE --disk_file=$DISK_FILE"

echo $CMD
eval $CMD
