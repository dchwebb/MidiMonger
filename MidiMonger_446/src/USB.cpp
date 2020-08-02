#include <USB.h>

	/* startup sequence:
0		40000000 SRQINT 	Session request/new session detected
1		800		USBSUSP 	USB suspend
2		80000000 WKUINT 	Resume/remote wakeup detected
3		1000	USBRST 		USB reset
4		2000	ENUMDNE 	Enumeration done
5		10 		RXFLVL
6  		10
7		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,100,0,40		Device descriptor USBD_FS_DeviceDesc
8		40000	IEPINT  	USB_OTG_DIEPINT_TXFE  Transmit FIFO empty
9		40000	IEPINT  	USB_OTG_DIEPINT_XFRC  Transfer completed
10 		10
11 		10
12		80000				USB_OTG_DOEPINT_XFRC
13		10					Address setup happens here
14 		10
15		80000	OEPINT		USB_OTG_DOEPINT_STUP req 0x0,5,31,0				Address setup - third param is address (0x31 in this case)
16		40000	IEPINT		USB_OTG_DIEPINT_XFRC
17		10					STS_SETUP_UPDT
18		10					STS_SETUP_COMP
19		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,100,12			Device descriptor again but with device address (rather than 0)
20		40000	IEPINT		USB_OTG_DIEPINT_TXFE
21		40000	IEPINT		USB_OTG_DIEPINT_XFRC
22		10					STS_DATA_UPDT
23		10					STS_SETUP_UPDT
24		80000	OEPINT 		USB_OTG_DOEPINT_XFRC
25		10					STS_SETUP_UPDT
26		10					STS_SETUP_COMP
27		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,200,0,FF		configuration descriptor usbd_audio_CfgDesc
28		40000	IEPINT		USB_OTG_DIEPINT_TXFE
29		40000	IEPINT		USB_OTG_DIEPINT_XFRC
30		40000	IEPINT		USB_OTG_DIEPINT_TXFE 							second part of configuration descriptor
31		40000	IEPINT		USB_OTG_DIEPINT_XFRC
32		10					STS_DATA_UPDT
33		10					STS_SETUP_UPDT
34		80000	OEPINT 		USB_OTG_DOEPINT_XFRC
35		10					STS_SETUP_UPDT
36		10					STS_SETUP_COMP
37		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,F00,0,FF		USBD_FS_BOSDesc
38		40000	IEPINT		USB_OTG_DIEPINT_TXFE
39		40000	IEPINT		USB_OTG_DIEPINT_XFRC
40		10					STS_DATA_UPDT
41		10					STS_SETUP_UPDT
42		80000	OEPINT 		USB_OTG_DOEPINT_XFRC
43		10					STS_SETUP_UPDT
44		10					STS_SETUP_COMP
45		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,303,409,FF		USBD_IDX_SERIAL_STR
46		40000	IEPINT		USB_OTG_DIEPINT_TXFE
47		40000	IEPINT		USB_OTG_DIEPINT_XFRC
48		10					STS_DATA_UPDT
49		10					STS_SETUP_UPDT
50		80000	OEPINT 		USB_OTG_DOEPINT_XFRC
51		10					STS_SETUP_UPDT
52		10					STS_SETUP_COMP
53		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,300,0,FF		USBD_IDX_LANGID_STR
54		40000	IEPINT		USB_OTG_DIEPINT_TXFE
55		40000	IEPINT		USB_OTG_DIEPINT_XFRC
56		10					STS_DATA_UPDT
57		10					STS_SETUP_UPDT
58		80000	OEPINT 		USB_OTG_DOEPINT_XFRC
59		10					STS_SETUP_UPDT
60		10					STS_SETUP_COMP
61		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,302,409,FF		USBD_IDX_PRODUCT_STR
62		40000	IEPINT		USB_OTG_DIEPINT_TXFE
63		40000	IEPINT		USB_OTG_DIEPINT_XFRC
64		10					STS_DATA_UPDT
65		10					STS_SETUP_UPDT
66		80000	OEPINT 		USB_OTG_DOEPINT_XFRC
67		10					STS_SETUP_UPDT
68		10					STS_SETUP_COMP
69		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,600,A			USB_DESC_TYPE_DEVICE_QUALIFIER > Stall
70		10					STS_DATA_UPDT
71		10					STS_SETUP_UPDT
72		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0,9,1,0,0 				Set configuration to 1
73		40000	IEPINT		USB_OTG_DIEPINT_XFRC
74		10					STS_DATA_UPDT
75		10					STS_SETUP_UPDT
76		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,302,409,4		USBD_IDX_PRODUCT_STR
77		40000	IEPINT		USB_OTG_DIEPINT_TXFE
78		40000	IEPINT		USB_OTG_DIEPINT_XFRC
79		10					STS_DATA_UPDT
80		10					STS_SETUP_UPDT
81		80000	OEPINT 		USB_OTG_DOEPINT_XFRC
82		10					STS_SETUP_UPDT reads
83		10					STS_SETUP_COMP
84		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,302,409,1C		USBD_IDX_PRODUCT_STR

92		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,302,409,1C		USBD_IDX_PRODUCT_STR

100		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,302,409,1C		USBD_IDX_PRODUCT_STR

108		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,600,A 			USB_DESC_TYPE_DEVICE_QUALIFIER > Stall

111		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,300,0,1FE		USBD_IDX_LANGID_STR

119		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,301,409,1FE		USBD_IDX_MFC_STR

127		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,302,409,1FE		USBD_IDX_PRODUCT_STR

135		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,3EE,409,1FE		Custom user string?

*/

void USB::USBInterruptHandler() {		// In Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_pcd.c

	int epnum, ep_intr, epint;

	// Handle spurious interrupt
	if ((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) == 0)
		return;


	///////////		10			RXFLVL: RxQLevel Interrupt:  Rx FIFO non-empty Indicates that there is at least one packet pending to be read from the Rx FIFO.
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_RXFLVL))
	{
		USB_OTG_FS->GINTMSK &= ~USB_OTG_GINTSTS_RXFLVL;

		uint32_t receiveStatus = USB_OTG_FS->GRXSTSP;		// OTG status read and pop register: not shown in SFR, but read only (ie do not pop) register under OTG_FS_GLOBAL->FS_GRXSTSR_Device
		epnum = receiveStatus & USB_OTG_GRXSTSP_EPNUM;		// Get the endpoint number
		uint16_t packetSize = (receiveStatus & USB_OTG_GRXSTSP_BCNT) >> 4;

		usbDebug[usbDebugNo].IntData = receiveStatus;
		usbDebug[usbDebugNo].endpoint = epnum;
		usbDebug[usbDebugNo].PacketSize = packetSize;

		if (((receiveStatus & USB_OTG_GRXSTSP_PKTSTS) >> 17) == STS_DATA_UPDT && packetSize != 0) {		// 2 = OUT data packet received
			USB_ReadPacket(xfer_buff, packetSize);
		} else if (((receiveStatus & USB_OTG_GRXSTSP_PKTSTS) >> 17) == STS_SETUP_UPDT) {				// 6 = SETUP data packet received
			USB_ReadPacket(xfer_buff, 8U);
			}
		if (packetSize != 0) {
			xfer_count = packetSize;
			usbDebug[usbDebugNo].xferBuff0 = xfer_buff[0];
			usbDebug[usbDebugNo].xferBuff1 = xfer_buff[1];
		}
		USB_OTG_FS->GINTMSK |= USB_OTG_GINTSTS_RXFLVL;
	}


	/////////// 	80000 		OEPINT OUT endpoint interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_OEPINT)) {

		// Read the output endpoint interrupt register to ascertain which endpoint(s) fired an interrupt
		ep_intr = ((USBx_DEVICE->DAINT & USBx_DEVICE->DAINTMSK) & 0xFFFF0000U) >> 16; // FIXME mask unnecessary with shift right?

		// process each endpoint in turn incrementing the epnum and checking the interrupts (ep_intr) if that endpoint fired
		epnum = 0;
		while (ep_intr != 0) {
			if ((ep_intr & 1) != 0) {
				epint = USBx_OUTEP(epnum)->DOEPINT & USBx_DEVICE->DOEPMSK;

				usbDebug[usbDebugNo].endpoint = epnum;
				usbDebug[usbDebugNo].IntData = epint;

				if ((epint & USB_OTG_DOEPINT_XFRC) == USB_OTG_DOEPINT_XFRC) {		// 0x01 Transfer completed

					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_XFRC;				// Clear interrupt

					if (epnum == 0) {

				        // In CDC mode after 0x21 0x20 packets (line coding commands)
						if (dev_state == USBD_STATE_CONFIGURED && CmdOpCode != 0) {
							if (CmdOpCode == 0x20) {			// SET_LINE_CODING - capture the data passed to return when queried with GET_LINE_CODING
								for (uint8_t i = 0; i < outBuffSize; ++i) {
									((uint8_t*)USBD_CDC_LineCoding)[i] = ((uint8_t*)xfer_buff)[i];
								}
							}
							USB_EP0StartXfer(DIR_IN, 0, 0);
							CmdOpCode = 0;
						}

						ep0_state = USBD_EP0_IDLE;
						USBx_OUTEP(epnum)->DOEPCTL |= USB_OTG_DOEPCTL_STALL;
					} else {
						// Add buffer contents to midiArray
						//midiArray[midiEventWrite++].data = *xfer_buff;
						//if (midiEventWrite >= MIDIBUFFERSIZE)	midiEventWrite = 0;

						USB_EP0StartXfer(DIR_OUT, epnum, xfer_count);
						dataHandler((uint8_t*)xfer_buff, xfer_count);
					}
				}

				if ((epint & USB_OTG_DOEPINT_STUP) == USB_OTG_DOEPINT_STUP) {		// SETUP phase done: the application can decode the received SETUP data packet.
					// Parse Setup Request containing data in xfer_buff filled by RXFLVL interrupt
					req.loadData((uint8_t*)xfer_buff);

					usbDebug[usbDebugNo].Request = req;

					ep0_state = USBD_EP0_SETUP;

					switch (req.mRequest & 0x1F) {		// originally USBD_LL_SetupStage
					case USB_REQ_RECIPIENT_DEVICE:
						//initially USB_REQ_GET_DESCRIPTOR >> USB_DESC_TYPE_DEVICE (bmrequest is 0x6)
						USBD_StdDevReq(req);
						break;

					case USB_REQ_RECIPIENT_INTERFACE:
						if ((req.mRequest & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_CLASS) {		// 0xA1 & 0x60 == 0x20

							if (req.Length > 0) {
								if ((req.mRequest & USB_REQ_DIRECTION_MASK) != 0U) {		// Device to host [USBD_CtlSendData]
									// CDC request 0xA1, 0x21, 0x0, 0x0, 0x7		GetLineCoding 0xA1 0x21 0 Interface 7; Data: Line Coding Data Structure
									// 0xA1 [1|01|00001] Device to host | Class | Interface

									outBuffSize = req.Length;
									outBuff = (uint8_t*)USBD_CDC_LineCoding;
									ep0_state = USBD_EP0_DATA_IN;

									usbDebug[usbDebugNo].PacketSize = outBuffSize;
									usbDebug[usbDebugNo].xferBuff0 = ((uint32_t*)outBuff)[0];
									usbDebug[usbDebugNo].xferBuff1 = ((uint32_t*)outBuff)[1];

									USB_EP0StartXfer(DIR_IN, 0, req.Length);		// sends blank request back
								} else {
									//CDC request 0x21, 0x20, 0x0, 0x0, 0x7			// USBD_CtlPrepareRx
									// 0x21 [0|01|00001] Host to device | Class | Interface
									CmdOpCode = req.Request;
									USB_EP0StartXfer(DIR_OUT, epnum, req.Length);
								}
							} else {
								// 0x21, 0x22, 0x0, 0x0, 0x0	SetControlLineState 0x21 | 0x22 | 2 | Interface | 0 | None
								// 0x21, 0x20, 0x0, 0x0, 0x0	SetLineCoding       0x21 | 0x20 | 0 | Interface | 0 | Line Coding Data Structure
								USB_EP0StartXfer(DIR_IN, 0, 0);
							}

						} else if (req.mRequest == 0x81) {

							if (req.Value >> 8 == 0x22)		// 0x22 = CUSTOM_HID_REPORT_DESC
							{
								outBuffSize = std::min((uint16_t)0x4A, req.Length);		// 0x4A = USBD_CUSTOM_HID_REPORT_DESC_SIZE
								outBuff = CUSTOM_HID_ReportDesc_FS;
								ep0_state = USBD_EP0_DATA_IN;
								USB_EP0StartXfer(DIR_IN, 0, outBuffSize);
							}
						}
						break;

					case USB_REQ_RECIPIENT_ENDPOINT:
						break;

					default:
						//USBD_LL_StallEP(pdev, (req.mRequest & 0x80U));
						break;
					}

					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_STUP;				// Clear interrupt
				}

				if ((epint & USB_OTG_DOEPINT_OTEPDIS) == USB_OTG_DOEPINT_OTEPDIS) {	// OUT token received when endpoint disabled
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_OTEPDIS;			// Clear interrupt
				}
				if ((epint & USB_OTG_DOEPINT_OTEPSPR) == USB_OTG_DOEPINT_OTEPSPR) {	// Status Phase Received interrupt
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_OTEPSPR;			// Clear interrupt
				}
				if ((epint & USB_OTG_DOEPINT_NAK) == USB_OTG_DOEPINT_NAK) {			// 0x2000 OUT NAK interrupt
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_NAK;				// Clear interrupt
				}
			}
			epnum++;
			ep_intr >>= 1U;
		}

	}

	///////////		40000 		IEPINT: IN endpoint interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_IEPINT))
	{
		// Read in the device interrupt bits [initially 1]
		ep_intr = (USBx_DEVICE->DAINT & USBx_DEVICE->DAINTMSK) & 0xFFFFU;

		// process each endpoint in turn incrementing the epnum and checking the interrupts (ep_intr) if that endpoint fired
		epnum = 0;
		while (ep_intr != 0) {
			if ((ep_intr & 1) != 0) { // In ITR [initially true]
				usbDebug[usbDebugNo].endpoint = epnum;

				epint = USBx_INEP((uint32_t)epnum)->DIEPINT & (USBx_DEVICE->DIEPMSK | (((USBx_DEVICE->DIEPEMPMSK >> (epnum & EP_ADDR_MSK)) & 0x1U) << 7));
				usbDebug[usbDebugNo].IntData = epint;

				if ((epint & USB_OTG_DIEPINT_XFRC) == USB_OTG_DIEPINT_XFRC) {			// 0x1 Transfer completed interrupt
					uint32_t fifoemptymsk = (uint32_t)(0x1UL << (epnum & EP_ADDR_MSK));
					USBx_DEVICE->DIEPEMPMSK &= ~fifoemptymsk;

					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_XFRC;

					if (epnum == 0) {

						if (ep0_state == USBD_EP0_DATA_IN) {
							if (xfer_rem > 0) {
								outBuffSize = xfer_rem;
								xfer_rem = 0;

								usbDebug[usbDebugNo].PacketSize = outBuffSize;
								usbDebug[usbDebugNo].xferBuff0 = ((uint32_t*)outBuff)[0];
								usbDebug[usbDebugNo].xferBuff1 = ((uint32_t*)outBuff)[1];

								USB_EP0StartXfer(DIR_IN, 0, outBuffSize);
							} else {

								//USB_EPSetStall(epnum);

								ep0_state = USBD_EP0_STATUS_OUT;

								//HAL_PCD_EP_Receive
								xfer_rem = 0;
								xfer_buff[0] = 0;
								USB_EP0StartXfer(DIR_OUT, 0, ep0_maxPacket);
							}
						} else if ((ep0_state == USBD_EP0_STATUS_IN) || (ep0_state == USBD_EP0_IDLE)) {		// second time around
							//USB_EPSetStall(epnum);
						}
					} else {
						transmitting = false;
					}
				}

				if ((epint & USB_OTG_DIEPINT_TXFE) == USB_OTG_DIEPINT_TXFE) {				// 0x80 Transmit FIFO empty
					uint32_t maxPacket = (epnum == 0 ? ep0_maxPacket : ep_maxPacket);

					usbDebug[usbDebugNo].PacketSize = outBuffSize;
					usbDebug[usbDebugNo].xferBuff0 = ((uint32_t*)outBuff)[0];
					usbDebug[usbDebugNo].xferBuff1 = ((uint32_t*)outBuff)[1];

					if (outBuffSize > maxPacket) {
						xfer_rem = outBuffSize - maxPacket;
						outBuffSize = maxPacket;
					}

					USB_WritePacket(outBuff, epnum, (uint16_t)outBuffSize);

					outBuff += outBuffSize;		// Move pointer forwards
					uint32_t fifoemptymsk = (uint32_t)(0x1UL << (epnum & EP_ADDR_MSK));
					USBx_DEVICE->DIEPEMPMSK &= ~fifoemptymsk;
				}

				if ((epint & USB_OTG_DIEPINT_TOC) == USB_OTG_DIEPINT_TOC) {					// Timeout condition
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_TOC;
				}
				if ((epint & USB_OTG_DIEPINT_ITTXFE) == USB_OTG_DIEPINT_ITTXFE) {			// IN token received when Tx FIFO is empty
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_ITTXFE;
				}
				if ((epint & USB_OTG_DIEPINT_INEPNE) == USB_OTG_DIEPINT_INEPNE) {			// IN endpoint NAK effective
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_INEPNE;
				}
				if ((epint & USB_OTG_DIEPINT_EPDISD) == USB_OTG_DIEPINT_EPDISD) {			// Endpoint disabled interrupt
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_EPDISD;
				}

			}
			epnum++;
			ep_intr >>= 1U;
		}

	}


	/////////// 	800 		USBSUSP: Suspend Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_USBSUSP))
	{
		if ((USBx_DEVICE->DSTS & USB_OTG_DSTS_SUSPSTS) == USB_OTG_DSTS_SUSPSTS) {
			dev_state  = USBD_STATE_SUSPENDED;
			USBx_PCGCCTL |= USB_OTG_PCGCCTL_STOPCLK;
		}
		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_USBSUSP;
	}


	/////////// 	1000 		USBRST: Reset Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_USBRST))
	{
		USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_RWUSIG;

		// USB_FlushTxFifo
		USB_OTG_FS->GRSTCTL = (USB_OTG_GRSTCTL_TXFFLSH | (0x10 << 6));
		while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH);

		for (int i = 0; i < 6; i++) {				// hpcd->Init.dev_endpoints
			USBx_INEP(i)->DIEPINT = 0xFB7FU;		// see p1177 for explanation: based on datasheet should be more like 0b10100100111011
			USBx_INEP(i)->DIEPCTL &= ~USB_OTG_DIEPCTL_STALL;
			USBx_OUTEP(i)->DOEPINT = 0xFB7FU;
			USBx_OUTEP(i)->DOEPCTL &= ~USB_OTG_DOEPCTL_STALL;
		}
		USBx_DEVICE->DAINTMSK |= 0x10001U;

		USBx_DEVICE->DOEPMSK |= USB_OTG_DOEPMSK_STUPM | USB_OTG_DOEPMSK_XFRCM | USB_OTG_DOEPMSK_EPDM | USB_OTG_DOEPMSK_OTEPSPRM;			//  | USB_OTG_DOEPMSK_NAKM
		USBx_DEVICE->DIEPMSK |= USB_OTG_DIEPMSK_TOM | USB_OTG_DIEPMSK_XFRCM | USB_OTG_DIEPMSK_EPDM;

		// Set Default Address to 0 (will be set later when address instruction received from host)
		USBx_DEVICE->DCFG &= ~USB_OTG_DCFG_DAD;

		// setup EP0 to receive SETUP packets
		if ((USBx_OUTEP(0U)->DOEPCTL & USB_OTG_DOEPCTL_EPENA) != USB_OTG_DOEPCTL_EPENA)	{

			// Set PKTCNT to 1, XFRSIZ to 24, STUPCNT to 3 (number of back-to-back SETUP data packets endpoint can receive)
			USBx_OUTEP(0U)->DOEPTSIZ = (1U << 19) | (3U * 8U) | USB_OTG_DOEPTSIZ_STUPCNT;
		}

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_USBRST;
	}


	/////////// 	2000		ENUMDNE: Enumeration done Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_ENUMDNE))
	{
		// Set the Maximum packet size of the IN EP based on the enumeration speed
		USBx_INEP(0U)->DIEPCTL &= ~USB_OTG_DIEPCTL_MPSIZ;
		USBx_DEVICE->DCTL |= USB_OTG_DCTL_CGINAK;		//  Clear global IN NAK

		// Assuming Full Speed USB and clock > 32MHz Set USB Turnaround time
		USB_OTG_FS->GUSBCFG &= ~USB_OTG_GUSBCFG_TRDT;
		USB_OTG_FS->GUSBCFG |= (6 << 10);

		USB_ActivateEndpoint(0, DIR_OUT, 0);			// Open EP0 OUT
		USB_ActivateEndpoint(0, DIR_IN, 0);				// Open EP0 IN

		ep0_state = USBD_EP0_IDLE;

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_ENUMDNE;
	}


	///////////		40000000	SRQINT: Connection event Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_SRQINT))
	{
		//HAL_PCD_ConnectCallback(hpcd);		// this doesn't seem to do anything

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_SRQINT;
	}


	/////////// 	80000000	WKUINT: Resume Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_WKUINT))
	{
		// Clear the Remote Wake-up Signaling
		USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_RWUSIG;

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_WKUINT;
	}


	/////////// OTGINT: Handle Disconnection event Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_OTGINT))
	{
		uint32_t temp = USB_OTG_FS->GOTGINT;

		if ((temp & USB_OTG_GOTGINT_SEDET) == USB_OTG_GOTGINT_SEDET)
		{
			//HAL_PCD_DisconnectCallback(hpcd);
			//pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config);
		}
		USB_OTG_FS->GOTGINT |= temp;
	}

}



void USB::InitUSB()
{
	// USB_OTG_FS GPIO Configuration: PA8: USB_OTG_FS_SOF; PA9: USB_OTG_FS_VBUS; PA10: USB_OTG_FS_ID; PA11: USB_OTG_FS_DM; PA12: USB_OTG_FS_DP
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	// PA8 (SOF), PA10 (ID), PA11 (DM), PA12 (DP) (NB PA9 - VBUS uses default values)
	GPIOA->MODER |= GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1;					// 10: Alternate function mode
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR10 | GPIO_OSPEEDER_OSPEEDR11 | GPIO_OSPEEDER_OSPEEDR12;		// 11: High speed
	GPIOA->AFR[1] |= (10 << 12) | (10 << 16);															// Alternate Function 10 is OTG_FS

	RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;				// USB OTG FS clock enable
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;				// Enable system configuration clock: used to manage external interrupt line connection to GPIOs

	NVIC_SetPriority(OTG_FS_IRQn, 0);
	NVIC_EnableIRQ(OTG_FS_IRQn);

	USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN;			// Activate the transceiver in transmission/reception. When reset, the transceiver is kept in power-down. 0 = USB FS transceiver disabled; 1 = USB FS transceiver enabled
	USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;		// Force USB device mode
	//HAL_Delay(50U);

	// Not really sure what this is doing?
	// OTG device IN endpoint transmit FIFO size register	(OTG_DIEPTXFx) (x = 1..5[FS] /8[HS], where x is the	FIFO number)
	// Bits 31:16 INEPTXFD[15:0]: IN endpoint Tx FIFO depth
	// Bits 15:0 INEPTXSA[15:0]: IN endpoint FIFOx transmit RAM start address
	for (uint8_t i = 0U; i < 15U; i++) {
		USB_OTG_FS->DIEPTXF[i] = 0U;
	}

	USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBDEN; 			// Enable HW VBUS sensing
	USBx_DEVICE->DCFG |= USB_OTG_DCFG_DSPD;				// 11: Full speed using internal FS PHY

	USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_TXFNUM_4;	// Select buffers to flush. 10000: Flush all the transmit FIFOs in device or host mode
	USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_TXFFLSH;		// Flush the TX buffers
	while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH);

	USB_OTG_FS->GRSTCTL = USB_OTG_GRSTCTL_RXFFLSH;		// Flush the RX buffers
	while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_RXFFLSH) == USB_OTG_GRSTCTL_RXFFLSH);

	USB_OTG_FS->GINTSTS = 0xBFFFFFFFU;					// Clear pending interrupts (except SRQINT Session request/new session detected)

	// Enable interrupts
	USB_OTG_FS->GINTMSK = 0U;							// Disable all interrupts
	USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM | USB_OTG_GINTMSK_USBSUSPM |			// Receive FIFO non-empty mask; USB suspend
			USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_ENUMDNEM |							// USB reset; Enumeration done
			USB_OTG_GINTMSK_IEPINT | USB_OTG_GINTMSK_OEPINT | USB_OTG_GINTMSK_WUIM |	// IN endpoint; OUT endpoint; Resume/remote wakeup detected
			USB_OTG_GINTMSK_SRQIM | USB_OTG_GINTMSK_OTGINT;								// Session request/new session detected; OTG interrupt

	USB_OTG_FS->GRXFSIZ = 128;		 					// RxFIFO depth

	// Non-periodic transmit FIFO size register (FS_GNPTXFSIZ_Device in SFR)
	USB_OTG_FS->DIEPTXF0_HNPTXFSIZ = (64 << USB_OTG_TX0FD_Pos) |		// Endpoint 0 TxFIFO depth
			(128 << USB_OTG_TX0FSA_Pos);								// Endpoint 0 transmit RAM start  address

    // Multiply Tx_Size by 2 to get higher performance
    USB_OTG_FS->DIEPTXF[0] = (128 << USB_OTG_DIEPTXF_INEPTXFD_Pos) |	// IN endpoint TxFIFO depth
    		(192 << USB_OTG_DIEPTXF_INEPTXSA_Pos);  					// IN endpoint FIFO2 transmit RAM start address

    USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_SDIS;			// Activate USB
    USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT;		// Activate USB Interrupts
}

void USB::USB_ActivateEndpoint(uint32_t epnum, bool is_in, uint8_t eptype)
{
	uint8_t maxpktsize = (epnum == 0 ? ep0_maxPacket : ep_maxPacket);

	if (is_in) {
		USBx_DEVICE->DAINTMSK |= USB_OTG_DAINTMSK_IEPM & (uint32_t)(1UL << (epnum & EP_ADDR_MSK));

		if ((USBx_INEP(epnum)->DIEPCTL & USB_OTG_DIEPCTL_USBAEP) == 0U) {
			USBx_INEP(epnum)->DIEPCTL |= (maxpktsize & USB_OTG_DIEPCTL_MPSIZ) |
					((uint32_t)eptype << 18) | (epnum << 22) |
					USB_OTG_DIEPCTL_SD0PID_SEVNFRM |
					USB_OTG_DIEPCTL_USBAEP;
		}
	} else {
		USBx_DEVICE->DAINTMSK |= USB_OTG_DAINTMSK_OEPM & ((uint32_t)(1UL << (epnum & EP_ADDR_MSK)) << 16);

		if (((USBx_OUTEP(epnum)->DOEPCTL) & USB_OTG_DOEPCTL_USBAEP) == 0U) {
			USBx_OUTEP(epnum)->DOEPCTL |= (maxpktsize & USB_OTG_DOEPCTL_MPSIZ) |
					((uint32_t)eptype << 18) |
					USB_OTG_DIEPCTL_SD0PID_SEVNFRM |
					USB_OTG_DOEPCTL_USBAEP;
		}
	}

}

// USB_ReadPacket : read a packet from the RX FIFO
void USB::USB_ReadPacket(uint32_t *dest, uint16_t len)
{
	uint32_t *pDest = (uint32_t *)dest;
	uint32_t count32b = ((uint32_t)len + 3U) / 4U;

	for (uint32_t i = 0; i < count32b; i++)	{
		*pDest = USBx_DFIFO(0);
		pDest++;
	}
}

void USB::USB_WritePacket(uint8_t *src, uint32_t ch_ep_num, uint16_t len)
{
	uint32_t *pSrc = (uint32_t *)src;
	uint32_t count32b = ((uint32_t)len + 3U) / 4U;

	for (uint32_t i = 0; i < count32b; i++) {
		USBx_DFIFO(ch_ep_num) = *pSrc;
		pSrc++;
	}
}

// Descriptors in usbd_desc.c
void USB::USBD_GetDescriptor(usbRequest req)
{
	//uint32_t deviceserial0, deviceserial1, deviceserial2;

	switch (req.Value >> 8)	{
	case USB_DESC_TYPE_DEVICE:
		outBuff = USBD_FS_DeviceDesc;
		outBuffSize = sizeof(USBD_FS_DeviceDesc);
		break;

	case USB_DESC_TYPE_CONFIGURATION:

		outBuff = usbd_audio_CfgDesc;
		outBuffSize = sizeof(usbd_audio_CfgDesc);
		break;

	case USB_DESC_TYPE_BOS:

		outBuff = USBD_FS_BOSDesc;
		outBuffSize = sizeof(USBD_FS_BOSDesc);
		break;

	case USB_DESC_TYPE_STRING:

		switch ((uint8_t)(req.Value)) {
		case USBD_IDX_LANGID_STR:		// 300
			outBuff = USBD_LangIDDesc;
			outBuffSize = sizeof(USBD_LangIDDesc);
			break;
		case USBD_IDX_MFC_STR:			// 301
			outBuffSize = USBD_GetString((uint8_t*)USBD_MANUFACTURER_STRING, USBD_StrDesc);
			outBuff = USBD_StrDesc;
			break;
		case USBD_IDX_PRODUCT_STR:		// 302
			outBuffSize = USBD_GetString((uint8_t*)USBD_PRODUCT_STRING_FS, USBD_StrDesc);
			outBuff = USBD_StrDesc;
			break;
		case USBD_IDX_SERIAL_STR:		// 303
			{
				// STM32 unique device ID (96 bit number starting at UID_BASE)
					uint32_t deviceserial0 = *(uint32_t*) UID_BASE;
					uint32_t deviceserial1 = *(uint32_t*) UID_BASE + 4;
					uint32_t deviceserial2 = *(uint32_t*) UID_BASE + 8;
				deviceserial0 += deviceserial2;
	
				if (deviceserial0 != 0) {
					IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
					IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
				}
				outBuff = USBD_StringSerial;
				outBuffSize = sizeof(USBD_StringSerial);
			}
			break;
	    case USBD_IDX_CONFIG_STR:		// 304
			outBuffSize = USBD_GetString((uint8_t*)USBD_CONFIG_FS, USBD_StrDesc);
			outBuff = USBD_StrDesc;
	      break;

	    case USBD_IDX_INTERFACE_STR:	// 305
			outBuffSize = USBD_GetString((uint8_t*)USBD_INTERFACE_FS, USBD_StrDesc);
			outBuff = USBD_StrDesc;
			break;

		default:
			USBD_CtlError();
			return;
		}
		break;

		default:
			USBD_CtlError();
			return;
	}

	if ((outBuffSize != 0U) && (req.Length != 0U)) {
		usbDebug[usbDebugNo].PacketSize = outBuffSize;
		usbDebug[usbDebugNo].xferBuff0 = ((uint32_t*)outBuff)[0];
		usbDebug[usbDebugNo].xferBuff1 = ((uint32_t*)outBuff)[1];

		ep0_state = USBD_EP0_DATA_IN;
		outBuffSize = std::min(outBuffSize, (uint32_t)req.Length);
		USB_EP0StartXfer(DIR_IN, 0, outBuffSize);
	}

	if (req.Length == 0U) {
		USB_EP0StartXfer(DIR_IN, 0, 0);
	}
}

uint32_t USB::USBD_GetString(uint8_t *desc, uint8_t *unicode)
{
	uint32_t idx = 2;

	if (desc != NULL) {
		while (*desc != '\0') {
			unicode[idx++] = *desc++;
			unicode[idx++] = 0;
		}
		unicode[0] = idx;
		unicode[1] = USB_DESC_TYPE_STRING;
	}
	return idx;
}

void USB::IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len) {

	for (uint8_t idx = 0; idx < len; idx++) {
		if ((value >> 28) < 0xA) {
			pbuf[2 * idx] = (value >> 28) + '0';
		} else {
			pbuf[2 * idx] = (value >> 28) + 'A' - 10;
		}

		value = value << 4;

		pbuf[2 * idx + 1] = 0;
	}
}

void USB::USBD_StdDevReq(usbRequest req)
{
	uint8_t dev_addr;
	switch (req.mRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_CLASS:
	case USB_REQ_TYPE_VENDOR:
		// pdev->pClass->Setup(pdev, req);
		break;

	case USB_REQ_TYPE_STANDARD:

		switch (req.Request)
		{
		case USB_REQ_GET_DESCRIPTOR:
			USBD_GetDescriptor(req);
			break;

		case USB_REQ_SET_ADDRESS:
			dev_addr = (uint8_t)(req.Value) & 0x7FU;
			USBx_DEVICE->DCFG &= ~(USB_OTG_DCFG_DAD);
			USBx_DEVICE->DCFG |= ((uint32_t)dev_addr << 4) & USB_OTG_DCFG_DAD;
			ep0_state = USBD_EP0_STATUS_IN;
			USB_EP0StartXfer(DIR_IN, 0, 0);
			dev_state = USBD_STATE_ADDRESSED;
			break;

		case USB_REQ_SET_CONFIGURATION:
			if (dev_state == USBD_STATE_ADDRESSED) {
				dev_state = USBD_STATE_CONFIGURED;

				USB_ActivateEndpoint(req.Value, true, USBD_EP_TYPE_BULK);		// Activate in endpoint
				USB_ActivateEndpoint(req.Value, false, USBD_EP_TYPE_BULK);		// Activate out endpoint

				USB_EP0StartXfer(DIR_OUT, req.Value, 2);		// FIXME maxpacket is 2 for EP 1: CUSTOM_HID_EPIN_SIZE

				ep0_state = USBD_EP0_STATUS_IN;
				USB_EP0StartXfer(DIR_IN, 0, 0);
			}
			break;

		/*
		case USB_REQ_GET_CONFIGURATION:
			// USBD_GetConfig (pdev, req);
			break;

		case USB_REQ_GET_STATUS:
			//USBD_GetStatus (pdev, req);
			break;

		case USB_REQ_SET_FEATURE:
			//USBD_SetFeature (pdev, req);
			break;

		case USB_REQ_CLEAR_FEATURE:
			//USBD_ClrFeature (pdev, req);
			break;
*/

		default:
			USBD_CtlError();
			break;
		}
		break;

		default:
			USBD_CtlError();
			break;
	}

}

void USB::USB_EP0StartXfer(bool is_in, uint8_t epnum, uint32_t xfer_len)
{
	// IN endpoint
	if (is_in) {
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_PKTCNT);
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_XFRSIZ);

			uint32_t maxPacket = (epnum == 0 ? ep0_maxPacket : ep_maxPacket);

			// Program the transfer size and packet count as follows: xfersize = N * maxpacket + short_packet pktcnt = N + (short_packet exist ? 1 : 0)
		if (xfer_len > maxPacket && epnum == 0) {		// currently set to 0x40
				xfer_rem = xfer_len - maxPacket;
				xfer_len = maxPacket;
			}
		USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_PKTCNT & (((xfer_len + maxPacket - 1) / maxPacket) << 19));
			USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_XFRSIZ & xfer_len);

		USBx_INEP(epnum)->DIEPCTL |= (USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);	// EP enable, IN data in FIFO

		// Enable the Tx FIFO Empty Interrupt for this EP
		if (xfer_len > 0) {
			USBx_DEVICE->DIEPEMPMSK |= 1UL << (epnum & EP_ADDR_MSK);
		}
	} else { 		// OUT endpoint
		// Program the transfer size and packet count as follows: pktcnt = N xfersize = N * maxpacket
		USBx_OUTEP(epnum)->DOEPTSIZ &= ~(USB_OTG_DOEPTSIZ_XFRSIZ);
		USBx_OUTEP(epnum)->DOEPTSIZ &= ~(USB_OTG_DOEPTSIZ_PKTCNT);

		USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
		USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_XFRSIZ & xfer_len);

		USBx_OUTEP(epnum)->DOEPCTL |= (USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);		// EP enable
	}
}

void USB::USBD_CtlError() {
	USBx_INEP(0)->DIEPCTL |= USB_OTG_DIEPCTL_STALL;

/*			USBx_OUTEP(0U)->DOEPTSIZ = 0U;
	USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
	USBx_OUTEP(0U)->DOEPTSIZ |= (3U * 8U);
	USBx_OUTEP(0U)->DOEPTSIZ |=  USB_OTG_DOEPTSIZ_STUPCNT;*/

	USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_STALL;
}

/*void USB::USB_EPSetStall(uint8_t epnum) {
	if (((USBx_INEP(epnum)->DIEPCTL & USB_OTG_DIEPCTL_EPENA) == 0U) && (epnum != 0U)) {	//
		USBx_INEP(epnum)->DIEPCTL &= ~(USB_OTG_DIEPCTL_EPDIS);
	}
	USBx_INEP(epnum)->DIEPCTL |= USB_OTG_DIEPCTL_STALL;

	// FIXME - cleared in USB_EP0StartXfer?
	//USB_EP0_OutStart

	USBx_OUTEP(0U)->DOEPTSIZ = 0U;			// USB_EP0_OutStart - set STUPCNT=3; PKTCNT=1; XFRSIZ=0x18
	USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
	USBx_OUTEP(0U)->DOEPTSIZ |= (3U * 8U);
	USBx_OUTEP(0U)->DOEPTSIZ |=  USB_OTG_DOEPTSIZ_STUPCNT;

}*/

bool USB::USB_ReadInterrupts(uint32_t interrupt){

	if (((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) & interrupt) == interrupt && usbDebugNo < USB_DEBUG_COUNT) {
		usbDebugNo = usbDebugEvent % USB_DEBUG_COUNT;
		usbDebug[usbDebugNo].eventNo = usbDebugEvent;
		usbDebug[usbDebugNo].Interrupt = USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK;
		usbDebugEvent++;
	}

	return ((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) & interrupt) == interrupt;
}

void USB::SendData(const uint8_t *data, uint16_t len) {
	if (dev_state == USBD_STATE_CONFIGURED) {
		if (!transmitting) {
			transmitting = true;
			outBuff = (uint8_t*)data;
			outBuffSize = len;
			USB_EP0StartXfer(DIR_IN, 1, len);
		}
	}
}


