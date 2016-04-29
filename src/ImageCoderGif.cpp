/* ImageCoderGif.cpp
 * Copyright 2015 Marco Freudenberger.
 * 
 * This file is part of mfimage library.
 * 
 * Design and implementation by:
 * --------------------------------------------------
 * - Marco Freudenberger (Marco.Freudenberger@gmx.de)
 *
 */

#include "stdafx.h"
#include "ImageCoderGif.h"
#ifdef _WIN32
#  include <gif_lib.h>
#else
#  include "../temp/gif_lib5.h"
#endif
/*static*/ std::mutex ImageCoderGif::s_giflibMutex;

ImageCoderGif::ImageCoderGif(Image* img) : ImageCoder(IE_GIF, img)
{
}

ImageCoderGif::~ImageCoderGif()
{
}


bool ImageCoderGif::canEncode()
{
	return false;
}

bool ImageCoderGif::canDecode()
{
	return true;
}
bool ImageCoderGif::canEncode( PixelMode pixelMode )
{
	return false;
}

static const int InterlacedOffset[] = { 0, 4, 2, 1 }; /* The way Interlaced image should. */
static const int InterlacedJumps[]  = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */

static const int readGifData(GifFileType *, GifByteType *, int)
{
	return 0;	// todo, once we switched to newer GLIB version!
}

void ImageCoderGif::read(std::istream& stream)
{
	// hardcore for now --- write to temp file and read from that file.
	// unfortunately, in GIFLIB 5.1.1 and 5.1.2 there is an error in DGifOpen( to be used with a read function )
	// so we can't use that before switching to a new GIFLIB version...
	// 

	std::string tempFileName = storeStreamToTempFile( stream );
	read(tempFileName);
}


void ImageCoderGif::read(const std::string& filename)
{
	
	std::lock_guard<std::mutex> lock( s_giflibMutex );
	
	auto fileCloser = [](FILE* p){ /*if (p!=0) fclose(p);*/ };
	std::unique_ptr<FILE, decltype(fileCloser)> pFile( fopen(filename.c_str(), "rb"), fileCloser );
	if ( !pFile )
	{
		throw std::runtime_error("ImageCoderGif: Could not open file!");
	}
	
	int gifErrorCode = 0;
	auto gifFileCloser = [&gifErrorCode]( GifFileType* gif ){ DGifCloseFile(gif, &gifErrorCode); };
#ifdef _WIN32
	int fd = ::_fileno(pFile.get());
#else
	int fd = ::fileno(pFile.get());
#endif
	std::unique_ptr<GifFileType, decltype(gifFileCloser)> gif( DGifOpenFileHandle(fd,&gifErrorCode), gifFileCloser );

	if ( !gif )
	{
		throw std::runtime_error("ImageCoderGif: Failed to open GIF: " + std::string(GifErrorString(gifErrorCode))/*getLastErrorText()*/ );
	}
	
	image->create( gif->SWidth, gif->SHeight, PixelMode::ARGB8, 72.0, 72.0 );
	
	// read content of GIF
	GifRecordType recordType;
	size_t frameNo = 0;
	GraphicsControlExtension gce;
	bool continueToRead = true;
	do
	{
		if ( GIF_ERROR == DGifGetRecordType( gif.get(), &recordType) )
		{
			throw std::runtime_error("ImageCoderGif: Failed to read GIF record type: "+std::string(GifErrorString(gif->Error)) );
			
		}

		switch (recordType) 
		{
		case EXTENSION_RECORD_TYPE:
			{
				int extCode;
				GifByteType *extension;
				/* Skip any extension blocks in file: */
				
				if ( GIF_ERROR == DGifGetExtension( gif.get(), &extCode, &extension) ) 
					throw std::runtime_error("ImageCoderGif: Error reading GIF Extension: "+ std::string(GifErrorString(gif->Error)) );
				
				bool graphicsExtBlock = false;
				int blockNo = 0;
				switch (extCode) 
				{
				case COMMENT_EXT_FUNC_CODE:
					break;
				case GRAPHICS_EXT_FUNC_CODE:
					graphicsExtBlock = true;
					break;
				case PLAINTEXT_EXT_FUNC_CODE:
					break;
				case APPLICATION_EXT_FUNC_CODE:
					break;
				}				
				while ( extension != NULL) 
				{				
  					if ( graphicsExtBlock )
						handleGraphicsExtensionBlock( extension, blockNo, &gce );
  					blockNo++;
					
					if ( GIF_ERROR == DGifGetExtensionNext(gif.get(), &extension) )
						throw std::runtime_error("ImageCoderGif: Error reading GIF Extension: "+ std::string(GifErrorString(gif->Error)) );
				}	
			}
			break;
			
		case IMAGE_DESC_RECORD_TYPE:
			{
				if ( frameNo == 0 )
				{
					// set background !
					uint32_t bgARGB;
					if ( gce.transparencyFlag && gif->SBackGroundColor == gce.transparentColorIndex )
					{
						bgARGB = 0x00ffffff; // transparent white!
					}
					else
					{
						GifColorType bgColor = gif->SColorMap->Colors[gif->SBackGroundColor];
						bgARGB = 0xff000000 | bgColor.Red << 16 | bgColor.Green << 8 | bgColor.Blue;
					}
					image->fill( &bgARGB );					
				}
				if ( GIF_ERROR == DGifGetImageDesc(gif.get()) )
					throw std::runtime_error("ImageCoderGif: Failed to read GIF Image Desc: "+std::string(GifErrorString(gif->Error)) );

				int frameTop = gif->Image.Top;
				int frameLeft = gif->Image.Left;
				int frameWidth  = gif->Image.Width;
				int frameHeight = gif->Image.Height;
				++frameNo;
				
	//			GifQprintf("\n%s: Image %d at (%d, %d) [%dx%d]:     ",
	//				PROGRAM_NAME, ++frameNo, Col, Row, Width, Height);
				if ( (frameLeft + frameWidth)  > gif->SWidth || (frameTop + frameHeight) > gif->SHeight ) 
					throw std::runtime_error("ImageCoderGif: Frame "+std::to_string(frameNo)+" is not confined to screen dimension, aborted." );

				std::unique_ptr<GifPixelType[]> gifLine( new GifPixelType[frameWidth] );
				GifColorType* colorTable = (nullptr != gif->Image.ColorMap) ? gif->Image.ColorMap->Colors : gif->SColorMap->Colors;
				if ( colorTable == nullptr )
					throw std::runtime_error("ImageCoderGif: Gif Image does not have a colormap." );
					
				if ( gif->Image.Interlace ) 
				{
					/* Need to perform 4 passes on the images: */
					for ( size_t iPass=0; iPass<4; iPass++ )
					{
						for ( int y = frameTop + InterlacedOffset[iPass]; y < frameTop + frameHeight; y += InterlacedJumps[iPass] )
						{
							if ( GIF_ERROR == DGifGetLine( gif.get(), gifLine.get(), frameWidth ) )
								throw std::runtime_error("ImageCoderGif: Failed to get next line of GIF: "+std::string(GifErrorString(gif->Error)) );
							for ( int x = frameLeft; x < frameLeft + frameWidth; x++ )
							{
								unsigned char* imgPixel = reinterpret_cast<unsigned char*>(image->getPixel( y, x ));
								GifPixelType colorIndex = gifLine.get()[x];						
								if ( gce.transparencyFlag && colorIndex == gce.transparentColorIndex )
									*imgPixel++ = 0x00;	// transparent
								else
									*imgPixel++ = 0xff;	// opaque								
								*imgPixel++ = colorTable[colorIndex].Red;
								*imgPixel++ = colorTable[colorIndex].Green;
								*imgPixel++ = colorTable[colorIndex].Blue;
							}
						}
					}
				}
				else {
					for ( int y = frameTop; y < frameTop + frameHeight; y ++ )
					{
						if ( GIF_ERROR == DGifGetLine( gif.get(), gifLine.get(), frameWidth ) )
							throw std::runtime_error("ImageCoderGif: Failed to get next line of GIF: "+std::string(GifErrorString(gif->Error)) );
						for ( int x = frameLeft; x < frameLeft + frameWidth; x++ )
						{
							unsigned char* imgPixel = reinterpret_cast<unsigned char*>(image->getPixel( y, x ));
							GifPixelType colorIndex = gifLine.get()[x];							
							if ( gce.transparencyFlag && colorIndex == gce.transparentColorIndex )
								*imgPixel++ = 0x00;	// transparent
							else
								*imgPixel++ = 0xff;	// opaque								
							*imgPixel++ = colorTable[colorIndex].Red;
							*imgPixel++ = colorTable[colorIndex].Green;
							*imgPixel++ = colorTable[colorIndex].Blue;
						}
					}
				}
				continueToRead = onSubImageRead(frameNo);	// we have read image #frameNo
			}
			break;
			
		case TERMINATE_RECORD_TYPE:
			continueToRead = false;
			break;
		
		default:		    /* Should be traps by DGifGetRecordType. */
			break;
		}
    }
    while ( continueToRead );
	onSubImageRead(0);	// we are done!
}

/* The extension block is a block of bytes. It contains 5 bytes of data (including length).
 * 
 */
void ImageCoderGif::handleGraphicsExtensionBlock(void* extension, int blockNo, GraphicsControlExtension* gce )
{
	unsigned char * gceBin = reinterpret_cast<unsigned char *>(extension);
	size_t len = gceBin[0];
	if ( len != 4 || blockNo != 0 )
		throw std::runtime_error( "ImageCoderGif: Unexpected GraphicsControlExtension: blockNo: " + std::to_string(blockNo) + " len: " + std::to_string(len) );
	gce->transparencyFlag = ( gceBin[1] & 0x01 ) ? true : false;
	gce->userInputFlag    = ( gceBin[1] & 0x02 ) ? true : false;
	gce->disposalMethod   = ( gceBin[1] & 0x1c ) >> 2; // 0-7
	gce->reserved         = ( gceBin[1] & 0xe0 ) >> 5; // 0-7
	gce->delay10ms        = ( gceBin[3] << 8 ) | gceBin[2];
	gce->transparentColorIndex = gceBin[4];
}

void ImageCoderGif::write(const std::string& filename )
{
	std::lock_guard<std::mutex> lock( s_giflibMutex );
	throw std::runtime_error("writing GIF files not implemented");
}

void ImageCoderGif::write(std::ostream& stream)
{
	std::lock_guard<std::mutex> lock(s_giflibMutex);
	throw std::runtime_error("writing GIF files not implemented");
}
/*
std::string ImageCoderGif::getLastErrorText()
{
	// this is code copy & paste & modified from GIFLIB's "gif_error". Problem is, that Giflib only PRINTS errors to stderr !
	
	int gifLastError = GifLastError(); // clears the error at the same time!
	std::string text;
	
	switch (gifLastError) {
    case E_GIF_ERR_OPEN_FAILED:    text = "Failed to open given file"; break;
    case E_GIF_ERR_WRITE_FAILED:   text = "Failed to Write to given file"; break;
    case E_GIF_ERR_HAS_SCRN_DSCR:  text = "Screen Descriptor already been set"; break;
    case E_GIF_ERR_HAS_IMAG_DSCR:  text = "Image Descriptor is still active"; break;
	case E_GIF_ERR_NO_COLOR_MAP:   text = "Neither Global Nor Local color map"; break;
    case E_GIF_ERR_DATA_TOO_BIG:   text = "#Pixels bigger than Width * Height"; break;
    case E_GIF_ERR_NOT_ENOUGH_MEM: text = "Fail to allocate required memory"; break;
    case E_GIF_ERR_DISK_IS_FULL:   text = "Write failed (disk full?)"; break;
    case E_GIF_ERR_CLOSE_FAILED:   text = "Failed to close given file"; break;
    case E_GIF_ERR_NOT_WRITEABLE:  text = "Given file was not opened for write"; break;
    case D_GIF_ERR_OPEN_FAILED:    text = "Failed to open given file"; break; 
    case D_GIF_ERR_READ_FAILED:    text = "Failed to Read from given file"; break;
	case D_GIF_ERR_NOT_GIF_FILE:   text = "Given file is NOT GIF file"; break;
	case D_GIF_ERR_NO_SCRN_DSCR:   text = "No Screen Descriptor detected"; break;
    case D_GIF_ERR_NO_IMAG_DSCR:   text = "No Image Descriptor detected"; break;
    case D_GIF_ERR_NO_COLOR_MAP:   text = "Neither Global Nor Local color map"; break;
    case D_GIF_ERR_WRONG_RECORD:   text = "Wrong record type detected"; break;
    case D_GIF_ERR_DATA_TOO_BIG:   text = "#Pixels bigger than Width * Height"; break;
    case D_GIF_ERR_NOT_ENOUGH_MEM: text = "Fail to allocate required memory"; break;
    case D_GIF_ERR_CLOSE_FAILED:   text = "Failed to close given file"; break;
    case D_GIF_ERR_NOT_READABLE:   text = "Given file was not opened for read"; break;
    case D_GIF_ERR_IMAGE_DEFECT:   text = "Image is defective, decoding aborted"; break;
    case D_GIF_ERR_EOF_TOO_SOON:   text = "Image EOF detected, before image complete"; break;
    default: 
		text = "GIF-LIB undefined error " + std::to_string(gifLastError );
		break;
    }
	return text;
}
*/