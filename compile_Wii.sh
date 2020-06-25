#!/bin/bash


function build_expack {
	make clean
	find . -name '*.o' -print0 | xargs -0r rm -f

	[ ! -f Makefile ] && ./autogen.sh
	./configure --disable-alsa
	cd files && make libu7file.la
	cd ..
	cd tools && make expack
	cp expack ../
	cd ../

	cd files && make clean
	cd ..
}


function build_dol {
	make -f Makefile.wii -j5
	cp exult boot.elf
	${DEVKITPPC}/bin/powerpc-eabi-strip 'exult' && mv exult boot.dol
}


if [ -f "boot.dol" ]; then
	echo "## Build Exult Wii dol... ##"
	build_dol
else
	echo "## Build expack...(PC tool needed for compilation) ##"
	build_expack
	build_dol
fi
