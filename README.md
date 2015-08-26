# Helgrind_uml
Apply helgrind on UML

## Environment
We tested our work on Ubuntu 7.10 (kernel version 2.6.22.5, same as UML).


## Download && Extract

Download all required directories and files from our repository, including:
- uml/linux-2.6.22.5: linux kernel codes in version 2.6.22.5
- valgrind-3.3.0: valgrind source codes in version 3.3.0
- po37258-linux-2.6.22.5.patch: patch on UML
- po37258-valgrind-3.3.0.patch: patch on Valgrind
- extra_patch_on_kernel.patch: the extra patch on UML 
- Slackware-12.2-root_fs.bz2: filesystem for UML

For Slackware-12.2-root_fs.bz2, run:
> bunzip2 ./Slackware-12.2-root_fs.bz2

> mv ./Slackware-12.2-root_fs ./uml/linux-2.6.22.5/Slackware-12.2-root_fs


## Patch
1. Patch linux
    
    > cd ./uml/linux-2.6.22.5
    
    > patch -p1 < ../../po37258-linux-2.6.22.5.patch
    
    > patch -p1 < ../../extra_patch_on_kernel.patch

2. Patch Valgrind

    > cd ./valgrind-3.3.0
    
    > patch -p1 < ../po37258-valgrind-3.3.0.patch

	> patch -p1 < ../../extra_patch_on_valgrind.patch


## Configure && Compile

1. Configure && Compile UML

    > cd ./uml/linux-2.6.22.5
    
    > make menuconfig ARCH=um
    
    > make linux ARCH=um

2. Compile Valgrind

    > cd ./valgrind-3.3.0

	>./configure
    
    > make
    
    > sudo make install


## Test


> cd ./uml/linux-2.6.22.5

> valgrind --tool=helgrind ./linux ubda=Slackware-12.2-root_fs ro mem=512m 2>| error 
