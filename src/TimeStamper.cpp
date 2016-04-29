/* TimeStamper.cpp
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
#include "TimeStamper.h"
#include <algorithm>

TimeStamper::TimeStamper(size_t numStamps /*= 2*/) : stamps(numStamps)
{
}


TimeStamper::~TimeStamper()
{
}

void TimeStamper::setStamp(size_t index)
{
	stamps[index] = std::chrono::high_resolution_clock::now();
}

std::chrono::high_resolution_clock::duration TimeStamper::getDuration(size_t index, bool fromPrevious/*=true*/)
{
	size_t indexStart = fromPrevious ? std::max(size_t(1),index)-1 : 0;
	return stamps[index] - stamps[indexStart];
}

size_t TimeStamper::getDurationMs(size_t index, bool fromPrevious /*= true*/)
{
	long long ms = std::chrono::duration_cast<std::chrono::milliseconds>( getDuration(index,fromPrevious) ).count();
	return static_cast<size_t>(ms);
}

std::string TimeStamper::toString(bool fromPrevious/*=true*/)
{
	std::stringstream ss;
	
	for (size_t i = 1; i < stamps.size(); i++)
	{
		ss << i << ":" << getDurationMs(i, fromPrevious) << " ms; ";
	}
	return ss.str();
}




