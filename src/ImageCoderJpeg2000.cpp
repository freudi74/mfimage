/* ImageCoderJpeg.cpp
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
#include "ImageCoderJpeg2000.h"
#include "ColorManager.h"
#include <openjpeg-2.1/openjpeg.h>
#include <fstream>

ImageCoderJpeg2000::ImageCoderJpeg2000(Image* img) : ImageCoder(IE_JPEG2000, img)
{
}

ImageCoderJpeg2000::~ImageCoderJpeg2000()
{
}


bool ImageCoderJpeg2000::canEncode()
{
	return false;
}
bool ImageCoderJpeg2000::canDecode()
{
	return true;
}
bool ImageCoderJpeg2000::canEncode( PixelMode pixelMode )
{
	return false;
}

static OPJ_SIZE_T streamReadFct( void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * pUserStream )
{
	std::iostream* stream = reinterpret_cast<std::iostream*>(pUserStream);
	if ( stream->eof() )
		return -1;
	stream->read( reinterpret_cast<char*>(p_buffer), p_nb_bytes );
	if ( stream->fail() && !stream->eof() )
		return -1;
	return stream->gcount();
}
static OPJ_OFF_T streamSkipFct( OPJ_OFF_T p_nb_bytes, void * pUserStream )
{
	std::iostream* stream = reinterpret_cast<std::iostream*>(pUserStream);
	int posBefore = stream->tellg();
	stream->seekg( p_nb_bytes, std::ios_base::cur );
	if ( stream->fail() )
		return -1;
	int posAfter = stream->tellg();
	return posAfter - posBefore;
}
static OPJ_BOOL streamSeekFct( OPJ_OFF_T p_nb_bytes, void * pUserStream )
{
	std::iostream* stream = reinterpret_cast<std::iostream*>(pUserStream);
	stream->seekg( p_nb_bytes );
	return stream->fail() ? OPJ_FALSE : OPJ_TRUE;
}

void ImageCoderJpeg2000::read(const std::string & filename )
{
	std::fstream file( filename.data(), std::ios::in|std::ios::binary );
	file.seekg(0, std::ios_base::end );
	size_t fileLength = file.tellg();
	file.seekg(0);
	if ( file.fail() )
 	{
		throw std::runtime_error( "Failed to open file: " + filename );
	}
	opj_dparameters_t parameters;			/* decompression parameters */
	
	
	///////////////////////////////////////////////
	// READ JPEG 2000 FILE using libopenjpeg 2.1
	///////////////////////////////////////////////
	try
	{
		
		opj_image_t* opj_image = NULL;
		opj_stream_t *l_stream = NULL;				/* Stream */
		opj_codec_t* l_codec = NULL;				/* Handle to a decompressor */
//		opj_codestream_index_t* cstr_index = NULL;

		int failed = 0;



		/* set decoding parameters to default values */
		opj_set_default_decoder_parameters(&parameters);

#if 1
		/* read the input file and put it in memory */
		/* ---------------------------------------- */
		l_stream = opj_stream_create_default_file_stream( filename.data(), 1 );
		if (!l_stream){
			throw std::runtime_error( "OpenJPEG: failed to create the stream from the file " + filename );
		}
#else		
		l_stream = opj_stream_default_create( true );
		if (!l_stream){
			throw std::runtime_error( "OpenJPEG: failed to create the stream from the file " + filename );
		}
//		l_stream = opj_stream_create( 16*1024, true );
		opj_stream_set_read_function( l_stream, streamReadFct );
		opj_stream_set_seek_function( l_stream, streamSeekFct );
		opj_stream_set_skip_function( l_stream, streamSkipFct );
		opj_stream_set_user_data( l_stream, &file, nullptr );
		opj_stream_set_user_data_length( l_stream, fileLength );
#endif

		/* decode the JPEG2000 stream */
		/* ---------------------- */

		int format = 2; // todo: find the format from file

		switch( format /*parameters.decod_format*/) {
			case 1: //J2K_CFMT:	/* JPEG-2000 codestream */
			{
				/* Get a decoder handle */
				l_codec = opj_create_decompress(OPJ_CODEC_J2K);
				break;
			}
			case 2: //JP2_CFMT:	/* JPEG 2000 compressed image data */
			{
				/* Get a decoder handle */
				l_codec = opj_create_decompress(OPJ_CODEC_JP2);
				break;
			}
			case 3: //JPT_CFMT:	/* JPEG 2000, JPIP */
			{
				/* Get a decoder handle */
				l_codec = opj_create_decompress(OPJ_CODEC_JPT);
				break;
			}
			default:
				opj_stream_destroy(l_stream);
				throw std::runtime_error( "OpenJPEG: failed to determine file format from the file " + filename );
		}

		/* catch events using our callbacks and give a local context */		
		// todo: handlers
//		opj_set_info_handler(l_codec, info_callback,00);
//		opj_set_warning_handler(l_codec, warning_callback,00);
//		opj_set_error_handler(l_codec, error_callback,00);

		/* Setup the decoder decoding parameters using user parameters */
		if ( !opj_setup_decoder(l_codec, &parameters) ){
			opj_stream_destroy(l_stream);
			opj_destroy_codec(l_codec);
			throw std::runtime_error( "OpenJPEG: ERROR -> opj_compress: failed to setup the decoder for the file " + filename );
		}


		/* Read the main header of the codestream and if necessary the JP2 boxes*/
		if(! opj_read_header(l_stream, l_codec, &opj_image))
		{
			opj_stream_destroy(l_stream);
			opj_destroy_codec(l_codec);
			opj_image_destroy(opj_image);
			throw std::runtime_error( "OpenJPEG: ERROR -> opj_compress: opj_decompress: failed to read the header from the file " + filename );
		}

		if (!parameters.nb_tile_to_decode) 
		{
			/* Optional if you want decode the entire image */
			if (!opj_set_decode_area(l_codec, opj_image, 
					(OPJ_INT32)parameters.DA_x0, (OPJ_INT32)parameters.DA_y0, 
					(OPJ_INT32)parameters.DA_x1, (OPJ_INT32)parameters.DA_y1))
			{
				opj_stream_destroy(l_stream);
				opj_destroy_codec(l_codec);
				opj_image_destroy(opj_image);
				throw std::runtime_error( "OpenJPEG: ERROR -> opj_decompress: failed to set the decoded area for the file " + filename );
			}

			/* Get the decoded image */
			if (!(opj_decode(l_codec, l_stream, opj_image) && opj_end_decompress(l_codec,	l_stream))) 
			{
				fprintf(stderr,"ERROR -> opj_decompress: failed to decode image!\n");
				opj_destroy_codec(l_codec);
				opj_stream_destroy(l_stream);
				opj_image_destroy(opj_image);
				throw std::runtime_error( "OpenJPEG: ERROR -> opj_decompress: failed to decode image for the file " + filename );
			}
		}
		else {

			/* It is just here to illustrate how to use the resolution after set parameters */
			/*if (!opj_set_decoded_resolution_factor(l_codec, 5)) {
				fprintf(stderr, "ERROR -> opj_decompress: failed to set the resolution factor tile!\n");
				opj_destroy_codec(l_codec);
				opj_stream_destroy(l_stream);
				opj_image_destroy(image);
				return EXIT_FAILURE;
			}*/

			if (!opj_get_decoded_tile(l_codec, l_stream, opj_image, parameters.tile_index)) {
				opj_destroy_codec(l_codec);
				opj_stream_destroy(l_stream);
				opj_image_destroy(opj_image);
				throw std::runtime_error( "OpenJPEG: ERROR -> opj_decompress: failed to decode tile " + std::to_string(parameters.nb_tile_to_decode) + " for the file " + filename );
			}
		}

		/* Close the byte stream */
		opj_stream_destroy(l_stream);

		/* free remaining structures */
		if (l_codec) {
			opj_destroy_codec(l_codec);
		}

// todo 
//
//		if(image->color_space == OPJ_CLRSPC_SYCC){
//			color_sycc_to_rgb(image); /* FIXME */
//		}
		

		
		bool isRGB = false;
		bool isCMYK = false;
		bool isGray = false;
		uint32_t bpc = 8;
		bool hasAlpha = true;
		size_t numComps = 0;
		switch ( opj_image->color_space ) {
		case OPJ_CLRSPC_SRGB:
		case OPJ_CLRSPC_SYCC: // YUV
		case OPJ_CLRSPC_EYCC: // e-YCC
			isRGB = true;
			numComps = 3;
			break;
		case OPJ_CLRSPC_GRAY:
			isGray = true;
			numComps = 1;
			break;
		case OPJ_CLRSPC_CMYK:
			isCMYK = true;
			numComps = 4;
			break;		
		case OPJ_CLRSPC_UNKNOWN: 
			// colorspace is not always clearly defined, typically for non-sRGB RGB color spaces ...
			if ( opj_image->numcomps == 1 )
			{
				isGray=true;
				numComps = 1;
				break;
			}
			else if ( opj_image->numcomps == 3 )
			{
				isRGB=true;
				numComps = 3;
				break;				
			}
			// nobreak
			
		case OPJ_CLRSPC_UNSPECIFIED:
		default:	
			opj_image_destroy(opj_image);
			throw std::runtime_error( "Unsupported or unspecified color space in image " + filename );
		}
		
		size_t width  = opj_image->x1 - opj_image->x0;
		size_t height = opj_image->y1 - opj_image->y0;
		
		props->offsetPixels[0] = opj_image->x0;
		props->offsetPixels[1] = opj_image->y0;
		
		uint32_t bppMax = 0;
		for ( size_t iComp=0; iComp<numComps; iComp++ )
		{
			if ( opj_image->comps[iComp].w != width || opj_image->comps[iComp].h != height )
				throw std::runtime_error( "jpeg2000 unsupported reader - required component has different size then main image " + filename );
			if ( opj_image->comps[iComp].bpp > bppMax )
				bppMax = opj_image->comps[iComp].bpp;
		}
		if ( bppMax > 8 )
			bpc = 16;

		// todo: bpc, alpha, dpi!
		PixelMode pm = PixelMode::create( isCMYK, isRGB, isGray, hasAlpha, bpc ); 

		image->create( width, height, pm,  72, 72 );
		
		// icc profile
		if ( opj_image->icc_profile_buf && opj_image->icc_profile_len )
		{
			// embedded icc --- this doesn't work!!! Something is going on with openJPEG in this aspect, I assume
			// it is already used to translate YCbCr to RGB. 
//			getIccProfile().assign(reinterpret_cast<char*>(opj_image->icc_profile_buf), opj_image->icc_profile_len);
// 			props->embeddedIccProfile = true;
		}
		else if ( opj_image->color_space == OPJ_CLRSPC_SRGB )
		{
			// explicitely sRGB
			props->colorSpaceIsSRGB = true;
//			ColorManager cms;
//			cmsHPROFILE profile = cms.createSrgbProfile();
// 			cms.embeddProfile(image, profile);
//			cms.closeProfile(profile);
		}

		for ( size_t y=0; y<height; y++ )
		{
			size_t yOffJp2 = width*y;
			if ( isGray )
			{
				if ( bpc == 16 )
				{
					Image::Pixel_GRAY_16* data = reinterpret_cast<Image::Pixel_GRAY_16*>(image->getLine(y));
					for ( size_t x=0; x<width; x++ )
					{
						data->value = opj_image->comps[0].data[yOffJp2+x];
						data++;
					}					
				}
				else
				{
					Image::Pixel_GRAY_8* data = reinterpret_cast<Image::Pixel_GRAY_8*>(image->getLine(y));
					for ( size_t x=0; x<width; x++ )
					{
						data->value = opj_image->comps[0].data[yOffJp2+x];
						data++;
					}										
				}
			}
			else if ( isRGB )
			{
				if ( bpc == 16 )
				{
					Image::Pixel_RGB_16* data = reinterpret_cast<Image::Pixel_RGB_16*>(image->getLine(y));
					for ( size_t x=0; x<width; x++ )
					{
						data->r = opj_image->comps[0].data[yOffJp2+x];
						data->g = opj_image->comps[1].data[yOffJp2+x];
						data->b = opj_image->comps[2].data[yOffJp2+x];
						data++;
					}
				}
				else
				{
					Image::Pixel_RGB_8* data = reinterpret_cast<Image::Pixel_RGB_8*>(image->getLine(y));
					for ( size_t x=0; x<width; x++ )
					{
						data->r = opj_image->comps[0].data[yOffJp2+x];
						data->g = opj_image->comps[1].data[yOffJp2+x];
						data->b = opj_image->comps[2].data[yOffJp2+x];
						data++;
					}
				}
			}
			else if ( isCMYK )
			{
				if ( bpc == 16 )
				{
					Image::Pixel_CMYK_16* data = reinterpret_cast<Image::Pixel_CMYK_16*>(image->getLine(y));
					for ( size_t x=0; x<width; x++ )
					{
						data->c = opj_image->comps[0].data[yOffJp2+x];
						data->m = opj_image->comps[1].data[yOffJp2+x];
						data->y = opj_image->comps[2].data[yOffJp2+x];
						data->k = opj_image->comps[2].data[yOffJp2+x];
						data++;
					}
				}
				else
				{
					Image::Pixel_CMYK_8* data = reinterpret_cast<Image::Pixel_CMYK_8*>(image->getLine(y));
					for ( size_t x=0; x<width; x++ )
					{
						data->c = opj_image->comps[0].data[yOffJp2+x];
						data->m = opj_image->comps[1].data[yOffJp2+x];
						data->y = opj_image->comps[2].data[yOffJp2+x];
						data->k = opj_image->comps[2].data[yOffJp2+x];
						data++;
					}
				}
			}
		}
					   

		/* free image data structure */
		opj_image_destroy(opj_image);

		/* destroy the codestream index */
//		opj_destroy_cstr_index(&cstr_index);

		if(failed) remove(parameters.outfile);
		
//		fclose(pFile);
		
		onSubImageRead(1);	// we have read image #1
		onSubImageRead(0);	// we are done!
		
	}
	catch (std::runtime_error)
	{
//		jpeg_destroy_decompress(&cinfo);
//		fclose(pFile);
		throw;
	}
}

void ImageCoderJpeg2000::write(const std::string & filename )
{
	throw std::runtime_error("JPEG write not implemented");
}


