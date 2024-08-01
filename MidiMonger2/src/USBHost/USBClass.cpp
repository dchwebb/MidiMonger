#include "USBClass.h"
#include "USBHost.h"

USBClass::USBClass(USBHost* usbHost, const char* name, uint8_t classCode) : usbHost(usbHost), name(name), classCode(classCode) {

}



