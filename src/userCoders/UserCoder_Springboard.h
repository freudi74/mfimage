#pragma once

// additional includes go here - include only headers to classes that are derived from ImageCoder

#define IE_DCP IE_USER_1
#define CREATE_USER_CODER_1(IMAGE) (new ImageCoderDCP(IMAGE))
#define IS_USER_CODEC_1(byteArray4, istreamptr) ( ImageCoderDCP::isCodec(byteArray4, istreamptr) )
 
#define IE_DPEG IE_USER_2
#define CREATE_USER_CODER_2(IMAGE) (new ImageCoderDPEG(IMAGE))
#define IS_USER_CODEC_2(byteArray4, istreamptr) ( ImageCoderDPEG::isCodec(byteArray4, istreamptr) )
