/*
	FILE: 		DoubleResetDetector.h
	VERSION: 	1.0.0
	PURPOSE: 	Trigger configure mode by resetting ESP8266 twice.
	LICENCE:	MIT
 */

#ifndef DoubleResetDetector_H__
#define DoubleResetDetector_H__

#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define DOUBLERESETDETECTOR_VERSION "1.0.0"
#define DOUBLERESETDETECTOR_FLAG_SET 0xD0D01234
#define DOUBLERESETDETECTOR_FLAG_CLEAR 0xD0D04321

class DoubleResetDetector
{
public:
	DoubleResetDetector(int timeout, int address);
	bool detectDoubleReset();
	bool doubleResetDetected;
	void loop();
	void stop();
	
private:
	int timeout;
	int address;
	bool waitingForDoubleReset;
	bool detectRecentlyResetFlag();
	void clearRecentlyResetFlag();
	void setRecentlyResetFlag();
	uint32_t doubleResetDetectorFlag;
};
#endif // DoubleResetDetector_H__
