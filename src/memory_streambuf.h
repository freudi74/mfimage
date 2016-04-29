#pragma once
#include <streambuf>
#include <cstdint>

// TODO !

#if 0
class memory_streambuf : public std::streambuf
{
public:
	memory_streambuf(const void * const buffer, size_t size);
	memory_streambuf(const uint8_t * const buffer, size_t size);
	virtual ~memory_streambuf();

protected:
	virtual std::streampos seekoff( std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out );
	virtual std::streampos seekpos( std::streampos sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
	virtual int_type underflow();
	virtual int_type uflow();
	virtual int_type pbackfail(int_type ch);
	virtual std::streamsize showmanyc();
	// copy ctor and assignment not implemented;
	// copying not allowed
	byte_array_buffer(const byte_array_buffer &);
	byte_array_buffer &operator= (const byte_array_buffer &);		sync
		[virtual]
	synchronizes the buffers with the associated character sequence
		(virtual protected member function)
		Get area
		showmanyc
		[virtual]
	obtains the number of characters available for input in the associated input sequence, if known
		(virtual protected member function)
		underflow
		[virtual]
	reads characters from the associated input sequence to the get area
		(virtual protected member function)
		uflow
		[virtual]
	reads characters from the associated input sequence to the get area and advances the next pointer
		(virtual protected member function)
		xsgetn
		[virtual]
	reads multiple characters from the input sequence
		(virtual protected member function)
		ebackgptregptr
		returns a pointer to the beginning, current character and the end of the get area
		(protected member function)
		gbump
		advances the next pointer in the input sequence
		(protected member function)
		setg
		repositions the beginning, next, and end pointers of the input sequence
		(protected member function)
		Put area
		xsputn
		[virtual]
	writes multiple characters to the output sequence
		(virtual protected member function)
		overflow
		[virtual]
	writes characters to the associated output sequence from the put area
		(virtual protected member function)
		pbasepptrepptr
		returns a pointer to the beginning, current character and the end of the put area
		(protected member function)
		pbump
		advances the next pointer of the output sequence
		(protected member function)
		setp
		repositions the beginning, next, and end pointers of the output sequence
		(protected member function)
		Putback
		pbackfail
		[virtual]
	puts a character back into the input sequence, possibly modifying the input sequence
		(virtual protected member function)

	int_type underflow();
	int_type uflow();
	int_type pbackfail(int_type ch);
	std::streamsize showmanyc();
	std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way,
		std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
	std::streampos seekpos(std::streampos sp,
		std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

	// copy ctor and assignment not implemented;
	// copying not allowed
	byte_array_buffer(const byte_array_buffer &);
	byte_array_buffer &operator= (const byte_array_buffer &);

private:
	const uint8_t * const _begin;
	const uint8_t * const _end;
	const uint8_t * _current;
};

#endif