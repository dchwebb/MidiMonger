#pragma once

#include "initialisation.h"
#include "USBClass.h"
#include <stdio.h>


#define USB_HOST	((USB_OTG_HostTypeDef*)((uint32_t)USB_OTG_FS + USB_OTG_HOST_BASE))
#define USB_HPRT0	*(volatile uint32_t*)((uint32_t)USB_OTG_FS + USB_OTG_HOST_PORT_BASE)
#define USB_FIFO(i)	*(volatile uint32_t*)((uint32_t)USB_OTG_FS + USB_OTG_FIFO_BASE + ((i) * USB_OTG_FIFO_SIZE))
#define USBx_HC(i)	(*(USB_OTG_HostChannelTypeDef*)((uint32_t)USB_OTG_FS + USB_OTG_HOST_CHANNEL_BASE + ((i) * USB_OTG_HOST_CHANNEL_SIZE)))



class USBHost
{
public:
	enum EndPointType { ControlEP = 0, Isochronous = 1, Bulk = 2, Interrupt = 3 };
	enum Direction { DirOut = 0, DirIn = 1 };
	enum USBMode { DeviceMode = 0, HostMode = 1, DRDMode = 2 };
	enum SpeedType {SpeedHigh = 0, SpeedFull = 1, SpeedLow = 2 };
	enum class UsbSpeed { HCFG_30_60_MHhz, HCFG_48_MHz, HCFG_6_MHz };
	enum class URBState { Idle, Done, NotReady, NotYet, Error, Stall };
	enum { PidData0 = 0, PidData2 = 1, PidData1 = 2, PidSetup = 3 };
	enum class pidToken { Setup, Data };
	enum { FeatureSelectorEndpoint = 0, FeatureSelectorRemoteWakeup = 1 };
	enum StandardRequestCodes { RequestGetStatus = 0, RequestClearFeature = 1, RequestSetFeature = 3, RequestSetAddress = 5,
		RequestGetDescriptor = 6, RequestSetDescriptor = 7, RequestGetConfiguration = 8, RequestSetConfiguration = 9,
		RequestGetInterface = 10, RequestSetInterface = 11, RequestSynchFrame = 12 };

	static constexpr uint32_t devResetTimeout = 1000;
	static constexpr uint32_t deviceAddress = 1;
	static constexpr uint32_t epTypeMask = 3;

	static constexpr uint32_t deviceDescriptorSize = 18;
	static constexpr uint32_t configurationDescriptorSize = 9;
	static constexpr uint32_t interfaceDescriptorSize = 9;
	static constexpr uint32_t endpointDescriptorSize = 7;
	static constexpr uint32_t setupPacketSize = 8;

	//// bmRequestType :D7 Data Phase Transfer Direction
	static constexpr uint32_t requestDirectionMask = 0x80;
	static constexpr uint32_t HostToDevice = 0x00;
	static constexpr uint32_t DeviceToHost = 0x80;

	// bmRequestType D4..0 Recipient
	static constexpr uint32_t requestRecipientDevice = 0;
	static constexpr uint32_t requestRecipientInterface = 1;
	static constexpr uint32_t requestRecipientEndpoint = 2;
	static constexpr uint32_t requestRecipientOther = 3;

	// bmRequestType D6..5 Type
	static constexpr uint32_t requestTypeStandard = 0x00;
	static constexpr uint32_t RequestTypeClass    = 0x20;
	static constexpr uint32_t requestTypeVendor   = 0x40;
	static constexpr uint32_t requestTypeReserved = 0x60;

	// Table 9-5. Descriptor Types of USB Specifications
	static constexpr uint32_t descriptorTypeString = 3;
	static constexpr uint32_t descriptorTypeInterface = 4;
	static constexpr uint32_t descriptorTypeEndpoint = 5;

	// Descriptor Type and Descriptor Index: for calling the function USBH_GetDescriptor
	static constexpr uint32_t descriptorDevice = 0x100;
	static constexpr uint32_t descriptorConfiguration = 0x200;
	static constexpr uint32_t descriptorString = 0x300;

	static constexpr uint32_t maxDataBuffer	= 1024;
	static constexpr uint32_t maxEpPacketSize = 1024;
	static constexpr uint8_t  maxNumEndpoints = 4;
	static constexpr uint32_t maxNumInterfaces = 8;
	static constexpr uint32_t maxNumSupportedClass = 2;
	static constexpr uint16_t maxSizeConfiguration = 512;
	static constexpr uint32_t maxNumPipes = 16;
	static constexpr uint32_t maxErrorCount = 2;
	static constexpr uint32_t maxPacketSizeDefault = 64;
	static constexpr uint16_t maxHcPktCount = 256;

	struct EndpointDescriptor {
		uint8_t   bLength;
		uint8_t   bDescriptorType;
		uint8_t   bEndpointAddress;		// indicates what endpoint this descriptor is describing
		uint8_t   bmAttributes;			// specifies the transfer type.
		uint16_t  wMaxPacketSize;		// Maximum Packet Size this endpoint is capable of sending or receiving
		uint8_t   bInterval;			// is used to specify the polling interval of certain transfers.
	};

	struct InterfaceDescriptor {
		uint8_t bLength;
		uint8_t bDescriptorType;
		uint8_t bInterfaceNumber;
		uint8_t bAlternateSetting;		// Value used to select alternative setting
		uint8_t bNumEndpoints;			// Number of Endpoints used for this interface
		uint8_t bInterfaceClass;		// Class Code (Assigned by USB Org)
		uint8_t bInterfaceSubClass;		// Subclass Code (Assigned by USB Org)
		uint8_t bInterfaceProtocol;		// Protocol Code
		uint8_t iInterface;				// Index of String Descriptor Describing this interface
		EndpointDescriptor epDesc[maxNumEndpoints];
	};

	struct DeviceDescriptor {
		uint8_t   bLength;
		uint8_t   bDescriptorType;
		uint16_t  bcdUSB;				// USB Specification Number which device complies too
		uint8_t   bDeviceClass;
		uint8_t   bDeviceSubClass;
		uint8_t   bDeviceProtocol;
		uint8_t   bMaxPacketSize;
		uint16_t  idVendor;				// Vendor ID (Assigned by USB Org)
		uint16_t  idProduct;			// Product ID (Assigned by Manufacturer)
		uint16_t  bcdDevice;			// Device Release Number
		uint8_t   iManufacturer;		// Index of Manufacturer String Descriptor
		uint8_t   iProduct;				// Index of Product String Descriptor
		uint8_t   iSerialNumber;		// Index of Serial Number String Descriptor
		uint8_t   bNumConfigurations;	// Number of Possible Configurations
	};

	struct {
		uint8_t		cfgDescRaw[maxSizeConfiguration];
		uint8_t		data[maxDataBuffer];	// To receive raw descriptor data
		uint8_t		address;
		uint8_t		speed;
		uint8_t		enumCount;
		uint8_t		resetCount;
		bool		isConnected;
		bool		isDisconnected;
		bool		isReEnumerated;
		bool		portEnabled;
		uint8_t		currentInterface;
		DeviceDescriptor	devDesc;
		struct {
			uint8_t	bLength;
			uint8_t	bDescriptorType;
			uint16_t wTotalLength;			// Total Length of Data Returned
			uint8_t	bNumInterfaces;			// Number of Interfaces
			uint8_t	bConfigurationValue; 	// Value to use as an argument to select this configuration
			uint8_t	iConfiguration;			// Index of String Descriptor Describing this configuration
			uint8_t	bmAttributes;	 		// D7 Bus Powered , D6 Self Powered, D5 Remote Wakeup , D4..0 Reserved (0)
			uint8_t	bMaxPower;				// Maximum Power Consumption
			InterfaceDescriptor ifDesc[maxNumInterfaces];
		} cfgDesc;
	} device;

	uint32_t timer;

	void Init();
	void Process();
	void HostChannelInit(uint8_t channel, uint8_t epNum, uint8_t devAddress, uint8_t speed, uint8_t epType, uint16_t mps);
	HostStatus ClearFeature(uint8_t ep_num);
	uint8_t AllocPipe(uint8_t ep);
	void FreePipe(uint8_t idx);
	InterfaceDescriptor* SelectInterface(uint8_t Class, uint8_t subClass);
	void SetToggle(uint8_t pipe, uint8_t toggle);
	uint32_t GetTransferCount(uint8_t channel);
	void IRQHandler();
	void StartTransfer(uint8_t channel, pidToken token, uint8_t* pbuff, uint16_t length);
	URBState GetURBState(uint8_t channel);
	void HaltChannel(uint8_t hc_num);
	void Disable();
	HostStatus ClassRequest(const uint8_t reqType, const uint8_t request, const uint16_t value, const uint16_t index, uint8_t* buff, const uint16_t length);


private:

	struct DescHeader {
		uint8_t  bLength;
		uint8_t  bDescriptorType;
	};

	void Start();
	void DeInitStateMachine();
	HostStatus DoEnumeration();
	void EnumerationError();

	HostStatus GetDevDesc(uint16_t length);
	HostStatus GetDescriptor(uint8_t reqType, uint16_t value_idx, uint8_t* buff, uint16_t length);
	HostStatus GetStringDesc(uint8_t stringIndex, uint8_t* buff, uint16_t length);
	HostStatus GetConfigDesc(uint16_t length);
	void ParseDevDesc(DeviceDescriptor& devDesc, uint8_t* buf, uint16_t length);
	void ParseStringDesc(uint8_t* psrc, uint8_t* pdest, uint16_t length);
	HostStatus ParseEPDesc(EndpointDescriptor& ep_descr, uint8_t* buf);
	void ParseInterfaceDesc(InterfaceDescriptor* ifDescriptor, uint8_t* buf);
	DescHeader* GetNextDesc(uint8_t* pbuf, uint16_t* ptr);

	HostStatus ControlCommand(uint8_t request, uint16_t wValue, uint16_t index);
	HostStatus CtlReq(uint8_t* buff, uint16_t length);
	HostStatus HandleControl();

	bool TestClearInterrupt(uint32_t interrupt);
	bool TestInterrupt(uint32_t interrupt);

	void ConnectHandler();
	void DisconnectHandler();
	void InIrqHandler(uint8_t channel);
	void OutIrqHandler(uint8_t channel);
	void PortIrqHandler();
	void RxqLvlIrqHandler();
	void SofIrqHandler();

	uint32_t GetHostSpeed();
	uint32_t GetMode();

	void FlushTxFifo(uint32_t num);
	void FlushRxFifo();

	void WritePacket(const uint8_t* src, const uint8_t endpoint, const uint32_t len);
	void ReadPacket(const uint32_t* dest, const uint16_t len, const uint32_t offset);

	void SetPhyClockSpeed(UsbSpeed freq);
	void ResetPort();
	void StopHost();


	const uint32_t hostChannels = 12;	// Host Channels number.
	uint32_t speed = SpeedFull;			// USB Core speed.

	struct HostChannel {
		uint8_t	epDir;					// Endpoint direction
		uint8_t	doPing;					// Enable or disable the use of the PING protocol for HS mode.
		uint8_t	processPing;			// Execute the PING protocol for HS mode.
		uint8_t	epType;					// Endpoint Type.
		uint16_t maxPacket;				// Endpoint Max packet size.
		uint8_t* xferBuff;				// Pointer to transfer buffer.
		uint32_t XferSize;				// OTG Channel transfer size.
		uint32_t xferLen;				// Current transfer length.
		uint32_t xferCount;				// Partial transfer length in case of multi packet transfer.
		uint8_t	toggleIn;				// IN transfer current toggle flag.
		uint8_t	toggleOut;				// OUT transfer current toggle flag
		uint32_t errCnt;				// Host channel error count.
		URBState urbState;				// URB state.
		enum { Idle, TransferCompleted, Halted, NAK, NYet, Stall, TransactionError, BabbleError, DataToggleError } state;
	} hc[16];


	enum class StateType { Idle, Send, Wait} requestState;
	uint32_t timeout;

	struct Control {
		uint8_t	pipeIn;
		uint8_t	pipeOut;
		uint8_t	pipeSize;
		uint8_t	*buff;
		uint16_t length;
		struct SetupPacket {
			uint8_t   bmRequestType;
			uint8_t   bRequest;
			uint16_t  wValue;
			uint16_t  wIndex;
			uint16_t  wLength;
		} setup;

		enum class StateType { Idle, Setup, SetupWait, DataIn, DataInWait, DataOut, DataOutWait,
			StatusIn, StatusInWait, StatusOut, StatusOutWait, Error, Stalled, Complete } state;

		uint8_t errorCount;
	} control;




	uint32_t classCount = 0;					// Number of supported USB classes
	USBClass* classes[maxNumSupportedClass];	// Holds pointers to all supported USB classes
	USBClass* activeClass = nullptr;			// Holds pointer to currently active USB class
	uint32_t pipes[16];


	enum class HostState { Idle, DevWaitForAttachment, DevAttached, DevDisconnected, DetectDeviceSpeed, Enumeration, Input,
		SetConfiguration, SetWakeupFeature, CheckClass, Class, Suspended, Abort
	} gState;


	enum class EnumState { Idle, GetFullDevDesc, SetAddr, GetCfgDesc, GetFullCfgDesc, GetMfcStringDesc, GetProductStringDesc, GetSerialStringDesc
	} enumState;

	//GpioPin PowerEnable {GPIOG, 6, GpioPin::Type::Output};
};



extern USBHost usbHost;









