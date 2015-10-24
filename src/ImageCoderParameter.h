/* ImageCoderParameter.h
 * Copyright 2015 Marco Freudenberger.
 * 
 * This file is part of mf-image library.
 * 
 * Design and implementation by:
 * --------------------------------------------------
 * - Marco Freudenberger (Marco.Freudenberger@gmx.de)
 *
 */

#pragma once
#include <memory>
#include <cstdint>
#include <tiff.h>
#include <map>
#include <vector>





/*
class ImageCoderParameter
{
public:
	enum class VariableType { BOOL=0, INT16, UINT16, INT32, UINT32, FLOAT, DOUBLE };
private:
	static constexpr size_t VariableTypeLen[] = { sizeof(bool), sizeof(int16_t), sizeof(uint16_t), sizeof(int32_t), sizeof(uint32_t), sizeof(float), sizeof(double) };
	static constexpr VariableType ParameterVariableTypes[] = { VariableType::UINT16, VariableType::UINT16, VariableType::BOOL };

private:		
	ImageCoderParameter( VariableType varType, ImageCoderParameterType parameterType, const void* const pValue=nullptr );
public:
	virtual ~ImageCoderParameter();
	
	static ImageCoderParameter* createParameter( ImageCoderParameterType type );
	
	ImageCoderParameterType getParameterType() const { return parameterType; }
	
	bool setValue( bool bVal ) { if ( varType != VariableType::BOOL ) return false; setValueInternal( &bVal ); return true; }
	bool setValue( int16_t i16Val ) { if ( varType != VariableType::INT16 ) return false; setValueInternal( &i16Val ); return true; }
	bool setValue( uint16_t u16Val ) { if ( varType != VariableType::UINT16 ) return false; setValueInternal( &u16Val ); return true; }
	bool setValue( int32_t i32Val ) { if ( varType != VariableType::INT32 ) return false; setValueInternal( &i32Val ); return true; }
	bool setValue( uint32_t u32Val ) { if ( varType != VariableType::UINT32 ) return false; setValueInternal( &u32Val ); return true; }
	bool setValue( float fVal ) { if ( varType != VariableType::FLOAT ) return false; setValueInternal( &fVal ); return true; }
	bool setValue( double dVal ) { if ( varType != VariableType::DOUBLE ) return false; setValueInternal( &dVal ); return true; }
	
	bool     getValueBool() const   { return *reinterpret_cast<bool*>(varValue.get()); }
	int16_t  getValueInt16() const  { return *reinterpret_cast<int16_t*>(varValue.get()); }
	uint16_t getValueUInt16() const { return *reinterpret_cast<uint16_t*>(varValue.get()); }
	int32_t  getValueInt32() const  { return *reinterpret_cast<int32_t*>(varValue.get()); }
	uint32_t getValueUInt32() const { return *reinterpret_cast<uint32_t*>(varValue.get()); }
	float    getValueFloat() const  { return *reinterpret_cast<float*>(varValue.get()); }
	double   getValueDouble() const { return *reinterpret_cast<double*>(varValue.get()); }
	
private:
	void setValueInternal( const void* const pVal );
	VariableType  varType;
	ImageCoderParameterType parameterType;
	std::unique_ptr<uint8_t[]> varValue;
};

*/