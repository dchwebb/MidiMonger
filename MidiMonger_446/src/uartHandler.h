#include "initialisation.h"
#include <string>
#include <sstream>
#include <iomanip>

std::string IntToString(const int32_t& v);
std::string HexToString(const uint32_t& v, const bool& spaces);
std::string HexByte(const uint16_t& v);
void uartSendStr(const std::string& s);
void InitUART();
