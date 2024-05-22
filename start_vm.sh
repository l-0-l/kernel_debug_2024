#!/bin/sh

qemu-system-x86_64 \
    -hda debian12.qcow2 \
    -kernel /home/me/work/meetup/linux-6.8.9/arch/x86/boot/bzImage \
    -append "root=/dev/sda1 ramoops.mem_address=0x140000000 ramoops.mem_size=204800 ramoops.ecc=1 console=ttyS0,115200n8 net.ifnames=0 crashkernel=384M-:128M randomize_kstack_offset=off nokaslr schedstats=enable" \
    -m 4G \
    -smp cpus=2 \
    -cpu host \
    -boot d \
    -s \
    -enable-kvm \
    -virtfs local,path=/home/me/work/,mount_tag=hostshare,security_model=mapped,id=hostshare \
    -netdev user,id=net0,hostfwd=tcp::2222-:22 \
    -device e1000,netdev=net0 \
    -nographic

#    -initrd /boot/initrd.img-6.1.0-20-amd64 \
