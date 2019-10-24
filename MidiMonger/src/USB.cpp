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
53		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,300,0,FF		USBD_IDX_LANGID_STR]
54		40000	IEPINT		USB_OTG_DIEPINT_TXFE
55		40000	IEPINT		USB_OTG_DIEPINT_XFRC
56		10					STS_DATA_UPDT
57		10					STS_SETUP_UPDT
58		80000	OEPINT 		USB_OTG_DOEPINT_XFRC
59		10					STS_SETUP_UPDT
60		10					STS_SETUP_COMP
61		80000	OEPINT 		USB_OTG_DOEPINT_STUP req 0x80,6,302,409,FF		USBD_IDX_PRODUCT_STR]
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

void USB::USBInterruptHandler() {

	int epnum, ep_intr, epint;

	if (usbEventNo >= 116) {
		int susp = 1;
	}

	// Handle spurious interrupt
	if ((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) == 0)
		return;

	/////////// 	80000 		OEPINT OUT endpoint interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_OEPINT)) {


		// Read the output endpoint interrupt register to ascertain which endpoint(s) fired an interrupt
		ep_intr = ((USBx_DEVICE->DAINT & USBx_DEVICE->DAINTMSK) & 0xFFFF0000U) >> 16; // FIXME mask unnecessary with shift right?


		// process each endpoint in turn incrementing the epnum and checking the interrupts (ep_intr) if that endpoint fired
		epnum = 0;
		while (ep_intr != 0) {
			if ((ep_intr & 1) != 0) {
				epint = USBx_OUTEP(epnum)->DOEPINT & USBx_DEVICE->DOEPMSK;

				if ((epint & USB_OTG_DOEPINT_XFRC) == USB_OTG_DOEPINT_XFRC) {		// Transfer completed
					if (usbEventNo > 14) {
						int susp = 1;
					}

					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_XFRC;				// Clear interrupt

					if (epnum == 0) {
						ep0_state = USBD_EP0_IDLE;
						USBx_OUTEP(epnum)->DOEPCTL |= USB_OTG_DOEPCTL_STALL;
					} else {
						// Add buffer contents to midiArray
						midiArray[midiEventWrite++].data = *xfer_buff;
						if (midiEventWrite >= MIDIBUFFERSIZE)	midiEventWrite = 0;

						USB_EP0StartXfer(DIR_OUT, epnum, 0);
					}
				}

				if ((epint & USB_OTG_DOEPINT_STUP) == USB_OTG_DOEPINT_STUP) {		// SETUP phase done
					// Parse Setup Request containing data in xfer_buff filled by RXFLVL interrupt
					uint8_t *pdata = (uint8_t*)xfer_buff;
					req.mRequest     = *(uint8_t *)  (pdata);
					req.Request      = *(uint8_t *)  (pdata +  1);
					req.Value        = SWAPBYTE      (pdata +  2);
					req.Index        = SWAPBYTE      (pdata +  4);
					req.Length       = SWAPBYTE      (pdata +  6);

/*					reqEvents[reqEventNo++] = usbEventNo - 1;
					reqEvents[reqEventNo++] = (uint32_t)(req.mRequest << 24) | (uint32_t)(req.Request << 16) | req.Value;
					reqEvents[reqEventNo++] = (uint32_t)(req.Index << 16) | req.Length;
					reqEventNo++;*/

					ep0_state = USBD_EP0_SETUP;

					switch (req.mRequest & 0x1F) {		// originally USBD_LL_SetupStage
					case USB_REQ_RECIPIENT_DEVICE:
						//initially USB_REQ_GET_DESCRIPTOR >> USB_DESC_TYPE_DEVICE (bmrequest is 0x6)
						USBD_StdDevReq(req);
						break;

					case USB_REQ_RECIPIENT_INTERFACE:
						if (req.mRequest == 0x21) {
							USB_EP0StartXfer(DIR_IN, 0, 0);		// sends blank request back
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
						//USBD_StdEPReq(pdev, req);
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
				if ((epint & USB_OTG_DOEPINT_NAK) == USB_OTG_DOEPINT_NAK) {			// OUT NAK interrupt
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
		while (ep_intr != 0U) {
			if ((ep_intr & 1) != 0) { // In ITR [initially true]

				epint = USBx_INEP((uint32_t)epnum)->DIEPINT & (USBx_DEVICE->DIEPMSK | (((USBx_DEVICE->DIEPEMPMSK >> (epnum & EP_ADDR_MSK)) & 0x1U) << 7));

				if ((epint & USB_OTG_DIEPINT_XFRC) == USB_OTG_DIEPINT_XFRC) {
					uint32_t fifoemptymsk = (uint32_t)(0x1UL << (epnum & EP_ADDR_MSK));
					USBx_DEVICE->DIEPEMPMSK &= ~fifoemptymsk;

					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_XFRC;

					if (epnum == 0) {

						if (ep0_state == USBD_EP0_DATA_IN) {
							if (xfer_rem > 0) {
								outBuffSize = xfer_rem;
								xfer_rem = 0;
								USB_EP0StartXfer(DIR_IN, 0, outBuffSize);
							} else {

								USB_EPSetStall(epnum);

								ep0_state = USBD_EP0_STATUS_OUT;

								//HAL_PCD_EP_Receive
								xfer_rem = 0;
								xfer_buff[0] = 0;
								USB_EP0StartXfer(DIR_OUT, 0, ep0_maxPacket);
							}
						} else if ((ep0_state == USBD_EP0_STATUS_IN) || (ep0_state == USBD_EP0_IDLE)) {		// second time around
							USB_EPSetStall(epnum);
						}
					} else {
						hid_state = CUSTOM_HID_IDLE;
					}
				}

				if ((epint & USB_OTG_DIEPINT_TXFE) == USB_OTG_DIEPINT_TXFE) {
					uint32_t maxPacket = (epnum == 0 ? ep0_maxPacket : ep_maxPacket);
					if (outBuffSize > maxPacket) {
						xfer_rem = outBuffSize - maxPacket;
						outBuffSize = maxPacket;
					}

					USB_WritePacket(outBuff, epnum, (uint16_t)outBuffSize);

					outBuff += outBuffSize;		// Move pointer forwards
					uint32_t fifoemptymsk = (uint32_t)(0x1UL << (epnum & EP_ADDR_MSK));
					USBx_DEVICE->DIEPEMPMSK &= ~fifoemptymsk;

				}

				if ((epint & USB_OTG_DIEPINT_TOC) == USB_OTG_DIEPINT_TOC) {
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_TOC;
				}
				if ((epint & USB_OTG_DIEPINT_ITTXFE) == USB_OTG_DIEPINT_ITTXFE) {
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_ITTXFE;
				}
				if ((epint & USB_OTG_DIEPINT_INEPNE) == USB_OTG_DIEPINT_INEPNE) {
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_INEPNE;
				}
				if ((epint & USB_OTG_DIEPINT_EPDISD) == USB_OTG_DIEPINT_EPDISD) {
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_EPDISD;
				}

			}
			epnum++;
			ep_intr >>= 1U;
		}

	}

	///////////		10			RXFLVL: RxQLevel Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_RXFLVL))
	{
		USB_OTG_FS->GINTMSK &= ~USB_OTG_GINTSTS_RXFLVL;

		uint32_t temp = USB_OTG_FS->GRXSTSP;		//OTG receive status debug read/OTG status read and	pop registers (OTG_GRXSTSR/OTG_GRXSTSP) not shown in SFR

		// Get the endpoint number
		epnum = temp & USB_OTG_GRXSTSP_EPNUM;

		if (((temp & USB_OTG_GRXSTSP_PKTSTS) >> 17) == STS_DATA_UPDT) {
			if ((temp & USB_OTG_GRXSTSP_BCNT) != 0)	{
				USB_ReadPacket(xfer_buff, (temp & USB_OTG_GRXSTSP_BCNT) >> 4);
			}
		} else if (((temp & USB_OTG_GRXSTSP_PKTSTS) >> 17) == STS_SETUP_UPDT) {
			USB_ReadPacket(xfer_buff, 8U);
		}

		USB_OTG_FS->GINTMSK |= USB_OTG_GINTSTS_RXFLVL;
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

		USBx_DEVICE->DOEPMSK |= USB_OTG_DOEPMSK_STUPM | USB_OTG_DOEPMSK_XFRCM | USB_OTG_DOEPMSK_EPDM | USB_OTG_DOEPMSK_OTEPSPRM | USB_OTG_DOEPMSK_NAKM;
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
	USB_OTG_FS->DIEPTXF0_HNPTXFSIZ = ((uint32_t)64 << USB_OTG_TX0FD_Pos) |		// Endpoint 0 TxFIFO depth
			((uint32_t)128 << USB_OTG_TX0FSA_Pos);								// Endpoint 0 transmit RAM start  address

    // Multiply Tx_Size by 2 to get higher performance
    USB_OTG_FS->DIEPTXF[0] = ((uint32_t)128 << USB_OTG_DIEPTXF_INEPTXFD_Pos) |	// IN endpoint TxFIFO depth
    		((uint32_t)192 << USB_OTG_DIEPTXF_INEPTXSA_Pos);  					// IN endpoint FIFO2 transmit RAM start address

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

	for (uint32_t i = 0; i < count32b; i++)
	{
		*pDest = USBx_DFIFO(0U);
		pDest++;
	}

	//return ((void *)pDest);
}

void USB::USB_WritePacket(uint8_t *src, uint32_t ch_ep_num, uint16_t len)
{
	uint32_t *pSrc = (uint32_t *)src;
	uint32_t count32b, i;

	count32b = ((uint32_t)len + 3U) / 4U;
	for (i = 0; i < count32b; i++) {
		USBx_DFIFO(ch_ep_num) = *pSrc;
		pSrc++;
	}


}

// Descriptors in usbd_desc.c
void USB::USBD_GetDescriptor(usbRequest req)
{
	uint32_t deviceserial0, deviceserial1, deviceserial2;

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
		case USBD_IDX_LANGID_STR:
			outBuff = USBD_LangIDDesc;
			outBuffSize = sizeof(USBD_LangIDDesc);
			//pbuf = pdev->pDesc->GetLangIDStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_MFC_STR:
			outBuffSize = USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc);
			outBuff = USBD_StrDesc;
			//pbuf = pdev->pDesc->GetManufacturerStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_PRODUCT_STR:
			outBuffSize = USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StrDesc);
			outBuff = USBD_StrDesc;
			//pbuf = pdev->pDesc->GetProductStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_SERIAL_STR:
			// STM32 unique device ID (96 bit number starting at UID_BASE)
			deviceserial0 = *(uint32_t *) UID_BASE;
			deviceserial1 = *(uint32_t *) UID_BASE + 4;
			deviceserial2 = *(uint32_t *) UID_BASE + 8;
			deviceserial0 += deviceserial2;

			if (deviceserial0 != 0) {
				IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
				IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
			}
			outBuff = USBD_StringSerial;
			outBuffSize = sizeof(USBD_StringSerial);
			break;
		case USBD_IDX_CONFIG_STR:
			//pbuf = pdev->pDesc->GetConfigurationStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_INTERFACE_STR:
			//pbuf = pdev->pDesc->GetInterfaceStrDescriptor(pdev->dev_speed, &len);
			break;
		default:

			USBx_INEP(0)->DIEPCTL |= USB_OTG_DIEPCTL_STALL;

			USBx_OUTEP(0U)->DOEPTSIZ = 0U;
			USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
			USBx_OUTEP(0U)->DOEPTSIZ |= (3U * 8U);
			USBx_OUTEP(0U)->DOEPTSIZ |=  USB_OTG_DOEPTSIZ_STUPCNT;

			USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_STALL;
			return;
		}
		break;
		case USB_DESC_TYPE_DEVICE_QUALIFIER:
			//USBD_CtlError(pdev , req);
			USBx_INEP(0)->DIEPCTL |= USB_OTG_DIEPCTL_STALL;

			USBx_OUTEP(0U)->DOEPTSIZ = 0U;
			USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
			USBx_OUTEP(0U)->DOEPTSIZ |= (3U * 8U);
			USBx_OUTEP(0U)->DOEPTSIZ |=  USB_OTG_DOEPTSIZ_STUPCNT;

			USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_STALL;

			return;
		case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
			//USBD_CtlError(pdev , req);
			return;
		default:
			//USBD_CtlError(pdev , req);
			return;
	}

	if ((outBuffSize != 0U) && (req.Length != 0U)) {
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
			unicode[idx++] =  0U;
		}
		unicode[0] = idx;
		unicode[1] = USB_DESC_TYPE_STRING;
	}
	return idx;
}

void USB::IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len) {

	for (uint8_t idx = 0; idx < len; idx++) {
		if (((value >> 28)) < 0xA) {
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
			//USBD_SetAddress (pdev, req)
			dev_addr = (uint8_t)(req.Value) & 0x7FU;
			USBx_DEVICE->DCFG &= ~(USB_OTG_DCFG_DAD);
			USBx_DEVICE->DCFG |= ((uint32_t)dev_addr << 4) & USB_OTG_DCFG_DAD;
			ep0_state = USBD_EP0_STATUS_IN;
			USB_EP0StartXfer(DIR_IN, 0, 0);
			dev_state = USBD_STATE_ADDRESSED;
			break;

		case USB_REQ_SET_CONFIGURATION:
			//USBD_SetConfig (pdev, req);

			if (dev_state == USBD_STATE_ADDRESSED) {
				dev_state = USBD_STATE_CONFIGURED;

				USB_ActivateEndpoint(req.Value, true, USBD_EP_TYPE_INTR);		// Activate in endpoint
				USB_ActivateEndpoint(req.Value, false, USBD_EP_TYPE_INTR);		// Activate out endpoint

				USB_EP0StartXfer(DIR_OUT, req.Value, 2);		// FIXME maxpacket is 2 for EP 1: CUSTOM_HID_EPIN_SIZE

				ep0_state = USBD_EP0_STATUS_IN;
				USB_EP0StartXfer(DIR_IN, 0, 0);
			}
			break;

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

		default:
			//USBD_CtlError(pdev, req);
			break;
		}
		break;

		default:
			//USBD_CtlError(pdev, req);
			break;
	}

}

void USB::USB_EP0StartXfer(bool is_in, uint8_t epnum, uint32_t xfer_len)
{

	// IN endpoint
	if (is_in)
	{
		// Zero Length Packet?
		if (xfer_len == 0U) {
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_PKTCNT);
			USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_PKTCNT & (1U << 19));
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_XFRSIZ);
		} else {
			uint32_t maxPacket = (epnum == 0 ? ep0_maxPacket : ep_maxPacket);
			// Program the transfer size and packet count as follows: xfersize = N * maxpacket + short_packet pktcnt = N + (short_packet exist ? 1 : 0)
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_XFRSIZ);
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_PKTCNT);

			if (xfer_len > maxPacket) {		// currently set to 0x40
				xfer_rem = xfer_len - maxPacket;
				xfer_len = maxPacket;
			}

			USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_PKTCNT & (1U << 19));
			USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_XFRSIZ & xfer_len);
		}

		/* EP enable, IN data in FIFO */
		USBx_INEP(epnum)->DIEPCTL |= (USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);

		/* Enable the Tx FIFO Empty Interrupt for this EP */
		if (xfer_len > 0U) {
			USBx_DEVICE->DIEPEMPMSK |= 1UL << (epnum & EP_ADDR_MSK);
		}
	}
	else // OUT endpoint
	{
		// Program the transfer size and packet count as follows: pktcnt = N xfersize = N * maxpacket
		USBx_OUTEP(epnum)->DOEPTSIZ &= ~(USB_OTG_DOEPTSIZ_XFRSIZ);
		USBx_OUTEP(epnum)->DOEPTSIZ &= ~(USB_OTG_DOEPTSIZ_PKTCNT);

		USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
		USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_XFRSIZ & xfer_len);

		/* EP enable */
		USBx_OUTEP(epnum)->DOEPCTL |= (USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
	}

}

void USB::USB_EPSetStall(uint8_t epnum) {
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
}

bool USB::USB_ReadInterrupts(uint32_t interrupt){

	if (((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) & interrupt) == interrupt && usbEventNo < 200) {
		usbEvents[usbEventNo] = USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK;
		usbEventNo++;
	}

	return ((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) & interrupt) == interrupt;
}

void USB::SendReport(uint8_t *report, uint16_t len) {
	if (dev_state == USBD_STATE_CONFIGURED) {
		if (hid_state == CUSTOM_HID_IDLE) {
			hid_state = CUSTOM_HID_BUSY;
			outBuff = report;
			outBuffSize = len;
			USB_EP0StartXfer(DIR_IN, 1, len);
		}
	}
}

