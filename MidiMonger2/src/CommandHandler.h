#pragma once

#include "initialisation.h"
#include <string>

class CommandHandler
{
public:
	void CheckCommands();
	void ProcessCommand(std::string_view cmd);
	int32_t ParseInt(const std::string_view cmd, const char precedingChar, const int32_t low = 0, const int32_t high = 0);
	float ParseFloat(const std::string_view cmd, const char precedingChar, const float low, const float high);

	static constexpr uint32_t bufSize = 1024;
	char buf[bufSize];
};


extern CommandHandler commandHandler;
