# libmfimage
Library for reading / writing images from multiple file formats in a way suitable for color management and manipulating them.

The main purpose is reading image files, if suitableobeying color space instructions embedded in them in a coprehensible way and indicating if no color space instructions are included, so that the image loaded in memory can now be used in a color managed application.

There are also manipulation routines (flip, rotation, separation, combination, levels & gamma, brightness+contrast, alpha-channel manipulation, compositing obeying alpha-channel) for images available as well as single color value coversion routines for often used color space conversions (icc-profile <-> icc-profile, icc-profile <-> lab, icc-profile <-> sRGB, L*a*b* <-> icc-profile, RGB <-> HSV, L*a*b* <-> LCh).

Writing images is limitted to some formats, at least in the initial version. The best implemented format for writing images to disk is tiff, which by design allows flexible, lossless, compressed and multi-page images - everything one could ask for!

The original purpose of this library was to be the backend for image reading/writing/manipulation of the mfscan sane frontend.

# Features:
- reading/writing images from multiple file formats, inlcuding GIF, TIF, JPEG, JP2, PNG, BMP, PCX, TGA.
- When suitable, assigning color spaces to the image (e.g. PNG sRGB, JP2 sRGB, PNG cHRM/gAMA, embedded ICC profiles)
- Support to convert color spaces based on littleCms
- Using wide-spread standard and de-facto standard image format libraries like GIFLIB, libtiff-4, libjpeg-turbo, openJpeg, libpng.
- Wide range support for subformats (for example CMYK JPEG, TIF Compressions, ...)

# Language & Operating system:
The library is internally written in C++(11), but also will have an external C-interface to simplify usage from other languages and for Windows dlls. Its design goals are to be as operating system independant as possible, easy to use and maintain, high performance capable. And of course dealing with colors in the "right" way, allowing an ICC workflow to be built ontop of it.

# License
Dual-license. GPLv3, commercial license available.
