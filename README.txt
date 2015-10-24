Requires GCC 4.9 / VS2013 due to some c++11/14 features.

Dependencies:
--------------
GIFLib 4.1.6 (libgif)
LibJPegTurbo62 (libjpeg)
LibTiff-4 (libtiff)
LibPNG 1.6 (libpng)
LittleCMS2.6 (lcms2)

GIFLIb 4 is NOT reentrant, all calls are serialzed by a mutex, so the en/decoder is reentrant.

Reentrance status of LibJpegTurbo ?
Reentrance status of LibTiff ?
Reentrance status of LibPng ?

LittleCMs 2.6 is reentrant.




