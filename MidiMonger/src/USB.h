#pragma once

#include "initialisation.h"

extern uint32_t usbEvents[200], reqEvents[100];
extern uint8_t usbEventNo, eventOcc, reqEventNo, midiEventNo, midiEventRead, midiEventWrite;

// USB Definitions
#define USBx_PCGCCTL    *(__IO uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE)
#define USBx_DEVICE     ((USB_OTG_DeviceTypeDef *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE))
#define USBx_INEP(i)    ((USB_OTG_INEndpointTypeDef *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))
#define USBx_OUTEP(i)   ((USB_OTG_OUTEndpointTypeDef *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))
#define USBx_DFIFO(i)   *(uint32_t*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE + ((i) * USB_OTG_FIFO_SIZE))


// USB Transfer status definitions
#define STS_GOUT_NAK					1U
#define STS_DATA_UPDT					2U
#define STS_XFER_COMP					3U
#define STS_SETUP_COMP					4U
#define STS_SETUP_UPDT					6U

// USB Request Recipient types
#define USB_REQ_RECIPIENT_DEVICE		0x00U
#define USB_REQ_RECIPIENT_INTERFACE		0x01U
#define USB_REQ_RECIPIENT_ENDPOINT		0x02U
#define USB_REQ_RECIPIENT_MASK			0x03U

#define EP_ADDR_MSK						0xFU

// USB Request types
#define USB_REQ_TYPE_STANDARD			0x00U
#define USB_REQ_TYPE_CLASS				0x20U
#define USB_REQ_TYPE_VENDOR				0x40U
#define USB_REQ_TYPE_MASK				0x60U

#define USB_REQ_GET_STATUS				0x00U
#define USB_REQ_CLEAR_FEATURE			0x01U
#define USB_REQ_SET_FEATURE				0x03U
#define USB_REQ_SET_ADDRESS				0x05U
#define USB_REQ_GET_DESCRIPTOR			0x06U
#define USB_REQ_SET_DESCRIPTOR			0x07U
#define USB_REQ_GET_CONFIGURATION		0x08U
#define USB_REQ_SET_CONFIGURATION		0x09U
#define USB_REQ_GET_INTERFACE			0x0AU
#define USB_REQ_SET_INTERFACE			0x0BU
#define USB_REQ_SYNCH_FRAME				0x0CU

#define USBD_IDX_LANGID_STR				0x00U
#define USBD_IDX_MFC_STR				0x01U
#define USBD_IDX_PRODUCT_STR			0x02U
#define USBD_IDX_SERIAL_STR				0x03U
#define USBD_IDX_CONFIG_STR				0x04U
#define USBD_IDX_INTERFACE_STR			0x05U

#define USB_DESC_TYPE_DEVICE			0x01U
#define USB_DESC_TYPE_CONFIGURATION		0x02U
#define USB_DESC_TYPE_STRING			0x03U
#define USB_DESC_TYPE_INTERFACE			0x04U
#define USB_DESC_TYPE_ENDPOINT			0x05U
#define USB_DESC_TYPE_DEVICE_QUALIFIER	0x06U
#define USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION        0x07U
#define USB_DESC_TYPE_BOS				0x0FU

#define USB_LEN_DEV_QUALIFIER_DESC		0x0AU
#define USB_LEN_DEV_DESC				0x12U
#define USB_LEN_CFG_DESC				0x09U
#define USB_LEN_IF_DESC					0x09U
#define USB_LEN_EP_DESC					0x07U
#define USB_LEN_OTG_DESC				0x03U
#define USB_LEN_LANGID_STR_DESC			0x04U
#define USB_LEN_OTHER_SPEED_DESC_SIZ	0x09U
#define USBD_MAX_STR_DESC_SIZ     		512U
#define USBD_CUSTOM_HID_REPORT_DESC_SIZE	0x4A

// EP0 State
#define USBD_EP0_IDLE					0x00U
#define USBD_EP0_SETUP					0x01U
#define USBD_EP0_DATA_IN				0x02U
#define USBD_EP0_DATA_OUT				0x03U
#define USBD_EP0_STATUS_IN				0x04U
#define USBD_EP0_STATUS_OUT				0x05U
#define USBD_EP0_STALL					0x06U

#define USBD_EP_TYPE_CTRL				0x00U
#define USBD_EP_TYPE_ISOC				0x01U
#define USBD_EP_TYPE_BULK				0x02U
#define USBD_EP_TYPE_INTR				0x03U

//  Device Status
#define USBD_STATE_DEFAULT				0x01U
#define USBD_STATE_ADDRESSED			0x02U
#define USBD_STATE_CONFIGURED			0x03U
#define USBD_STATE_SUSPENDED			0x04U


#define USBD_VID						1155
#define USBD_LANGID_STRING				1033
#define USBD_MANUFACTURER_STRING		"Mountjoy Modular"
#define USBD_PID_FS						22352
#define USBD_PRODUCT_STRING_FS			"Mountjoy MIDI"

#define CLASS_SPECIFIC_DESC_SIZE		(7+6+9+9+5+9+5)
#define MIDI_CONFIG_DESC_SIZE 			(9+9+9+9+CLASS_SPECIFIC_DESC_SIZE)
#define USB_DEVICE_CLASS_AUDIO			0x01
#define USB_DEVICE_SUBCLASS_MIDISTREAMING      0x03


#define SWAPBYTE(addr)        (((uint16_t)(*((uint8_t *)(addr)))) + \
		(((uint16_t)(*(((uint8_t *)(addr)) + 1U))) << 8U))
#define LOBYTE(x)  ((uint8_t)(x & 0x00FFU))
#define HIBYTE(x)  ((uint8_t)((x & 0xFF00U) >> 8U))

#define DIR_IN true
#define DIR_OUT false

struct usbRequest {
	uint8_t mRequest;
	uint8_t Request;
	uint16_t Value;
	uint16_t Index;
	uint16_t Length;
};

union MidiData {
	uint32_t data;
	struct {
		uint8_t dummy;
		uint8_t chn : 4;
		uint8_t msg : 4;
		uint8_t db1;
		uint8_t db2;
	};
};
extern MidiData midiArray[MIDIBUFFERSIZE];


typedef enum {
	CUSTOM_HID_IDLE = 0U,
	CUSTOM_HID_BUSY,
}
CUSTOM_HID_StateTypeDef;

class USB {
public:
	void USBInterruptHandler();
	void InitUSB();
	void USB_ActivateEndpoint(uint32_t epnum, bool is_in, uint8_t eptype);
	void USB_ReadPacket(uint32_t *dest, uint16_t len);
	void USB_WritePacket(uint8_t *src, uint32_t ch_ep_num, uint16_t len);
	void USBD_GetDescriptor(usbRequest req);
	void USBD_StdDevReq (usbRequest req);
	void USB_EP0StartXfer(bool is_in, uint8_t epnum, uint32_t xfer_len);
	void USB_EPSetStall(uint8_t epnum);
	bool USB_ReadInterrupts(uint32_t interrupt);
	void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len);
	uint32_t USBD_GetString(uint8_t *desc, uint8_t *unicode);
	void SendReport(uint8_t *report, uint16_t len);

	usbRequest req;
	uint8_t ep0_maxPacket = 0x40;
	uint8_t ep_maxPacket = 0x4;
	uint32_t xfer_buff[32];		// in HAL there is a transfer buffer for each in and out endpoint
	uint32_t xfer_count;
	uint32_t xfer_rem;			// If transfer is larger than maximum packet size store remaining byte count
	//	FIXME - should there be one of these output buffers for each endpoint?
	uint8_t* outBuff;
	uint32_t outBuffSize;
	uint32_t outCount;
	uint32_t ep0_state;
	uint8_t dev_state;
	CUSTOM_HID_StateTypeDef hid_state;

	// USB standard device descriptor - in usbd_desc.c
	uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] = {
			0x12,					// bLength
			USB_DESC_TYPE_DEVICE,	// bDescriptorType
			0x01,					// bcdUSB  - 0x01 if LPM enabled
			0x02,
			0x00,					// bDeviceClass
			0x00,					// bDeviceSubClass
			0x00,					// bDeviceProtocol
			64,  					// bMaxPacketSize
			LOBYTE(USBD_VID),		// idVendor
			HIBYTE(USBD_VID),		// idVendor
			LOBYTE(USBD_PID_FS),	// idProduct
			HIBYTE(USBD_PID_FS),	// idProduct
			0x00,					// bcdDevice rel. 2.00
			0x02,
			USBD_IDX_MFC_STR,		// Index of manufacturer  string
			USBD_IDX_PRODUCT_STR,	// Index of product string
			USBD_IDX_SERIAL_STR,	// Index of serial number string
			1						// bNumConfigurations
	};


	// USB AUDIO device Configuration Descriptor
	uint8_t usbd_audio_CfgDesc[MIDI_CONFIG_DESC_SIZE] =	{

			0x09,							// bLength
			USB_DESC_TYPE_CONFIGURATION,	// bDescriptorType
			LOBYTE(MIDI_CONFIG_DESC_SIZE),	// wTotalLength
			HIBYTE(MIDI_CONFIG_DESC_SIZE),
			0x02,							// bNumInterfaces
			0x01,							// bConfigurationValue
			0x00,							// iConfiguration
			0xC0,							// bmAttributes  = b6 + b7 : self powered
			0x32,							// bMaxPower = 100 mA

			// B.3.1 Standard Audio Control standard Interface Descriptor
			0x09,							// sizeof(usbDescrInterface): length of descriptor in bytes
			0x04,							// interface descriptor type
			0x00,							// index of this interface
			0x00,							// alternate setting for this interface
			0x00,							// endpoints excl 0: number of endpoint descriptors to follow
			0x01,							// AUDIO
			0x01,							// AUDIO_Control
			0x00,							// bInterfaceProtocol
			0x00,							// string index for interface

			// B.3.2 Class-specific AC Interface Descriptor
			0x09,							// sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes
			0x24,							// descriptor type
			0x01,							// header functional descriptor
			0x00, 0x01,						// bcdADC
			0x09, 0x00,						// wTotalLength
			0x01,							// bInCollection
			0x01,							// baInterfaceNr[1]

			// B.4 MIDIStreaming Interface Descriptors

			// B.4.1 Standard MS Interface Descriptor
			0x09,							// bLength
			0x04,							// bDescriptorType
			0x01,							// bInterfaceNumber
			0x00,							// bAlternateSetting
			0x02,							// bNumEndpoints
			USB_DEVICE_CLASS_AUDIO,			// bInterfaceClass
			USB_DEVICE_SUBCLASS_MIDISTREAMING,	// bInterfaceSubClass: MIDIStreaming
			0x00,							// InterfaceProtocol
			0x00,							// iInterface: No String Descriptor

			// B.4.2 Class-specific MS Interface Descriptor
			0x07,							// length of descriptor in bytes
			0x24,							// bDescriptorType: Class Specific Interface Descriptor
			0x01,							// header functional descriptor
			0x0, 0x01,						// bcdADC
			CLASS_SPECIFIC_DESC_SIZE, 0,	// wTotalLength

			// B.4.3 MIDI IN Jack Descriptor (Embedded)
			0x06,							// bLength
			0x24,							// descriptor type
			0x02,							// bDescriptorSubtype: MIDI_IN_JACK
			0x01,							// bJackType: Embedded
			0x01,							// bJackID
			0x00,							// iJack: No String Descriptor

			// Table B4.4 Midi Out Jack Descriptor (Embedded)
			0x09,							// length of descriptor in bytes
			0x24,							// descriptor type
			0x03,							// MIDI_OUT_JACK descriptor
			0x01,							// bJackType: Embedded
			0x02,							// bJackID
			0x01,							// No of input pins
			0x01,							// ID of the Entity to which this Pin is connected.
			0x01,							// Output Pin number of the Entity to which this Input Pin is connected.
			0X00,							// iJack

			//B.5.1 Standard Bulk OUT Endpoint Descriptor
			0x09,							// bLength
			0x05,							// bDescriptorType = endpoint
			0x01,							// bEndpointAddress OUT endpoint number 1
			0x02,							// bmAttributes: 2:Bulk, 3:Interrupt endpoint
			0x40, 0X00,						// wMaxPacketSize 64 bytes per packet.
			0x00,							// bInterval in ms : ignored for bulk
			0x00,							// bRefresh Unused
			0x00,							// bSyncAddress Unused

			// B.5.2 Class-specific MS Bulk OUT Endpoint Descriptor
			0x05,							// bLength of descriptor in bytes
			0x25,							// bDescriptorType (Audio Endpoint Descriptor)
			0x01,							// bDescriptorSubtype: MS General
			0x01,							// bNumEmbMIDIJack
			0x01,							// baAssocJackID (0) ID of the Embedded MIDI IN Jack.

			//B.6.1 Standard Bulk IN Endpoint Descriptor
			0x09,							// bLength
			0x05,							// bDescriptorType = endpoint
			0x81,							// bEndpointAddress IN endpoint number 1
			0X02,							// bmAttributes: 2: Bulk, 3: Interrupt endpoint
			0x40, 0X00,						// wMaxPacketSize
			0X00,							// bInterval in ms
			0X00,							// bRefresh
			0X00,							// bSyncAddress

			// B.6.2 Class-specific MS Bulk IN Endpoint Descriptor
			0X05,							// bLength of descriptor in bytes
			0X25,							// bDescriptorType
			0x01,							// bDescriptorSubtype
			0X01,							// bNumEmbMIDIJack (0)
			0X02,							// baAssocJackID (0) ID of the Embedded MIDI OUT Jack

	};

	// Binary Object Store (BOS) Descriptor
	uint8_t USBD_FS_BOSDesc[0xC] = {
			0x05,						// Length
			USB_DESC_TYPE_BOS,			// DescriptorType
			0x0C,						// TotalLength
			0x00, 0x01,					// NumDeviceCaps

			// USB 2.0 Extension Descriptor: device capability
			0x07,						// bLength
			0x10, 						// USB_DEVICE_CAPABITY_TYPE
			0x02,						// Attributes
			0x02, 0x00, 0x00, 0x00		// Link Power Management protocol is supported

	};


	uint8_t USBD_StringSerial[0x1A] = {
			0x1A,		// size
			0x03, 		// USB_DESC_TYPE_STRING
	};

	// USB lang indentifier descriptor
	uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] = {
			USB_LEN_LANGID_STR_DESC,
			USB_DESC_TYPE_STRING,
			LOBYTE(USBD_LANGID_STRING),
			HIBYTE(USBD_LANGID_STRING)
	};

	uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ];

	uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] = {
			0x05,   0x01,
			0x09,   0x02,
			0xA1,   0x01,
			0x09,   0x01,

			0xA1,   0x00,
			0x05,   0x09,
			0x19,   0x01,
			0x29,   0x03,

			0x15,   0x00,
			0x25,   0x01,
			0x95,   0x03,
			0x75,   0x01,

			0x81,   0x02,
			0x95,   0x01,
			0x75,   0x05,
			0x81,   0x01,

			0x05,   0x01,
			0x09,   0x30,
			0x09,   0x31,
			0x09,   0x38,

			0x15,   0x81,
			0x25,   0x7F,
			0x75,   0x08,
			0x95,   0x03,

			0x81,   0x06,
			0xC0,   0x09,
			0x3c,   0x05,
			0xff,   0x09,

			0x01,   0x15,
			0x00,   0x25,
			0x01,   0x75,
			0x01,   0x95,

			0x02,   0xb1,
			0x22,   0x75,
			0x06,   0x95,
			0x01,   0xb1,

			0x01,
			0xC0
	};
};
