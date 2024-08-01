#include "USBHost.h"
#include "MidiHostClass.h"

USBHost usbHost;

void USBHost::Init()
{
	DeInitStateMachine();							// Restore default states and prepare EP0

	// Restore default Device connection states
	device.portEnabled = false;
	device.isConnected = false;
	device.isDisconnected = false;
	device.isReEnumerated = false;

	// PA11 = USB_OTG_FS_DM; PA12 = USB_OTG_FS_DP
	GpioPin::Init(GPIOA, 11, GpioPin::Type::AlternateFunction, 10, GpioPin::DriveStrength::VeryHigh);
	GpioPin::Init(GPIOA, 12, GpioPin::Type::AlternateFunction, 10, GpioPin::DriveStrength::VeryHigh);

	RCC->DCKCFGR2 |= RCC_DCKCFGR2_CK48MSEL;					// 0 = PLLQ Clock; 1 = PLLSAI_P
	RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;

	NVIC_SetPriority(OTG_FS_IRQn, 2);
	NVIC_EnableIRQ(OTG_FS_IRQn);

	USB_OTG_FS->GAHBCFG &= ~USB_OTG_GAHBCFG_GINT;			// Disable global interrupts
	USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_PHYSEL;			// Select FS Embedded PHY

	// Reset USB Core
	while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0) {};
	USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;
	while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_CSRST) == USB_OTG_GRSTCTL_CSRST);
	USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN;				// Activate Transceiver

	// Enter host mode
	USB_OTG_FS->GUSBCFG &= ~USB_OTG_GUSBCFG_FDMOD;
	USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FHMOD;
	while ((USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_CMOD_Msk) != HostMode) {};

	// Set Rx FIFO size
	USB_OTG_FS->GRXFSIZ = 0x80;
	USB_OTG_FS->DIEPTXF0_HNPTXFSIZ = (uint32_t)(((0x60U << 16) & USB_OTG_NPTXFD) | 0x80);
	USB_OTG_FS->HPTXFSIZ = (uint32_t)(((0x40U << 16)& USB_OTG_HPTXFSIZ_PTXFD) | 0xE0);

	// Enable host interrupts
	USB_OTG_FS->GINTSTS = 0xFFFFFFFF;	// Clear any pending interrupts
	USB_OTG_FS->GINTMSK |= (USB_OTG_GINTMSK_RXFLVLM | USB_OTG_GINTMSK_PRTIM | USB_OTG_GINTMSK_HCIM | USB_OTG_GINTMSK_SOFM | USB_OTG_GINTSTS_DISCINT |
		USB_OTG_GINTMSK_PXFRM_IISOOXFRM | USB_OTG_GINTMSK_WUIM);

	timer = USB_HOST->HFNUM & USB_OTG_HFNUM_FRNUM;			// Host frame number

	classes[classNumber++] = &midiHostClass;				// link the class to the USB Host handle

	Start();
}


void USBHost::Start()
{
	USB_HPRT0 |= USB_OTG_HPRT_PPWR;							// Enable port power - not sure why as this should be done via external power supply
	USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT;			// Enable global interrupt
	PowerEnable.SetHigh();
}


void USBHost::DeInitStateMachine()
{
	// Clear Pipes flags
	for (uint32_t i = 0; i < maxNumPipes; ++i) {
		pipes[i] = 0;
	}

	for (uint32_t i = 0; i < maxDataBuffer; ++i) {
		device.data[i] = 0;
	}

	gState = HostState::Idle;
	enumState = EnumState::Idle;
	requestState = StateType::Send;
	timer = 0;

	control.state = Control::StateType::Setup;
	control.pipeSize = maxPacketSizeDefault;
	control.errorCount = 0;

	device.address = 0;
	device.speed = SpeedFull;
	device.resetCount = 0;
	device.enumCount = 0;
}


void USBHost::Process()
{
	HostStatus status = HostStatus::Fail;

	if (device.isDisconnected) {							// check for Host pending port disconnect event
		gState = HostState::DevDisconnected;
	}

	switch (gState) {
	case HostState::Idle:
		if (device.isConnected) {
			USBH_UsrLog("USB Device Connected");
			gState = HostState::DevWaitForAttachment;
			DelayMS(200);
			ResetPort();
			device.address = 0;
			timeout = 0;
		}
		break;

	case HostState::DevWaitForAttachment:					 // Wait for Port Enabled
		if (device.portEnabled) {
			USBH_UsrLog("USB Device Reset Completed");
			device.resetCount = 0;
			gState = HostState::DevAttached;
		} else if (timeout > devResetTimeout) {
			if (++device.resetCount > 3) {					// Buggy Device can't complete reset
				USBH_UsrLog("USB Reset Failed: Unplug the Device");
				gState = HostState::Abort;
			} else {
				gState = HostState::Idle;
			}
		} else {
			timeout += 10;
			DelayMS(10);
		}
		break;

	case HostState::DevAttached:
		DelayMS(100);		// Wait for 100 ms after Reset
		device.speed = GetHostSpeed();
		USBH_UsrLog("Device Attached. Speed: %d", device.speed);
		gState = HostState::Enumeration;
		control.pipeOut = AllocPipe(0x00);
		control.pipeIn  = AllocPipe(0x80);

		// Open Control pipes
		HostChannelInit(control.pipeIn, 0x80, device.address, device.speed, ControlEP, control.pipeSize);
		HostChannelInit(control.pipeOut, 0x00, device.address, device.speed, ControlEP, control.pipeSize);

		break;

	case HostState::Enumeration:
		status = DoEnumeration();
		if (status == HostStatus::OK) {
			device.currentInterface = 0;
			USBH_UsrLog("Enumeration done. Device has %d configuration(s)", device.devDesc.bNumConfigurations);
			gState = HostState::SetConfiguration;
		}
		break;

	case HostState::SetConfiguration:
		if (ControlCommand(RequestSetConfiguration, (uint16_t)device.cfgDesc.bConfigurationValue, 0) == HostStatus::OK) {
			gState = HostState::SetWakeupFeature;
			USBH_UsrLog("Default configuration set");
		}
		break;

	case HostState::SetWakeupFeature:
		if (device.cfgDesc.bmAttributes & (1 << 5)) {
			status = ControlCommand(RequestSetFeature, FeatureSelectorRemoteWakeup, 0);

			if (status == HostStatus::OK) {
				USBH_UsrLog("Device remote wakeup enabled");
			} else if (status == HostStatus::NotSupported) {
				USBH_UsrLog("Remote wakeup not supported by the device");
			}
		}
		gState = HostState::CheckClass;
		break;

	case HostState::CheckClass:

		if (classNumber == 0) {
			USBH_UsrLog("No Class has been registered");
		} else {
			activeClass = nullptr;
			for (uint32_t i = 0; i < maxNumSupportedClass; i++) {
				if (classes[i]->classCode == device.cfgDesc.ifDesc[0].bInterfaceClass) {
					activeClass = classes[i];
					break;
				}
			}

			if (activeClass != nullptr) {
				if (activeClass->InterfaceInit() == HostStatus::OK) {
					gState = HostState::Class;
					USBH_UsrLog("%s class started", activeClass->name);
				} else {
					gState = HostState::Abort;
					USBH_UsrLog("Device not supporting %s class", activeClass->name);
				}
			} else {
				gState = HostState::Abort;
				USBH_UsrLog("No registered class for this device");
			}
		}
		break;

	case HostState::Class:
		if (activeClass != nullptr) {					// process class state machine
			activeClass->Process();
		}
		break;

	case HostState::DevDisconnected:
		device.isDisconnected = false;
		DeInitStateMachine();
		if (activeClass != nullptr) {					// Re-Initialize Host for new enumeration
			activeClass->InterfaceDeInit();
			activeClass = nullptr;
		}
		if (device.isReEnumerated) {
			device.isReEnumerated = false;
		}
		USBH_UsrLog("USB Device disconnected");

		Start();										// Start the host and re-enable Vbus
		break;

	case HostState::Abort:
	default:
		break;
	}

}


HostStatus USBHost::DoEnumeration()
{
	// Manage the enumeration process
	HostStatus status = HostStatus::Busy;
	HostStatus reqStatus = HostStatus::Busy;

	switch (enumState) {
	case EnumState::Idle:
		reqStatus = GetDevDesc(8);		// Get Device Desc for 1st 8 bytes to get EP0 MaxPacketSize
		if (reqStatus == HostStatus::OK) {
			control.pipeSize = device.devDesc.bMaxPacketSize;
			USBH_UsrLog("Enumeration: Got max packet size: %d", control.pipeSize);
			enumState = EnumState::GetFullDevDesc;

			// Modify control channels configuration for MaxPacket size
			HostChannelInit(control.pipeIn, 0x80U, device.address, device.speed, ControlEP, control.pipeSize);
			HostChannelInit(control.pipeOut, 0x00U, device.address, device.speed, ControlEP, control.pipeSize);
		} else if (reqStatus == HostStatus::NotSupported) {
			EnumerationError();
		}
		break;

	case EnumState::GetFullDevDesc:
		reqStatus = GetDevDesc(deviceDescriptorSize);
		if (reqStatus == HostStatus::OK) {
			USBH_UsrLog("Enumeration: Get full device descriptor");
			USBH_UsrLog(" - PID: 0x%x", device.devDesc.idProduct);
			USBH_UsrLog(" - VID: 0x%x", device.devDesc.idVendor);
			enumState = EnumState::SetAddr;
		} else if (reqStatus == HostStatus::NotSupported) {
			EnumerationError();
		}
		break;

	case EnumState::SetAddr:
		reqStatus = ControlCommand(RequestSetAddress, deviceAddress, 0);
		if (reqStatus == HostStatus::OK) {
			DelayMS(2);
			device.address = deviceAddress;

			USBH_UsrLog("Enumeration: Address %d assigned", device.address);
			enumState = EnumState::GetCfgDesc;

			// modify control channels to update device address
			HostChannelInit(control.pipeIn, 0x80U,  device.address, device.speed, ControlEP, control.pipeSize);
			HostChannelInit(control.pipeOut, 0x00U, device.address, device.speed, ControlEP, control.pipeSize);

		} else if (reqStatus == HostStatus::NotSupported) {
			USBH_ErrLog("Control error: Device Set Address request failed");
			gState = HostState::Abort;
			enumState = EnumState::Idle;
		}
		break;

	case EnumState::GetCfgDesc:
		reqStatus = GetConfigDesc(configurationDescriptorSize);
		if (reqStatus == HostStatus::OK) {
			USBH_UsrLog("Enumeration: Get configuration descriptor size");
			enumState = EnumState::GetFullCfgDesc;
		} else if (reqStatus == HostStatus::NotSupported) {
			EnumerationError();
		}
		break;

	case EnumState::GetFullCfgDesc:
		reqStatus = GetConfigDesc(device.cfgDesc.wTotalLength);
		if (reqStatus == HostStatus::OK) {
			USBH_UsrLog("Enumeration: Got full configuration descriptor");
			enumState = EnumState::GetMfcStringDesc;
		} else if (reqStatus == HostStatus::NotSupported) {
			EnumerationError();
		}
		break;

	case EnumState::GetMfcStringDesc:
		if (device.devDesc.iManufacturer != 0) {
			reqStatus = GetStringDesc(device.devDesc.iManufacturer, device.data, 0xFF);
			if (reqStatus == HostStatus::OK) {
				USBH_UsrLog("Enumeration: Manufacturer %s", (char*)device.data);
				enumState = EnumState::GetProductStringDesc;

			} else if (reqStatus == HostStatus::NotSupported) {
				USBH_UsrLog("Enumeration: Manufacturer N/A");
				enumState = EnumState::GetProductStringDesc;
			}
		} else {
			USBH_UsrLog("Enumeration: Manufacturer N/A");
			enumState = EnumState::GetProductStringDesc;
		}
		break;

	case EnumState::GetProductStringDesc:
		if (device.devDesc.iProduct != 0) {
			reqStatus = GetStringDesc(device.devDesc.iProduct, device.data, 0xFF);
			if (reqStatus == HostStatus::OK) {
				USBH_UsrLog("Enumeration: Product %s", (char*)device.data);
				enumState = EnumState::GetSerialStringDesc;

			} else if (reqStatus == HostStatus::NotSupported) {
				USBH_UsrLog("Enumeration: Product N/A");
				enumState = EnumState::GetSerialStringDesc;
			}
		} else {
			USBH_UsrLog("Enumeration: Product N/A");
			enumState = EnumState::GetSerialStringDesc;
		}
		break;

	case EnumState::GetSerialStringDesc:
		if (device.devDesc.iSerialNumber != 0) {
			reqStatus = GetStringDesc(device.devDesc.iSerialNumber, device.data, 0xFF);
			if (reqStatus == HostStatus::OK) {
				USBH_UsrLog("Enumeration: Serial Number %s", (char*)device.data);
				status = HostStatus::OK;
			} else if (reqStatus == HostStatus::NotSupported) {
				USBH_UsrLog("Enumeration: Serial Number N/A");
				status = HostStatus::OK;
			}
		} else {
			USBH_UsrLog("Enumeration: Serial Number N/A");
			status = HostStatus::OK;
		}
		break;

	default:
		break;
	}
	return status;
}


void USBHost::EnumerationError()
{
	USBH_ErrLog("Control error: Get Device configuration descriptor request failed");
	if (++device.enumCount > 3) {
		USBH_UsrLog("Control error, Device not Responding Please unplug the Device");
		gState = HostState::Abort;
	} else {
		FreePipe(control.pipeOut);
		FreePipe(control.pipeIn);

		// Reset the USB Device
		enumState = EnumState::Idle;
		gState = HostState::Idle;
	}
}


void USBHost::HostChannelInit(const uint8_t channel, const uint8_t epNum, const uint8_t devAddress, const uint8_t speed, const uint8_t epType, const uint16_t mps)
{
	hc[channel].doPing = 0;
	hc[channel].maxPacket = mps;
	hc[channel].epType = epType;
	hc[channel].epDir = ((epNum & 0x80) == 0x80);

	// Clear old interrupt conditions for this host channel
	auto USB_HC = USBx_HC(channel);
	USB_HC->HCINT = 0xFFFFFFFF;

	// Enable channel interrupts required for this transfer
	switch (epType) {
	case ControlEP:
	case Bulk:
		USB_HC->HCINTMSK = USB_OTG_HCINTMSK_XFRCM |	USB_OTG_HCINTMSK_STALLM | USB_OTG_HCINTMSK_TXERRM | USB_OTG_HCINTMSK_DTERRM |
			USB_OTG_HCINTMSK_AHBERR | USB_OTG_HCINTMSK_NAKM;

		if (epNum & 0x80) {
			USB_HC->HCINTMSK |= USB_OTG_HCINTMSK_BBERRM;
		} else if (USB_OTG_FS->CID & (1 << 8)) {
			USB_HC->HCINTMSK |= USB_OTG_HCINTMSK_NYET | USB_OTG_HCINTMSK_ACKM;
		}
		break;

	case Interrupt:
		USB_HC->HCINTMSK = USB_OTG_HCINTMSK_XFRCM | USB_OTG_HCINTMSK_STALLM | USB_OTG_HCINTMSK_TXERRM | USB_OTG_HCINTMSK_DTERRM |
			USB_OTG_HCINTMSK_NAKM | USB_OTG_HCINTMSK_AHBERR | USB_OTG_HCINTMSK_FRMORM;

		if (epNum & 0x80) {
			USB_HC->HCINTMSK |= USB_OTG_HCINTMSK_BBERRM;
		}

		break;

	case Isochronous:
		USB_HC->HCINTMSK = USB_OTG_HCINTMSK_XFRCM | USB_OTG_HCINTMSK_ACKM | USB_OTG_HCINTMSK_AHBERR | USB_OTG_HCINTMSK_FRMORM;

		if (epNum & 0x80) {
			USB_HC->HCINTMSK |= USB_OTG_HCINTMSK_TXERRM | USB_OTG_HCINTMSK_BBERRM;
		}
		break;

	default:
		break;
	}

	USB_HC->HCINTMSK |= USB_OTG_HCINTMSK_CHHM;				// Enable host channel Halt interrupt
	USB_HOST->HAINTMSK |= 1 << (channel & 0xF);				// Enable the top level host channel interrupt.
	USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_HCIM;			// Make sure host channel interrupts are enabled.

	const uint32_t charEpDir = (epNum & 0x80) ? USB_OTG_HCCHAR_EPDIR : 0;

	uint32_t charLowSpeed = 0;
	if (speed == SpeedLow && GetHostSpeed() != SpeedLow) {
		charLowSpeed = USB_OTG_HCCHAR_LSDEV;
	}

	// Host channel characteristics
	USB_HC->HCCHAR = ((devAddress << 22) & USB_OTG_HCCHAR_DAD) |
			(((epNum & 0x7F) << 11) & USB_OTG_HCCHAR_EPNUM) |
			((epType << 18) & USB_OTG_HCCHAR_EPTYP) |
			(mps & USB_OTG_HCCHAR_MPSIZ) |
			charEpDir |
			charLowSpeed;

	if (epType == Interrupt || epType == Isochronous) {
		USB_HC->HCCHAR |= USB_OTG_HCCHAR_ODDFRM;
	}
}


HostStatus USBHost::GetDevDesc(const uint16_t length)
{
	// Issue Get Device Descriptor command to the device. Once the response received, parse the device descriptor and update the status
	HostStatus status = GetDescriptor(requestRecipientDevice | requestTypeStandard, descriptorDevice, device.data, length);

	if (status == HostStatus::OK) {		// Commands successfully sent and Response Received
		ParseDevDesc(device.devDesc, device.data, length);
	}

	return status;
}


HostStatus USBHost::GetDescriptor(const uint8_t reqType, const uint16_t valueIdx, uint8_t* buff, const uint16_t length)
{
	if (requestState == StateType::Send) {
		control.setup.bmRequestType = DeviceToHost | reqType;
		control.setup.bRequest      = RequestGetDescriptor;
		control.setup.wValue        = valueIdx;
		control.setup.wIndex        = ((valueIdx & 0xFF00) == descriptorString) ? 0x0409 : 0;
		control.setup.wLength       = length;
	}

	return CtlReq(buff, length);
}


HostStatus USBHost::GetStringDesc(const uint8_t stringIndex, uint8_t* buff, const uint16_t length)
{
	HostStatus status = GetDescriptor(requestRecipientDevice | requestTypeStandard, descriptorString | stringIndex, device.data, length);

	if (status == HostStatus::OK) {
		ParseStringDesc(device.data, buff, length);
	}
	return status;
}


HostStatus USBHost::GetConfigDesc(const uint16_t length)
{
	uint8_t* buf = device.cfgDescRaw;
	HostStatus status = GetDescriptor((requestRecipientDevice | requestTypeStandard), descriptorConfiguration, buf, length);

	if (status == HostStatus::OK) {
		DescHeader* pdesc = (DescHeader*)buf;

		// Parse configuration descriptor
		device.cfgDesc.bLength             = *(buf + 0);
		device.cfgDesc.bDescriptorType     = *(buf + 1);
		device.cfgDesc.wTotalLength        = std::min(((uint16_t) LE16(buf + 2)), (uint16_t)maxSizeConfiguration);
		device.cfgDesc.bNumInterfaces      = *(buf + 4);
		device.cfgDesc.bConfigurationValue = *(buf + 5);
		device.cfgDesc.iConfiguration      = *(buf + 6);
		device.cfgDesc.bmAttributes        = *(buf + 7);
		device.cfgDesc.bMaxPower           = *(buf + 8);

		// Make sure that the Confguration descriptor's bLength is equal to USB_CONFIGURATION_DESC_SIZE
		if (device.cfgDesc.bLength != configurationDescriptorSize) {
			device.cfgDesc.bLength = configurationDescriptorSize;
		}

		if (length > configurationDescriptorSize) {
			uint16_t ptr = configurationDescriptorSize;			// Position of the pointer into the descriptor - used to jump from section to section

			// DW removed code altering the length of descriptor items (pdesc->bLength) as this was breaking USBH_GetNextDesc

			uint32_t interface = 0;
			while (interface < maxNumInterfaces && ptr < device.cfgDesc.wTotalLength) {
				pdesc = GetNextDesc((uint8_t*)pdesc, &ptr);
				if (pdesc->bDescriptorType == descriptorTypeInterface) {

					InterfaceDescriptor& pif = device.cfgDesc.ifDesc[interface];
					uint8_t* ifBuf = (uint8_t*)pdesc;

					pif.bLength            = ifBuf[0];
					pif.bDescriptorType    = ifBuf[1];
					pif.bInterfaceNumber   = ifBuf[2];
					pif.bAlternateSetting  = ifBuf[3];
					pif.bNumEndpoints      = ifBuf[4];
					pif.bInterfaceClass    = ifBuf[5];
					pif.bInterfaceSubClass = ifBuf[6];
					pif.bInterfaceProtocol = ifBuf[7];
					pif.iInterface         = ifBuf[8];

					uint8_t epIdx = 0;

					while (epIdx < pif.bNumEndpoints && ptr < device.cfgDesc.wTotalLength) {
						pdesc = GetNextDesc((uint8_t*)pdesc, &ptr);

						if (pdesc->bDescriptorType == descriptorTypeEndpoint) {
							status = ParseEPDesc(pif.epDesc[epIdx], (uint8_t*)pdesc);
							epIdx++;
						}
					}

					if (epIdx < pif.bNumEndpoints) {					// Check if the required endpoint(s) data are parsed
						status = HostStatus::NotSupported;
					}
					interface++;
				}
			}

			// Check if the required interface(s) data are parsed
			if (interface < std::min(device.cfgDesc.bNumInterfaces, (uint8_t)maxNumInterfaces)) {
				status = HostStatus::NotSupported;
			}
		}
	}
	return status;
}


void USBHost::ParseDevDesc(DeviceDescriptor& devDesc, uint8_t* buf, const uint16_t length)
{
	devDesc.bLength            = *(buf + 0);
	devDesc.bDescriptorType    = *(buf + 1);
	devDesc.bcdUSB             = LE16(buf + 2);
	devDesc.bDeviceClass       = *(buf + 4);
	devDesc.bDeviceSubClass    = *(buf + 5);
	devDesc.bDeviceProtocol    = *(buf + 6);
	devDesc.bMaxPacketSize     = *(buf + 7);

	// Make sure that the max packet size is either 8, 16, 32, 64 or force it to 64
	if (devDesc.bMaxPacketSize != 8 && devDesc.bMaxPacketSize != 16 && devDesc.bMaxPacketSize != 32 && devDesc.bMaxPacketSize != 64) {
		devDesc.bMaxPacketSize = 64;
	}

	if (length > 8) {
		// For 1st time after device connection, Host may issue only 8 bytes for Device Descriptor Length
		devDesc.idVendor           = LE16(buf +  8);
		devDesc.idProduct          = LE16(buf + 10);
		devDesc.bcdDevice          = LE16(buf + 12);
		devDesc.iManufacturer      = *(buf + 14);
		devDesc.iProduct           = *(buf + 15);
		devDesc.iSerialNumber      = *(buf + 16);
		devDesc.bNumConfigurations = *(buf + 17);
	}
}


void USBHost::ParseStringDesc(uint8_t* psrc, uint8_t* pdest, const uint16_t length)
{
	if (psrc[1] == descriptorTypeString) {					// Make sure the Descriptor is String Type
		uint16_t strlength = std::min((uint16_t)(psrc[0] - 2), length);		// psrc[0] contains Size of Descriptor, subtract 2 to get the length of string
		psrc += 2;											// Adjust the offset ignoring the String Len and Descriptor type

		for (uint16_t idx = 0; idx < strlength; idx += 2) {
			*pdest++ =  psrc[idx];
		}
		*pdest = 0;											// Terminate string
	}
}


HostStatus USBHost::ParseEPDesc(EndpointDescriptor& epDesc, uint8_t* buf)
{
	HostStatus status = HostStatus::OK;
	epDesc.bLength          = *(buf + 0);
	epDesc.bDescriptorType  = *(buf + 1);
	epDesc.bEndpointAddress = *(buf + 2);
	epDesc.bmAttributes     = *(buf + 3);
	epDesc.wMaxPacketSize   = LE16(buf + 4);
	epDesc.bInterval        = *(buf + 6);

	// Make sure that wMaxPacketSize is not 0
	if (epDesc.wMaxPacketSize == 0) {
		status = HostStatus::NotSupported;

	} else if (maxEpPacketSize < (uint16_t)maxDataBuffer) {
		// Make sure that maximum packet size (bits 0-10) does not exceed the max endpoint packet size
		epDesc.wMaxPacketSize &= ~0x7FF;
		epDesc.wMaxPacketSize |= std::min((uint16_t)(LE16(buf + 4) & 0x7FF), (uint16_t)maxEpPacketSize);

	} else if (maxDataBuffer < maxEpPacketSize) {
		// Make sure that maximum packet size (bits 0-10) does not exceed the total buffer length
		epDesc.wMaxPacketSize &= ~0x7FF;
		epDesc.wMaxPacketSize |= std::min((uint16_t)(LE16(buf + 4) & 0x7FF), (uint16_t)maxDataBuffer);
	}

	if ((epDesc.bmAttributes & epTypeMask) == Isochronous && (epDesc.bInterval == 0 || epDesc.bInterval > 0x10)) {
		status = HostStatus::NotSupported;

	} else if ((epDesc.bmAttributes & epTypeMask) == Interrupt && epDesc.bInterval == 0) {
		status = HostStatus::NotSupported;
	}

	return status;
}


HostStatus USBHost::ControlCommand(const uint8_t request, const uint16_t wValue, const uint16_t index)
{
	if (requestState == StateType::Send) {
		control.setup.bmRequestType = HostToDevice | requestRecipientDevice | requestTypeStandard;
		control.setup.bRequest      = request;
		control.setup.wValue        = wValue;
		control.setup.wIndex        = index;
		control.setup.wLength       = 0;
	}

	return CtlReq(nullptr, 0);
}


HostStatus USBHost::ClearFeature(const uint8_t ep)
{
	// Clear or disable a specific feature.
	return ControlCommand(RequestClearFeature, FeatureSelectorEndpoint, ep);
}


USBHost::DescHeader* USBHost::GetNextDesc(uint8_t* pbuf, uint16_t* ptr)
{
	*ptr += ((DescHeader*)pbuf)->bLength;
	return (DescHeader*)(pbuf + ((DescHeader*)pbuf)->bLength);
}


HostStatus USBHost::CtlReq(uint8_t* buff, const uint16_t length)
{
	HostStatus status = HostStatus::Busy;

	switch (requestState) {
	case StateType::Send:		// Start a SETUP transfer
		control.buff = buff;
		control.length = length;
		control.state = Control::StateType::Setup;
		requestState = StateType::Wait;
		status = HostStatus::Busy;
		break;

	case StateType::Wait:
		status = HandleControl();
		if (status == HostStatus::OK || status == HostStatus::NotSupported) {			// Transaction completed, move control state to idle
			requestState  = StateType::Send;
			control.state = Control::StateType::Idle;

		} else if (status == HostStatus::Fail) {
			requestState = StateType::Send;
		}
		break;

	default:
		break;
	}
	return status;
}



HostStatus USBHost::HandleControl()
{
	// Handles the USB control transfer state machine
	HostStatus status = HostStatus::Busy;
	URBState URB_Status = URBState::Idle;

	switch (control.state) {
	case Control::StateType::Setup:
		StartTransfer(control.pipeOut, pidToken::Setup, (uint8_t*)&control.setup, setupPacketSize);
		control.state = Control::StateType::SetupWait;
		break;

	case Control::StateType::SetupWait:
		URB_Status = hc[control.pipeOut].urbState;
		if (URB_Status == URBState::Done) {
			uint8_t direction = (control.setup.bmRequestType & requestDirectionMask);

			if (control.setup.wLength != 0) {				// There is a data stage
				control.state = direction == DeviceToHost ? Control::StateType::DataIn : Control::StateType::DataOut;
			} else {											// No Data Transfer Stage
				control.state = direction == DeviceToHost ? Control::StateType::StatusOut : Control::StateType::StatusIn;
			}

		} else if (URB_Status == URBState::Error || URB_Status == URBState::NotReady) {
			control.state = Control::StateType::Error;
		}
		break;

	case Control::StateType::DataIn:
		StartTransfer(control.pipeIn, pidToken::Data, control.buff, control.length);
		control.state = Control::StateType::DataInWait;
		break;

	case Control::StateType::DataInWait:
		URB_Status = hc[control.pipeIn].urbState;

		if (URB_Status == URBState::Done) {
			control.state = Control::StateType::StatusOut;

		} else if (URB_Status == URBState::Stall) {				// In stall case, return to previous machine state
			status = HostStatus::NotSupported;

		} else if (URB_Status == URBState::Error) {				// Device error
			control.state = Control::StateType::Error;
		}
		break;

	case Control::StateType::DataOut:
		StartTransfer(control.pipeOut, pidToken::Data, control.buff, control.length);
		control.state = Control::StateType::DataOutWait;
		break;

	case Control::StateType::DataOutWait:

		URB_Status = hc[control.pipeOut].urbState;

		if (URB_Status == URBState::Done) {
			control.state = Control::StateType::StatusIn;

		} else if (URB_Status == URBState::Stall) {				// In stall case, return to previous machine state
			control.state = Control::StateType::Stalled;
			status = HostStatus::NotSupported;

		} else if (URB_Status == URBState::NotReady) {
			control.state = Control::StateType::DataOut;						// Nack received from device

		} else if (URB_Status == URBState::Error) {
			control.state = Control::StateType::Error;
			status = HostStatus::Fail;
		}
		break;

	case Control::StateType::StatusIn:
		StartTransfer(control.pipeIn, pidToken::Data, nullptr, 0);	// Send 0 byte packet
		control.state = Control::StateType::StatusInWait;
		break;

	case Control::StateType::StatusInWait:
		URB_Status = hc[control.pipeIn].urbState;

		if (URB_Status == URBState::Done) {
			control.state = Control::StateType::Complete;						// Control transfers completed, Exit the State Machine
			status = HostStatus::OK;

		} else if (URB_Status == URBState::Error) {
			control.state = Control::StateType::Error;

		} else  if (URB_Status == URBState::Stall) {
			status = HostStatus::NotSupported;					// Control transfers completed, Exit the State Machine
		}
		break;

	case Control::StateType::StatusOut:
		StartTransfer(control.pipeOut, pidToken::Data, nullptr, 0);
		control.state = Control::StateType::StatusOutWait;
		break;

	case Control::StateType::StatusOutWait:
		URB_Status = hc[control.pipeOut].urbState;
		if (URB_Status == URBState::Done) {
			status = HostStatus::OK;
			control.state = Control::StateType::Complete;

		} else if (URB_Status == URBState::NotReady) {
			control.state = Control::StateType::StatusOut;

		} else if (URB_Status == URBState::Error) {
			control.state = Control::StateType::Error;
		}
		break;

	case Control::StateType::Error:
		// After halt or error condition is encountered  a control endpoint is allowed to recover by accepting the next Setup PID;
		// For default Control Pipe, a device reset will be required to clear the halt or error if next Setup PID is not accepted.

		if (++control.errorCount <= maxErrorCount) {
			control.state = Control::StateType::Setup;			// Do the transmission again, starting from SETUP Packet
			requestState = StateType::Send;
		} else {

			control.errorCount = 0;
			USBH_ErrLog("Control error: Device not responding");

			FreePipe(control.pipeOut);
			FreePipe(control.pipeIn);

			gState = HostState::Idle;
			status = HostStatus::Fail;
		}
		break;

	default:
		break;
	}

	return status;
}


uint8_t USBHost::AllocPipe(const uint8_t epAddr)
{
	for (uint8_t pipe = 0; pipe < maxNumPipes; pipe++) {
		if ((pipes[pipe] & 0x8000) == 0) {
			pipes[pipe & 0xF] = (0x8000 | epAddr);
			return pipe;
		}
	}
	return 0xFF;
}


void USBHost::FreePipe(uint8_t idx)
{
	if (idx < maxNumPipes) {
		pipes[idx] &= 0x7FFF;
	}
}


USBHost::InterfaceDescriptor* USBHost::SelectInterface(const uint8_t ifClass, const uint8_t subClass)
{
	//device.cfgDesc.bNumInterfaces
	for (uint32_t i = 0; i < maxNumInterfaces; ++i) {
		if (device.cfgDesc.ifDesc[i].bInterfaceClass == ifClass && device.cfgDesc.ifDesc[i].bInterfaceSubClass == subClass) {
			device.currentInterface = i;
			USBH_UsrLog("Switching to Interface %lu", i);
			USBH_UsrLog("Class    : %xh", device.cfgDesc.ifDesc[i].bInterfaceClass);
			USBH_UsrLog("SubClass : %xh", device.cfgDesc.ifDesc[i].bInterfaceSubClass);
			return &device.cfgDesc.ifDesc[i];
		}
	}
	USBH_ErrLog("Cannot Select This Interface");
	return nullptr;
}


void USBHost::SetToggle(const uint8_t pipe, const uint8_t toggle)
{
	if (hc[pipe].epDir == DirIn) {
		hc[pipe].toggleIn = toggle;
	} else {
		hc[pipe].toggleOut = toggle;
	}
}


USBHost::URBState USBHost::GetURBState(const uint8_t channel)
{
	return hc[channel].urbState;
}


uint32_t USBHost::GetTransferCount(uint8_t channel)
{
	return hc[channel].xferCount;
}


uint32_t USBHost::GetHostSpeed()
{
	return ((USB_HPRT0 & USB_OTG_HPRT_PSPD) >> USB_OTG_HPRT_PSPD_Pos);		// 0 High speed; 1 Full speed; 2 Low speed
}


uint32_t USBHost::GetMode()
{
	return (USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_CMOD_Msk);
}


bool USBHost::TestInterrupt(uint32_t interrupt)
{
	// Test for specific interrupt type
	return ((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) & interrupt);
}


bool USBHost::TestClearInterrupt(uint32_t interrupt)
{
	// Test for specific interrupt type and clear if fired
	if ((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) & interrupt) {
		USB_OTG_FS->GINTSTS = interrupt;
		return true;
	}
	return false;
}


void USBHost::IRQHandler()
{
	if (GetMode() == HostMode) {
		if ((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) == 0) {	// Avoid spurious interrupt
			return;
		}

		// Clear interrupts
		USB_OTG_FS->GINTSTS = USB_OTG_GINTSTS_PXFR_INCOMPISOOUT | USB_OTG_GINTSTS_IISOIXFR | USB_OTG_GINTSTS_PTXFE | USB_OTG_GINTSTS_MMIS;

		if (TestClearInterrupt(USB_OTG_GINTSTS_DISCINT) && (USB_HPRT0 & USB_OTG_HPRT_PCSTS) == 0) {
			FlushTxFifo(0x10);									// Flush USB Fifo
			FlushRxFifo();
			SetPhyClockSpeed(UsbSpeed::HCFG_48_MHz);			// Restore FS Clock
			DisconnectHandler();								// Handle Host Port Disconnect Interrupt
		}

		if (TestInterrupt(USB_OTG_GINTSTS_HPRTINT)) {			// Handle Host Port Interrupts
			PortIrqHandler();
		}

		if (TestClearInterrupt(USB_OTG_GINTSTS_SOF)) {			// Handle Host SOF Interrupt
			SofIrqHandler();
		}

		if (TestInterrupt(USB_OTG_GINTSTS_RXFLVL)) {			// Handle Rx Queue Level Interrupts
			USB_OTG_FS->GINTMSK &= ~USB_OTG_GINTSTS_RXFLVL;
			RxqLvlIrqHandler();
			USB_OTG_FS->GINTMSK |= USB_OTG_GINTSTS_RXFLVL;
		}

		if (TestInterrupt(USB_OTG_GINTSTS_HCINT)) {				// Handle Host channel Interrupt
			uint32_t interrupt = USB_HOST->HAINT;

			for (uint32_t i = 0; i < hostChannels; ++i) {
				if ((interrupt & (1UL << (i & 0xF))) != 0) {
					if (USBx_HC(i)->HCCHAR & USB_OTG_HCCHAR_EPDIR) {
						InIrqHandler(i);
					} else {
						OutIrqHandler(i);
					}
				}
			}
			TestClearInterrupt(USB_OTG_GINTSTS_HCINT);
		}
	}
}


void USBHost::ConnectHandler()
{
	device.isConnected = true;
	device.isDisconnected = false;
	device.isReEnumerated = false;
}


void USBHost::DisconnectHandler()
{
	device.isDisconnected = true;
	device.isConnected = false;
	device.portEnabled = false;

	StopHost();

	FreePipe(control.pipeIn);
	FreePipe(control.pipeOut);
}


void USBHost::InIrqHandler(const uint8_t channel)
{
	// Handle Host Channel IN interrupt requests.
	auto USB_HC = USBx_HC(channel);

	if (USB_HC->HCINT & USB_OTG_HCINT_BBERR) {			// Babble error
		USB_HC->HCINT = USB_OTG_HCINT_BBERR;
		hc[channel].state = HostChannel::BabbleError;
		HaltChannel(channel);

	} else if (USB_HC->HCINT & USB_OTG_HCINT_ACK) {
		USB_HC->HCINT = USB_OTG_HCINT_ACK;

	} else if (USB_HC->HCINT & USB_OTG_HCINT_STALL) {
		USB_HC->HCINT = USB_OTG_HCINT_STALL;
		hc[channel].state = HostChannel::Stall;
		HaltChannel(channel);

	} else if (USB_HC->HCINT & USB_OTG_HCINT_DTERR) {	// DTERR: Data toggle error
		USB_HC->HCINT = USB_OTG_HCINT_DTERR;
		hc[channel].state = HostChannel::DataToggleError;
		HaltChannel(channel);

	} else if (USB_HC->HCINT & USB_OTG_HCINT_TXERR) {	// Transaction error ie: CRC check failure, Timeout, Bit stuff error or False EOP
		USB_HC->HCINT = USB_OTG_HCINT_TXERR;
		hc[channel].state = HostChannel::TransactionError;
		HaltChannel(channel);
	}

	if (USB_HC->HCINT & USB_OTG_HCINT_FRMOR) {			// Frame overrun
		HaltChannel(channel);
		USB_HC->HCINT = USB_OTG_HCINT_FRMOR;

	} else if (USB_HC->HCINT & USB_OTG_HCINT_XFRC) {
		hc[channel].state = HostChannel::TransferCompleted;
		hc[channel].errCnt = 0;
		USB_HC->HCINT = USB_OTG_HCINT_XFRC;

		if (hc[channel].epType == ControlEP || hc[channel].epType == Bulk) {
			HaltChannel(channel);
			USB_HC->HCINT = USB_OTG_HCINT_NAK;

		} else if (hc[channel].epType == Interrupt || hc[channel].epType == Isochronous) {
			USB_HC->HCCHAR |= USB_OTG_HCCHAR_ODDFRM;
			hc[channel].urbState = URBState::Done;
		}

		hc[channel].toggleIn ^= 1;

	} else if (USB_HC->HCINT & USB_OTG_HCINT_CHH) {		// Channel halted
		if (hc[channel].state == HostChannel::TransferCompleted) {
			hc[channel].urbState = URBState::Done;

		} else if (hc[channel].state == HostChannel::Stall) {
			hc[channel].urbState = URBState::Stall;

		} else if (hc[channel].state == HostChannel::TransactionError || hc[channel].state == HostChannel::DataToggleError) {
			if (++hc[channel].errCnt > 2) {
				hc[channel].errCnt = 0;
				hc[channel].urbState = URBState::Error;
			} else {
				hc[channel].urbState = URBState::NotReady;
				USB_HC->HCCHAR = (USB_HC->HCCHAR & ~USB_OTG_HCCHAR_CHDIS) | USB_OTG_HCCHAR_CHENA;		// re-activate the channel
			}

		} else if (hc[channel].state == HostChannel::NAK) {
			hc[channel].urbState  = URBState::NotReady;
			USB_HC->HCCHAR = (USB_HC->HCCHAR & ~USB_OTG_HCCHAR_CHDIS) | USB_OTG_HCCHAR_CHENA;		// re-activate the channel

		} else if (hc[channel].state == HostChannel::BabbleError) {
			hc[channel].errCnt++;
			hc[channel].urbState = URBState::Error;
		}

		USB_HC->HCINT = USB_OTG_HCINT_CHH;

	} else if (USB_HC->HCINT & USB_OTG_HCINT_NAK) {
		if (hc[channel].epType == Interrupt) {
			hc[channel].errCnt = 0;
			HaltChannel(channel);

		} else if (hc[channel].epType == ControlEP || hc[channel].epType == Bulk) {
			hc[channel].errCnt = 0;
			hc[channel].state = HostChannel::NAK;
			HaltChannel(channel);
		}
		USB_HC->HCINT = USB_OTG_HCINT_NAK;
	}
}


void USBHost::OutIrqHandler(const uint8_t channel)
{
	// Handle Host Channel OUT interrupt requests.
	auto USB_HC = USBx_HC(channel);

	if ((USB_HC->HCINT & USB_OTG_HCINT_ACK) == USB_OTG_HCINT_ACK) {
		USB_HC->HCINT = USB_OTG_HCINT_ACK;
		if (hc[channel].doPing) {
			hc[channel].doPing = 0;
			hc[channel].urbState = URBState::NotReady;
			HaltChannel(channel);
		}

	} else if (USB_HC->HCINT & USB_OTG_HCINT_FRMOR) {
		USB_HC->HCINT = USB_OTG_HCINT_FRMOR;
		HaltChannel(channel);

	} else if (USB_HC->HCINT & USB_OTG_HCINT_XFRC) {
		hc[channel].errCnt = 0;
		if (USB_HC->HCINT & USB_OTG_HCINT_NYET) {		// transaction completed with NYET state, update do ping state
			hc[channel].doPing = 1;
			USB_HC->HCINT = USB_OTG_HCINT_NYET;
		}
		USB_HC->HCINT = USB_OTG_HCINT_XFRC;
		hc[channel].state = HostChannel::TransferCompleted;
		HaltChannel(channel);

	} else if (USB_HC->HCINT & USB_OTG_HCINT_NYET) {
		hc[channel].state = HostChannel::NYet;
		hc[channel].doPing = 1;
		hc[channel].errCnt = 0;
		HaltChannel(channel);
		USB_HC->HCINT = USB_OTG_HCINT_NYET;

	} else if (USB_HC->HCINT & USB_OTG_HCINT_STALL) {
		USB_HC->HCINT = USB_OTG_HCINT_STALL;
		hc[channel].state = HostChannel::Stall;
		HaltChannel(channel);

	} else if (USB_HC->HCINT & USB_OTG_HCINT_NAK) {
		hc[channel].errCnt = 0;
		hc[channel].state = HostChannel::NAK;
		HaltChannel(channel);
		USB_HC->HCINT = USB_OTG_HCINT_NAK;

	} else if (USB_HC->HCINT & USB_OTG_HCINT_TXERR) {
		hc[channel].state = HostChannel::TransactionError;
		HaltChannel(channel);
		USB_HC->HCINT = USB_OTG_HCINT_TXERR;

	} else if (USB_HC->HCINT & USB_OTG_HCINT_DTERR) {
		hc[channel].state = HostChannel::DataToggleError;
		HaltChannel(channel);
		USB_HC->HCINT = USB_OTG_HCINT_DTERR;

	} else if (USB_HC->HCINT & USB_OTG_HCINT_CHH) {
		if (hc[channel].state == HostChannel::TransferCompleted) {
			hc[channel].urbState  = URBState::Done;
			if (hc[channel].epType == Bulk || hc[channel].epType == Interrupt) {
				hc[channel].toggleOut ^= 1;
			}

		} else if (hc[channel].state == HostChannel::NAK) {
			hc[channel].urbState = URBState::NotReady;

		} else if (hc[channel].state == HostChannel::NYet) {
			hc[channel].urbState  = URBState::NotReady;

		} else if (hc[channel].state == HostChannel::Stall) {
			hc[channel].urbState  = URBState::Stall;

		} else if (hc[channel].state == HostChannel::TransactionError || hc[channel].state == HostChannel::DataToggleError) {
			if (++hc[channel].errCnt > 2) {
				hc[channel].errCnt = 0;
				hc[channel].urbState = URBState::Error;
			} else {
				hc[channel].urbState = URBState::NotReady;
				USB_HC->HCCHAR = (USB_HC->HCCHAR & ~USB_OTG_HCCHAR_CHDIS) | USB_OTG_HCCHAR_CHENA;		// re-activate the channel
			}
		}

		USB_HC->HCINT = USB_OTG_HCINT_CHH;
	}
}


void USBHost::RxqLvlIrqHandler()
{
	// Handle Rx Queue Level interrupt requests.
	const uint32_t gRxStspReg = USB_OTG_FS->GRXSTSP;
	const uint32_t channel = gRxStspReg & USB_OTG_GRXSTSP_EPNUM;
	const uint32_t pktStatus = (gRxStspReg & USB_OTG_GRXSTSP_PKTSTS) >> USB_OTG_GRXSTSP_PKTSTS_Pos;
	const uint32_t pktCount = (gRxStspReg & USB_OTG_GRXSTSP_BCNT) >> USB_OTG_GRXSTSP_BCNT_Pos;

	auto USB_HC = USBx_HC(channel);

	enum ReceivePacketStatus { PacketStatusIn = 2, PacketStatusInTransferComplete = 3, PacketStatusDataToggleError = 5, PacketStatusChannelHalted = 7};

	if (pktStatus == PacketStatusIn) {
		// Read the data into the host buffer.
		if ((pktCount > 0) && (hc[channel].xferBuff != nullptr)) {
			if ((hc[channel].xferCount + pktCount) <= hc[channel].xferLen) {

				ReadPacket((uint32_t*)hc[channel].xferBuff, pktCount, 0);

				hc[channel].xferBuff += pktCount;				// manage multiple packet transfers
				hc[channel].xferCount += pktCount;

				// get transfer size packet count
				const uint32_t xferSizePktCnt = (USB_HC->HCTSIZ & USB_OTG_HCTSIZ_PKTCNT) >> USB_OTG_HCTSIZ_PKTCNT_Pos;

				if (hc[channel].maxPacket == pktCount && xferSizePktCnt > 0) {
					USB_HC->HCCHAR = (USB_HC->HCCHAR & ~USB_OTG_HCCHAR_CHDIS) | USB_OTG_HCCHAR_CHENA;		// re-activate the channel when more packets are expected
					hc[channel].toggleIn ^= 1;
				}
			} else {
				hc[channel].urbState = URBState::Error;
			}
		}
	}
}


void USBHost::PortIrqHandler()
{
	// Handle Host Port interrupt requests.
	const uint32_t hprt0 = USB_HPRT0;
	uint32_t hprt0Upd = USB_HPRT0 &= ~(USB_OTG_HPRT_PENA | USB_OTG_HPRT_PCDET | USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);

	if (hprt0 & USB_OTG_HPRT_PCDET) {				// Check whether Port Connect detected
		if (hprt0 & USB_OTG_HPRT_PCSTS) {
			ConnectHandler();
		}
		hprt0Upd |= USB_OTG_HPRT_PCDET;
	}

	if (hprt0 & USB_OTG_HPRT_PENCHNG) {				// Check whether Port Enable Changed
		hprt0Upd |= USB_OTG_HPRT_PENCHNG;

		if (hprt0 & USB_OTG_HPRT_PENA) {
			if (GetHostSpeed() == SpeedLow) {
				SetPhyClockSpeed(UsbSpeed::HCFG_6_MHz);
			} else {
				SetPhyClockSpeed(UsbSpeed::HCFG_48_MHz);
			}
			device.portEnabled = true;
		} else {
			device.portEnabled = false;
		}
	}

	if (hprt0 & USB_OTG_HPRT_POCCHNG) {				// Check for an overcurrent
		hprt0Upd |= USB_OTG_HPRT_POCCHNG;
	}

	USB_HPRT0 = hprt0Upd;							// Clear Port Interrupts
}


void USBHost::SofIrqHandler()
{
	++timer;
}


void USBHost::FlushTxFifo(const uint32_t fifo)
{
	volatile uint32_t count = 0;
	while (++count < 200000 && (USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0);		// Wait for AHB master IDLE state.
	count = 0;
	USB_OTG_FS->GRSTCTL = (USB_OTG_GRSTCTL_TXFFLSH | (fifo << USB_OTG_GRSTCTL_TXFNUM_Pos));	// Flush TX Fifo
	while (++count < 200000 && (USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH);
}


void USBHost::FlushRxFifo()
{
	volatile uint32_t count = 0;
	while (++count < 200000 && (USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0);		// Wait for AHB master IDLE state.
	count = 0;
	USB_OTG_FS->GRSTCTL = USB_OTG_GRSTCTL_RXFFLSH;											// Flush RX Fifo
	while (++count < 200000 && (USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_RXFFLSH) == USB_OTG_GRSTCTL_RXFFLSH);
}


void USBHost::StartTransfer(const uint8_t channel, const pidToken token, uint8_t* pbuff, const uint16_t length)
{
	hc[channel].xferBuff = pbuff;
	hc[channel].xferCount = 0;
	hc[channel].urbState = URBState::Idle;
	hc[channel].state = HostChannel::Idle;

	// Manage Data Toggle
	uint8_t dataPid = (token == pidToken::Setup) ? PidSetup : PidData1;
	if (hc[channel].epType == ControlEP) {
		if (token == pidToken::Data && hc[channel].epDir == DirOut) {
			if (length == 0) {
				hc[channel].toggleOut = 1;				// For Status OUT stage, Length==0, Status Out PID = 1
			}
			dataPid = (hc[channel].toggleOut == 0) ? PidData0 : PidData1;
		}
	} else if (hc[channel].epType == Bulk || hc[channel].epType == Interrupt) {
		if (hc[channel].epDir == DirOut) {
			dataPid = (hc[channel].toggleOut == 0) ? PidData0 : PidData1;
		} else {
			dataPid = (hc[channel].toggleIn == 0) ? PidData0 : PidData1;
		}
	}

	// Compute the expected number of packets associated to the transfer
	uint16_t numPackets = 1;
	if (length > 0) {
		numPackets = std::min((length + hc[channel].maxPacket - 1) / hc[channel].maxPacket, (int)maxHcPktCount);
	}

	// For IN channel XferSize is expected to be an integer multiple of max_packet size.
	hc[channel].XferSize = (hc[channel].epDir == DirIn) ? numPackets * hc[channel].maxPacket : length;
	hc[channel].xferLen = length;

	auto USB_HC = USBx_HC(channel);
	USB_HC->HCTSIZ = (hc[channel].XferSize & USB_OTG_HCTSIZ_XFRSIZ) |
			((numPackets << USB_OTG_HCTSIZ_PKTCNT_Pos) & USB_OTG_HCTSIZ_PKTCNT) |
			((dataPid << USB_OTG_HCTSIZ_DPID_Pos) & USB_OTG_HCTSIZ_DPID);

	// Set host channel enable
	uint32_t tmpReg = USB_HC->HCCHAR & ~(USB_OTG_HCCHAR_CHDIS | USB_OTG_HCCHAR_EPDIR | USB_OTG_HCCHAR_ODDFRM);
	tmpReg |= ((hc[channel].epDir == DirIn) ? USB_OTG_HCCHAR_EPDIR : 0) |			// Endpoint direction
			  ((USB_HOST->HFNUM & 1) == 0 ? USB_OTG_HCCHAR_ODDFRM : 0) |			// Odd frame toggle
			  USB_OTG_HCCHAR_CHENA;													// Channel Enable
	USB_HC->HCCHAR = tmpReg;

	if (hc[channel].epDir == DirOut && hc[channel].xferLen > 0) {
		if (hc[channel].epType == ControlEP || hc[channel].epType == Bulk) {		// Non periodic transfer
			if ((hc[channel].xferLen + 3) / 4 > (USB_OTG_FS->HNPTXSTS & 0xFFFF)) {	// check if there is enough space in FIFO
				USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_NPTXFEM;						// need to process data in nptxfempty interrupt
			}
		} else {																	// Periodic transfer (Isochronous or interrupt)
			if ((hc[channel].xferLen + 3) / 4 > (USB_HOST->HPTXSTS & 0xFFFF)) { 	// check if there is enough space in FIFO space
				USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_PTXFEM;						// need to process data in ptxfempty interrupt
			}
		}

		WritePacket(hc[channel].xferBuff, channel, hc[channel].xferLen);			// Write packet into the Tx FIFO.
	}
}


void USBHost::WritePacket(const uint8_t* src, const uint8_t endpoint, const uint32_t len)
{
	uint32_t* pSrc = (uint32_t*)src;
	const uint32_t count32b = (len + 3) / 4;

	for (uint32_t i = 0; i < count32b; ++i) {
		USB_FIFO(endpoint) = *pSrc++;
	}
}


void USBHost::ReadPacket(const uint32_t* dest, const uint16_t len, const uint32_t offset)
{
	// Read a packet from the RX FIFO
	uint32_t* pDest = (uint32_t*)dest + offset;
	const uint32_t count32b = (len + 3) / 4;

	for (uint32_t i = 0; i < count32b; ++i)	{
		*pDest++ = USB_FIFO(0);
	}
}




void USBHost::SetPhyClockSpeed(const UsbSpeed freq)
{
	// Initializes the FSLSPClkSel field of the HCFG register on the PHY type and set the right frame interval
	USB_HOST->HCFG &= ~USB_OTG_HCFG_FSLSPCS;
	USB_HOST->HCFG |= (uint32_t)freq & USB_OTG_HCFG_FSLSPCS;				// FS/LS PHY clock select

	if (freq == UsbSpeed::HCFG_48_MHz) {
		USB_HOST->HFIR = 48000;
	} else if (freq == UsbSpeed::HCFG_6_MHz) {
		USB_HOST->HFIR = 6000;
	}
}



void USBHost::ResetPort()
{
	// Reset Host Port: The application must wait at least 10 ms before clearing the reset bit.
	uint32_t hprt0 = USB_HPRT0;
	hprt0 &= ~(USB_OTG_HPRT_PENA | USB_OTG_HPRT_PCDET |	USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);

	USB_HPRT0 = (USB_OTG_HPRT_PRST | hprt0);
	DelayMS(100);
	USB_HPRT0 = ((~USB_OTG_HPRT_PRST) & hprt0);
	DelayMS(10);
}


void USBHost::HaltChannel(const uint8_t channel)
{
	auto USB_HC = USBx_HC(channel);

	// Halt a host channel
	volatile  uint32_t count = 0;
	const uint32_t hcEpType = (USB_HC->HCCHAR & USB_OTG_HCCHAR_EPTYP) >> USB_OTG_HCCHAR_EPTYP_Pos;
	const uint32_t channelEna = (USB_HC->HCCHAR & USB_OTG_HCCHAR_CHENA) >> USB_OTG_HCCHAR_CHENA_Pos;

	if ((USB_OTG_FS->GAHBCFG & USB_OTG_GAHBCFG_DMAEN) && !channelEna) {
		return;
	}

	// Check for space in the request queue to issue the halt
	USB_HC->HCCHAR |= USB_OTG_HCCHAR_CHDIS;

	if (hcEpType == ControlEP || hcEpType == Bulk) {
		if ((USB_OTG_FS->GAHBCFG & USB_OTG_GAHBCFG_DMAEN) == 0) {
			if ((USB_OTG_FS->HNPTXSTS & USB_OTG_HPTXSTS_PTXQSAV_Msk) == 0) {
				USB_HC->HCCHAR &= ~USB_OTG_HCCHAR_CHENA;
				USB_HC->HCCHAR |= USB_OTG_HCCHAR_CHENA;
				do {
					if (count++ > 1000) {
						break;
					}
				} while (USB_HC->HCCHAR & USB_OTG_HCCHAR_CHENA);
			} else {
				USB_HC->HCCHAR |= USB_OTG_HCCHAR_CHENA;
			}
		}
	} else {
		if ((USB_HOST->HPTXSTS & USB_OTG_HPTXSTS_PTXQSAV_Msk) == 0) {
			USB_HC->HCCHAR &= ~USB_OTG_HCCHAR_CHENA;
			USB_HC->HCCHAR |= USB_OTG_HCCHAR_CHENA;
			do {
				if (count++ > 1000) {
					break;
				}
			} while (USB_HC->HCCHAR & USB_OTG_HCCHAR_CHENA);
		} else {
			USB_HC->HCCHAR |= USB_OTG_HCCHAR_CHENA;
		}
	}
}



void USBHost::StopHost()
{
	USB_OTG_FS->GAHBCFG &= ~USB_OTG_GAHBCFG_GINT;

	// Flush USB FIFO
	FlushTxFifo(0x10);
	FlushRxFifo();

	// Flush out any leftover queued requests.
	for (uint8_t i = 0; i <= 15; ++i) {
		uint32_t value = USBx_HC(i)->HCCHAR;
		value |=  USB_OTG_HCCHAR_CHDIS;
		value &= ~USB_OTG_HCCHAR_CHENA;
		value &= ~USB_OTG_HCCHAR_EPDIR;
		USBx_HC(i)->HCCHAR = value;
	}

	// Halt all channels to put them into a known state.
	for (uint8_t i = 0; i <= 15; ++i) {
		uint32_t value = USBx_HC(i)->HCCHAR;
		value |= USB_OTG_HCCHAR_CHDIS;
		value |= USB_OTG_HCCHAR_CHENA;
		value &= ~USB_OTG_HCCHAR_EPDIR;
		USBx_HC(i)->HCCHAR = value;

		volatile uint32_t count = 0;
		while (++count <  1000 && (USBx_HC(i)->HCCHAR & USB_OTG_HCCHAR_CHENA));
	}

	USB_HOST->HAINT = 0xFFFFFFFF;			// Clear any pending Host interrupts
	USB_OTG_FS->GINTSTS = 0xFFFFFFFF;
	USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT;
}


void USBHost::Disable()
{
	DisconnectHandler();
	DeInitStateMachine();									// Restore default states and prepare EP0

	USB_OTG_FS->GUSBCFG &= ~USB_OTG_GUSBCFG_FHMOD;			// Disable host mode
	USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_PWRDWN;				// Deactivate Transceiver

	if (activeClass != nullptr) {
		activeClass->InterfaceDeInit();
		activeClass = nullptr;
	}
}


/*

// Activate or de-activate vbus:  state 0 : Deactivate VBUS; 1 : Activate VBUS
void USB_DriveVbus(uint8_t state)
{
	volatile uint32_t hprt0 = USB_HPRT0;

	hprt0 &= ~(USB_OTG_HPRT_PENA | USB_OTG_HPRT_PCDET | USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);

	if (((hprt0 & USB_OTG_HPRT_PPWR) == 0) && (state == 1)) {
		USB_HPRT0 = (USB_OTG_HPRT_PPWR | hprt0);
	}
	if (((hprt0 & USB_OTG_HPRT_PPWR) == USB_OTG_HPRT_PPWR) && (state == 0)) {
		USB_HPRT0 = ((~USB_OTG_HPRT_PPWR) & hprt0);
	}
}

*/
