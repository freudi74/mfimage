/* TimeStamper.h
 * Copyright 2015 Marco Freudenberger.
 * 
 * This file is part of mfimage library.
 * 
 * Design and implementation by:
 * --------------------------------------------------
 * - Marco Freudenberger (Marco.Freudenberger@gmx.de)
 *
 */
 
#pragma once
#include <chrono>
#include <vector>

class TimeStamper
{
public:
	TimeStamper(size_t numStamps = 2);
	virtual ~TimeStamper();

	void setStamp(size_t index);
	std::chrono::high_resolution_clock::duration getDuration(size_t index, bool fromPrevious=true);
	size_t getDurationMs(size_t index, bool fromPrevious = true);

	std::string toString(bool fromPrevious=true);

private:
	std::vector<std::chrono::high_resolution_clock::time_point> stamps;
};

