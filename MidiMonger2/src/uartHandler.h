#include "initialisation.h"
#include <string>
#include <sstream>
#include <iomanip>

extern volatile uint8_t uartCmdPos;
extern volatile char uartCmd[100];
extern volatile bool uartCmdRdy;

std::string IntToString(const int32_t& v);
std::string HexToString(const uint32_t& v, const bool& spaces);
std::string HexByte(const uint16_t& v);
size_t uartSendStr(const unsigned char* s, size_t len);
void uartSendStr(const std::string& s);
void InitUART();