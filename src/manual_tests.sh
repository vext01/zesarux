#!/bin/bash

echo "z80 tests. errores de flags normalmente en: BIT n,(HL). errores de memptr en ninguno"
./zesarux --noconfigfile extras/media/spectrum/tests/z80tests.tap

echo "z80doc tests. van todos"
./zesarux --noconfigfile ./extras/media/spectrum/tests/z80doc.tap

echo "fusetest. en 48k, 128k. falla Floating bus. skipped read 3ffd, 7ffd. en +2a falla ademas LDIR, Floating bus skipped, bffd read failed. en 128k dado que lee puerto
paginacion, se pone pantalla en negro"
./zesarux --noconfigfile ./extras/media/spectrum/tests/fusetest.tap
./zesarux --noconfigfile --machine 128k ./extras/media/spectrum/tests/fusetest.tap

echo "Timing_Tests-48k_v1.0.sna. errores normalmente en test 4 add etc, test 25 add etc, test 36 in a etc, test 37 in a etc"
./zesarux --noconfigfile ./extras/media/spectrum/tests/Timing_Tests-48k_v1.0.sna

echo "Probar ClckFreq.p"
./zesarux --noconfigfile --machine zx81 extras/media/zx81/tests/ClckFreq.p

echo "Probar chroma81"
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/ALMINCOL.P
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/GalaxCol.P
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/MazogsColour.P
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/proftime.p
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/MISSICOL.P
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/stellar.p
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/wall-expanded.p
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/rebound/Rebound.p




