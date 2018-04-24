#!/bin/bash


echo "Probar ClckFreq.p"
./zesarux --noconfigfile --machine zx81 extras/media/zx81/tests/ClckFreq.p
ClckFreq.p

echo "Probar chroma81"
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/ALMINCOL.P
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/GalaxCol.P
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/MazogsColour.P
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/proftime.p
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/MISSICOL.P
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/stellar.p
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/wall-expanded.p
./zesarux --noconfigfile --machine zx81 --realvideo --chroma81 extras/media/zx81/Chroma_81/rebound/Rebound.p




