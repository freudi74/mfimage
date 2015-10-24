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
#include "ImageCoderJpeg.h"
#include <jpeglib.h>

#define EXIF_MARKER		(JPEG_APP0+1)	// EXIF marker / Adobe XMP marker
#define ICC_MARKER		(JPEG_APP0+2)	// ICC profile marker content: "ICC_PROFILE\0" <seqNo> <numMarkers> where seqNo = 1 ... numMarkers
#define IPTC_MARKER		(JPEG_APP0+13)	// IPTC marker / BIM marker 


ImageCoderJpeg::ImageCoderJpeg(Image* img) : ImageCoder(IE_JPEG, img)
{
}


ImageCoderJpeg::~ImageCoderJpeg()
{
}

bool ImageCoderJpeg::canEncode()
{
	return false;
}
bool ImageCoderJpeg::canDecode()
{
	return true;
}
bool ImageCoderJpeg::canEncode( PixelMode pixelMode )
{
	return false;
}

static bool readIccProfile(j_decompress_ptr cinfo, std::string & iccData)
{
	std::vector<jpeg_marker_struct*> iccMarkers;
	for (jpeg_marker_struct* marker = cinfo->marker_list; marker != NULL; marker = marker->next) {
		if (marker->marker == ICC_MARKER && marker->data_length >= 14 && 0 == strcmp(reinterpret_cast<const char*>(marker->data), "ICC_PROFILE")) {
			size_t seqNo = GETJOCTET(marker->data[12]);
			size_t numMarkers = GETJOCTET(marker->data[13]);
			if (iccMarkers.empty())
				iccMarkers.resize(numMarkers, nullptr);
			else if ( iccMarkers.size() != numMarkers )
				throw std::runtime_error("ICC Profile: inconsistent numMarkers field");
			if (seqNo <= 0 || seqNo > numMarkers)
				throw std::runtime_error("ICC Profile: bogus sequenceNo");
			if (iccMarkers[seqNo - 1] != nullptr)
				throw std::runtime_error("ICC Profile: duplicate sequenceNo");
			iccMarkers[seqNo - 1] = marker;
		}
	}

	if (iccMarkers.empty())
		return false;

	// process all markers !
	iccData.clear();
	for (const auto m : iccMarkers)
	{
		if (m == nullptr)
			throw std::runtime_error("ICC Profile: missing sequenceNo");
		iccData.append(reinterpret_cast<const char*>(m->data + 14), m->data_length - 14);
	}
	if (iccData.empty())
		return false;		// found only empty markers ?
	return true;
}


/**
Read JPEG special markers
*/
/*static void read_JPEG_Markers(j_decompress_ptr cinfo, Image* img) {
jpeg_saved_marker_ptr marker;

for (marker = cinfo->marker_list; marker != NULL; marker = marker->next) {
switch (marker->marker) {
case JPEG_COM:
// JPEG comment
jpeg_read_comment(dib, marker->data, marker->data_length);
break;
case EXIF_MARKER:
// Exif or Adobe XMP profile
jpeg_read_exif_profile(dib, marker->data, marker->data_length);
jpeg_read_xmp_profile(dib, marker->data, marker->data_length);
break;
case IPTC_MARKER:
// IPTC/NAA or Adobe Photoshop profile
jpeg_read_iptc_profile(dib, marker->data, marker->data_length);
break;
}
}

// ICC profile
BYTE *icc_profile = NULL;
unsigned icc_length = 0;

if (jpeg_read_icc_profile(cinfo, &icc_profile, &icc_length)) {
// copy ICC profile data
FreeImage_CreateICCProfile(dib, icc_profile, icc_length);
// clean up
free(icc_profile);
}

return TRUE;
}
*/

void ImageCoderJpeg::read(const std::string & filename)
{
	FILE* pFile = fopen(filename.c_str(), "rb");
	///////////////////////////////////////////////
	// READ JPEG FILE using libjpeg-turbo
	///////////////////////////////////////////////
	auto errorRoutine = [](j_common_ptr cinfo) {
		char jpegLastErrorMsg[JMSG_LENGTH_MAX];
		(*(cinfo->err->format_message)) (cinfo, jpegLastErrorMsg);
		throw std::runtime_error(jpegLastErrorMsg);
	};

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	jerr.error_exit = errorRoutine;
	cinfo.err = jpeg_std_error(&jerr);

// 	int row_stride;

	try
	{
		jpeg_create_decompress(&cinfo);

		/* Step 2: specify data source (eg, a file) */
#if 1
		jpeg_stdio_src(&cinfo, pFile); //we are not using a C - File
#else
		// cover the file object (Jpeg generically uses stdio-FILE
		struct JpegStream {
			jpeg_source_mgr pub;
			std::istream* stream;
			JOCTET buffer[4096];
		};
		auto streamInitSource = [](j_decompress_ptr cinfo) {
			JpegStream* src = reinterpret_cast<JpegStream*> (cinfo->src);
			src->pub.next_input_byte = src->buffer;
			src->pub.bytes_in_buffer = 0;
			src->stream->seekg(0); // seek to 0 here
		};
		auto streamFillBuffer = [](j_decompress_ptr cinfo) {
			JpegStream* src = reinterpret_cast<JpegStream*> (cinfo->src);
			src->pub.next_input_byte = src->buffer;
			src->stream->read(reinterpret_cast<char*>(src->buffer), 4096);
			src->pub.bytes_in_buffer = static_cast<size_t>(src->stream->gcount());
			return static_cast<boolean>(src->stream->eof() ? 0 : 1);
		};
		auto streamSkip = [](j_decompress_ptr cinfo, long count) {
			JpegStream* src = reinterpret_cast<JpegStream*> (cinfo->src);
			if (src->pub.bytes_in_buffer > count)
			{
				src->pub.next_input_byte += count;
				src->pub.bytes_in_buffer -= count;
			}
			else
			{
				count -= src->pub.bytes_in_buffer;
				src->stream->seekg(count, std::ios_base::cur);
				src->pub.bytes_in_buffer = 0;
			}
		};
		auto streamTerm = [](j_decompress_ptr cinfo){};

		cinfo.src = (struct jpeg_source_mgr *) (*cinfo.mem->alloc_small) ((j_common_ptr)&cinfo, JPOOL_PERMANENT, sizeof(JpegStream));
		JpegStream * src = reinterpret_cast<JpegStream*> (cinfo.src);
		src->pub.init_source = streamInitSource;
		src->pub.fill_input_buffer = streamFillBuffer;
		src->pub.skip_input_data = streamSkip;
		src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
		src->pub.term_source = streamTerm;
		src->stream = &file;
		src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
		src->pub.next_input_byte = NULL; /* until buffer loaded */
#endif
		jpeg_save_markers(&cinfo, JPEG_COM, 0xffff);
		for (int m = 0; m < 16; m++) {
			jpeg_save_markers(&cinfo, JPEG_APP0 + m, 0xFFFF);
		}

		/* Step 3: read file parameters with jpeg_read_header() */
		jpeg_read_header(&cinfo, TRUE);
		/* We can ignore the return value from jpeg_read_header since
		*   (a) suspension is not possible with the stdio data source, and
		*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
		* See libjpeg.txt for more info.
		*/
		jpeg_start_decompress(&cinfo);

		PixelMode pm;
		switch (cinfo.out_color_space) {
		case JCS_GRAYSCALE:
			if (cinfo.output_components != 1)
				throw std::runtime_error("Unexpected number of color components for GRAYSCALE");
			pm = PixelMode::GRAY8;
			break;
		case JCS_RGB:
			if (cinfo.output_components != 3)
				throw std::runtime_error("Unexpected number of color components for RGB");
			pm = PixelMode::RGB8;
			break;
		case JCS_CMYK:
			if (cinfo.output_components != 4)
				throw std::runtime_error("Unexpected number of color components for CMYK");
			pm = PixelMode::CMYK8;
			break;
		default:
			throw std::runtime_error("Unexpected pixel format (color space): " + std::to_string(cinfo.out_color_space));
		}
		double dpiX, dpiY;
		switch (cinfo.density_unit)
		{
		case 0: // "aspect ratio"
			dpiX = 96.0;
			dpiY = 96.0;
			break;
		case 1: // dpi
			dpiX = cinfo.X_density;
			dpiY = cinfo.Y_density;
			break;
		case 2: // pixels per cm
			dpiX = cinfo.X_density * 2.54;
			dpiX = cinfo.Y_density * 2.54;
			break;
		}
		image->create(cinfo.output_width, cinfo.output_height, pm, dpiX, dpiY);


		/* We may need to do some setup of our own at this point before reading
		* the data.  After jpeg_start_decompress() we have the correct scaled
		* output image dimensions available, as well as the output colormap
		* if we asked for color quantization.
		* In this example, we need to make an output work buffer of the right size.
		*/
		/* JSAMPLEs per row in output buffer */
		// row_stride = cinfo.output_width * cinfo.output_components;
		/* Make a one-row-high sample array that will go away when done with image */
		//			buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

		/* Here we use the library's state variable cinfo.output_scanline as the
		* loop counter, so that we don't have to keep track ourselves.
		*/
		unsigned char *rowp[1];
		while (cinfo.output_scanline < cinfo.output_height)
		{
			/* jpeg_read_scanlines expects an array of pointers to scanlines.
			* Here the array is only one element long, but you could ask for
			* more than one scanline at a time if that's more convenient.
			*/
			uint8_t* line = image->getLine(cinfo.output_scanline);
			rowp[0] = reinterpret_cast<unsigned char *>(line);
			(void)jpeg_read_scanlines(&cinfo, rowp, 1);
			// cmyk is inverted ...
			if ( cinfo.out_color_space == JCS_CMYK ) {
				for ( size_t sample=0; sample<cinfo.output_width*4; sample++ )
				{
					line[sample] = ~line[sample];
				}
			}

			/* Assume put_scanline_someplace wants a pointer and sample count. */
		}

		readIccProfile(&cinfo, getIccProfile() );

		/* Step 7: Finish decompression */
		(void)jpeg_finish_decompress(&cinfo);
		/* We can ignore the return value since suspension is not possible
		* with the stdio data source.
		*/

		/* Step 8: Release JPEG decompression object */
		/* This is an important step since it will release a good deal of memory. */
		jpeg_destroy_decompress(&cinfo);

		/* After finish_decompress, we can close the input file.
		* Here we postpone it until after no more JPEG errors are possible,
		* so as to simplify the setjmp error logic above.  (Actually, I don't
		* think that jpeg_destroy can do an error exit, but why assume anything...)
		*/
		fclose(pFile);
		
		onSubImageRead(1);	// we have read image #1
		onSubImageRead(0);	// we are done!
		
	}
	catch (std::runtime_error)
	{
		jpeg_destroy_decompress(&cinfo);
		fclose(pFile);
		throw;
	}
}

void ImageCoderJpeg::write(const std::string & filename )
{
	throw std::runtime_error("JPEG write not implemented");
}

