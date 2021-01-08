/******************************************************************************
* \attention
*
* <h2><center>&copy; COPYRIGHT 2019 STMicroelectronics</center></h2>
*
* Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
* You may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
*        www.st.com/myliberty
*
* Unless required by applicable law or agreed to in writing, software 
* distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
* AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************************/

/*! \file
*
*  \author 
*
*  \brief Demo application
*
*  This demo shows how to poll for several types of NFC cards/devices and how 
*  to exchange data with these devices, using the RFAL library.
*
*  This demo does not fully implement the activities according to the standards,
*  it performs the required to communicate with a card/device and retrieve 
*  its UID. Also blocking methods are used for data exchange which may lead to
*  long periods of blocking CPU/MCU.
*  For standard compliant example please refer to the Examples provided
*  with the RFAL library.
* 
*/

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "demo.h"
#include "utils.h"
#include "rfal_nfc.h"
#include "usart.h"
#include "main.h"
#include "st25r95_com.h"
#include "oled.h"
#if (defined(ST25R3916) || defined(ST25R95)) && RFAL_FEATURE_LISTEN_MODE
#include "demo_ce.h"
#endif

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/* Definition of possible states the demo state machine could have */
#define DEMO_ST_NOTINIT 0         /*!< Demo State:  Not initialized        */
#define DEMO_ST_START_DISCOVERY 1 /*!< Demo State:  Start Discovery        */
#define DEMO_ST_DISCOVERY 2       /*!< Demo State:  Discovery              */

#define DEMO_NFCV_BLOCK_LEN 4 /*!< NFCV Block len                      */

#define DEMO_NFCV_USE_SELECT_MODE false /*!< NFCV demonstrate select mode        */
#define DEMO_NFCV_WRITE_TAG false       /*!< NFCV demonstrate Write Single Block */

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/

/* P2P communication data */
static uint8_t NFCID3[] = {0x01, 0xFE, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
static uint8_t GB[] = {0x46, 0x66, 0x6d, 0x01, 0x01, 0x11, 0x02, 0x02, 0x07, 0x80, 0x03, 0x02, 0x00, 0x03, 0x04, 0x01, 0x32, 0x07, 0x01, 0x03};

/* APDUs communication data */
static uint8_t ndefSelectApp[] = {0x00, 0xA4, 0x04, 0x00, 0x07, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 0x00};
static uint8_t ccSelectFile[] = {0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x03};
static uint8_t readBynary[] = {0x00, 0xB0, 0x00, 0x00, 0x0F};
/*static uint8_t ppseSelectApp[] = { 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00 };*/

/* P2P communication data */
static uint8_t ndefLLCPSYMM[] = {0x00, 0x00};
//static uint8_t ndefInit[] = {0x05, 0x20, 0x06, 0x0F, 0x75, 0x72, 0x6E, 0x3A, 0x6E, 0x66, 0x63, 0x3A, 0x73, 0x6E, 0x3A, 0x73, 0x6E, 0x65, 0x70, 0x02, 0x02, 0x07, 0x80, 0x05, 0x01, 0x02};
static uint8_t ndefInit[50];
static uint8_t ndefUriSTcom[] = {0x13, 0x20, 0x00, 0x10, 0x02, 0x00, 0x00, 0x00, 0x19, 0xc1, 0x01, 0x00, 0x00, 0x00, 0x12, 0x55, 0x00, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x73, 0x74, 0x2e, 0x63, 0x6f, 0x6d};

#if (defined(ST25R3916) || defined(ST25R95)) && RFAL_FEATURE_LISTEN_MODE
/* NFC-A CE config */
static uint8_t ceNFCA_NFCID[] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66}; /* NFCID / UID (7 bytes)                    */
static uint8_t ceNFCA_SENS_RES[] = {0x44, 0x00};                            /* SENS_RES / ATQA                          */
static uint8_t ceNFCA_SEL_RES = 0x20;
#endif /* RFAL_FEATURE_LISTEN_MODE */ /* SEL_RES / SAK                            */
#if defined(ST25R3916) && RFAL_FEATURE_LISTEN_MODE
/* NFC-F CE config */
static uint8_t ceNFCF_nfcid2[] = {0x02, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
static uint8_t ceNFCF_SC[] = {0x12, 0xFC};
static uint8_t ceNFCF_SENSF_RES[] = {0x01,                                           /* SENSF_RES                                */
                                     0x02, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, /* NFCID2                                   */
                                     0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x00, /* PAD0, PAD01, MRTIcheck, MRTIupdate, PAD2 */
                                     0x00, 0x00};                                    /* RD                                       */
#endif                                                                               /* RFAL_FEATURE_LISTEN_MODE */

/*
															******************************************************************************
															* LOCAL VARIABLES
															******************************************************************************
*/
static rfalNfcDiscoverParam discParam;
static uint8_t state = DEMO_ST_NOTINIT;
static uint8_t *NFC_SendMAC_Addr = RT_NULL;

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

static void demoP2P(rfalNfcDevice *nfcDev);
static void demoAPDU(void);
static void demoNfcv(rfalNfcvListenDevice *nfcvDev);
static void demoNfcf(rfalNfcfListenDevice *nfcfDev);
static void demoCE(rfalNfcDevice *nfcDev);
static void demoNotif(rfalNfcState st);
ReturnCode demoTransceiveBlocking(uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxBuf, uint16_t **rcvLen, uint32_t fwt);

extern rt_mailbox_t NFC_SendMAC_mb;
extern rt_event_t AbortWavplay_Event;
extern rt_event_t PlayWavplay_Event;
extern rt_mailbox_t LOW_PWR_mb;
extern void LOWPWR_Config(void);
extern char Last_Audio_Name[50];
/*!
*****************************************************************************
* \brief Demo Notification
*
*  This function receives the event notifications from RFAL
*****************************************************************************
*/
static void demoNotif(rfalNfcState st)
{
    uint8_t devCnt;
    rfalNfcDevice *dev;

    if (st == RFAL_NFC_STATE_WAKEUP_MODE)
    {
        platformLog("Wake Up mode started \r\n");
    }
    else if (st == RFAL_NFC_STATE_POLL_TECHDETECT)
    {
        platformLog("Wake Up mode terminated. Polling for devices \r\n");
    }
    else if (st == RFAL_NFC_STATE_POLL_SELECT)
    {
        /* Multiple devices were found, activate first of them */
        rfalNfcGetDevicesFound(&dev, &devCnt);
        rfalNfcSelect(0);

        platformLog("Multiple Tags detected: %d \r\n", devCnt);
    }
}

/*!
*****************************************************************************
* \brief Demo Ini
*
*  This function Initializes the required layers for the demo
*
* \return true  : Initialization ok
* \return false : Initialization failed
*****************************************************************************
*/
bool demoIni(void)
{
    ReturnCode err;

    err = rfalNfcInitialize();
    if (err == ERR_NONE)
    {
        discParam.compMode = RFAL_COMPLIANCE_MODE_NFC;
        discParam.devLimit = 1U;
        discParam.nfcfBR = RFAL_BR_212;
        discParam.ap2pBR = RFAL_BR_424;
        discParam.maxBR = RFAL_BR_KEEP;

        ST_MEMCPY(&discParam.nfcid3, NFCID3, sizeof(NFCID3));
        ST_MEMCPY(&discParam.GB, GB, sizeof(GB));
        discParam.GBLen = sizeof(GB);

        discParam.notifyCb = demoNotif;
        discParam.totalDuration = 1000U;
        discParam.wakeupEnabled = false;
        discParam.wakeupConfigDefault = true;
        discParam.techs2Find = (RFAL_NFC_POLL_TECH_A | RFAL_NFC_POLL_TECH_B | RFAL_NFC_POLL_TECH_F | RFAL_NFC_POLL_TECH_V | RFAL_NFC_POLL_TECH_ST25TB);

#if defined(ST25R3911) || defined(ST25R3916)
        discParam.techs2Find |= (RFAL_NFC_POLL_TECH_AP2P | RFAL_NFC_LISTEN_TECH_AP2P);
#endif /* ST25R3911 || ST25R3916 */

#if RFAL_FEATURE_LISTEN_MODE
#if defined(ST25R3916) || defined(ST25R95)

        /* Set configuration for NFC-A CE */
        ST_MEMCPY(discParam.lmConfigPA.SENS_RES, ceNFCA_SENS_RES, RFAL_LM_SENS_RES_LEN); /* Set SENS_RES / ATQA */
        ST_MEMCPY(discParam.lmConfigPA.nfcid, ceNFCA_NFCID, RFAL_NFCID1_DOUBLE_LEN);     /* Set NFCID / UID */
        discParam.lmConfigPA.nfcidLen = RFAL_LM_NFCID_LEN_07;                            /* Set NFCID length to 7 bytes */
        discParam.lmConfigPA.SEL_RES = ceNFCA_SEL_RES;                                   /* Set SEL_RES / SAK */
        discParam.techs2Find |= RFAL_NFC_LISTEN_TECH_A;
#endif /* ST25R3916 ||  ST25R95*/
#if defined(ST25R3916)
        /* Set configuration for NFC-F CE */
        ST_MEMCPY(discParam.lmConfigPF.SC, ceNFCF_SC, RFAL_LM_SENSF_SC_LEN);                /* Set System Code */
        ST_MEMCPY(&ceNFCF_SENSF_RES[RFAL_NFCF_CMD_LEN], ceNFCF_nfcid2, RFAL_NFCID2_LEN);    /* Load NFCID2 on SENSF_RES */
        ST_MEMCPY(discParam.lmConfigPF.SENSF_RES, ceNFCF_SENSF_RES, RFAL_LM_SENSF_RES_LEN); /* Set SENSF_RES / Poll Response */

        discParam.techs2Find |= RFAL_NFC_LISTEN_TECH_F;

#endif /* ST25R3916 */
#endif /* RFAL_FEATURE_LISTEN_MODE */
        state = DEMO_ST_START_DISCOVERY;
        return true;
    }
    return false;
}

/*!
*****************************************************************************
* \brief Demo Cycle
*
*  This function executes the demo state machine. 
*  It must be called periodically
*****************************************************************************
*/
rfalNfcDevice *nfcDevice;
void demoCycle(void)
{
    uint8_t Flag = 0;
    rt_uint32_t Play_rev = 0;
    static uint8_t Count_Num = 0;

    rfalNfcWorker(); /* Run RFAL worker periodically */
    switch (state)
    {
    case DEMO_ST_START_DISCOVERY:

        platformLedOff(PLATFORM_LED_AP2P_PORT, PLATFORM_LED_AP2P_PIN);
        platformLedOff(PLATFORM_LED_FIELD_PORT, PLATFORM_LED_FIELD_PIN);
        rfalNfcDeactivate(false);
        rfalNfcDiscover(&discParam);
        state = DEMO_ST_DISCOVERY;
        break;
        /*******************************************************************************/
    case DEMO_ST_DISCOVERY:

        if (rfalNfcIsDevActivated(rfalNfcGetState()))
        {
            rfalNfcGetActiveDevice(&nfcDevice);

            switch (nfcDevice->type)
            {
            case RFAL_NFC_LISTEN_TYPE_NFCA:
                switch (nfcDevice->dev.nfca.type)
                {
                default:
                    //platformLog("ISO14443A/NFC-A card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                    rt_mb_send(LOW_PWR_mb, NULL);
                    ConfigManager_TagHunting(TRACK_ALL);
                    rt_event_send(AbortWavplay_Event, 2);
                    rt_event_send(PlayWavplay_Event, 1);
                    Count_Num = 0; //发送成功，则停止检测
                    break;
                }
                break;
            case RFAL_NFC_LISTEN_TYPE_AP2P:
            case RFAL_NFC_POLL_TYPE_AP2P:

                platformLog("NFC Active P2P device found. NFCID3: %s\r\n", hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                platformLedOn(PLATFORM_LED_AP2P_PORT, PLATFORM_LED_AP2P_PIN);
                demoP2P(nfcDevice);
                break;
            default:
                break;
            }
            rfalNfcDeactivate(false);
#if !defined(DEMO_NO_DELAY_IN_DEMOCYCLE)
#endif
            state = DEMO_ST_START_DISCOVERY;
        }
        else
        {
            //如果连续10次都未检测到，就判定为没检测到
            Count_Num++;
            if (Count_Num == 10)
            {
                /*检测不到NFC标签时停止播放*/
                rt_event_send(AbortWavplay_Event, 1);
                Last_Audio_Name[0] = '$'; //整体播放完成，改变保存的信息，以便相同位置得以发送
                Count_Num = 0;
            }
        }
        break;
    default:
        break;
    }
}

static void demoCE(rfalNfcDevice *nfcDev)
{
#if (defined(ST25R3916) || defined(ST25R95)) && RFAL_FEATURE_LISTEN_MODE

    ReturnCode err;
    uint8_t *rxData;
    uint16_t *rcvLen;
    uint8_t txBuf[100];
    uint16_t txLen;

#if defined(ST25R3916)
    demoCeInit(ceNFCF_nfcid2);
#endif /* ST25R3916 */
    do
    {
        rfalNfcWorker();

        switch (rfalNfcGetState())
        {
        case RFAL_NFC_STATE_ACTIVATED:
            err = demoTransceiveBlocking(NULL, 0, &rxData, &rcvLen, 0);
            break;

        case RFAL_NFC_STATE_DATAEXCHANGE:
        case RFAL_NFC_STATE_DATAEXCHANGE_DONE:

            txLen = ((nfcDev->type == RFAL_NFC_POLL_TYPE_NFCA) ? demoCeT4T(rxData, *rcvLen, txBuf, sizeof(txBuf)) : demoCeT3T(rxData, *rcvLen, txBuf, sizeof(txBuf)));
            err = demoTransceiveBlocking(txBuf, txLen, &rxData, &rcvLen, RFAL_FWT_NONE);
            break;

        case RFAL_NFC_STATE_LISTEN_SLEEP:
        default:
            break;
        }
    } while ((err == ERR_NONE) || (err == ERR_SLEEP_REQ));

#endif /* RFAL_FEATURE_LISTEN_MODE */
}

/*!
*****************************************************************************
* \brief Demo NFC-F 
*
* Example how to exchange read and write blocks on a NFC-F tag
* 
*****************************************************************************
*/
static void demoNfcf(rfalNfcfListenDevice *nfcfDev)
{
    ReturnCode err;
    uint8_t buf[(RFAL_NFCF_NFCID2_LEN + RFAL_NFCF_CMD_LEN + (3 * RFAL_NFCF_BLOCK_LEN))];
    uint16_t rcvLen;
    rfalNfcfServ srv = RFAL_NFCF_SERVICECODE_RDWR;
    rfalNfcfBlockListElem bl[3];
    rfalNfcfServBlockListParam servBlock;
    // uint8_t                   wrData[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

    servBlock.numServ = 1;     /* Only one Service to be used           */
    servBlock.servList = &srv; /* Service Code: NDEF is Read/Writeable  */
    servBlock.numBlock = 1;    /* Only one block to be used             */
    servBlock.blockList = bl;
    bl[0].conf = RFAL_NFCF_BLOCKLISTELEM_LEN; /* Two-byte Block List Element           */
    bl[0].blockNum = 0x0001;                  /* Block: NDEF Data                      */

    err = rfalNfcfPollerCheck(nfcfDev->sensfRes.NFCID2, &servBlock, buf, sizeof(buf), &rcvLen);
    platformLog(" Check Block: %s Data:  %s \r\n", (err != ERR_NONE) ? "FAIL" : "OK", (err != ERR_NONE) ? "" : hex2Str(&buf[1], RFAL_NFCF_BLOCK_LEN));

#if 0 /* Writing example */
	err = rfalNfcfPollerUpdate( nfcfDev->sensfRes.NFCID2, &servBlock, buf , sizeof(buf), wrData, buf, sizeof(buf) );
	platformLog(" Update Block: %s Data: %s \r\n", (err != ERR_NONE) ? "FAIL": "OK", (err != ERR_NONE) ? "" : hex2Str((uint8_t *) wrData, RFAL_NFCF_BLOCK_LEN) );
	err = rfalNfcfPollerCheck( nfcfDev->sensfRes.NFCID2, &servBlock, buf, sizeof(buf), &rcvLen);
	platformLog(" Check Block:  %s Data: %s \r\n", (err != ERR_NONE) ? "FAIL": "OK", (err != ERR_NONE) ? "" : hex2Str( &buf[1], RFAL_NFCF_BLOCK_LEN) );
#endif
}

/*!
*****************************************************************************
* \brief Demo NFC-V Exchange
*
* Example how to exchange read and write blocks on a NFC-V tag
* 
*****************************************************************************
*/
static void demoNfcv(rfalNfcvListenDevice *nfcvDev)
{
    ReturnCode err;
    uint16_t rcvLen;
    uint8_t blockNum = 1;
    uint8_t rxBuf[1 + DEMO_NFCV_BLOCK_LEN + RFAL_CRC_LEN]; /* Flags + Block Data + CRC */
    uint8_t *uid;
#if DEMO_NFCV_WRITE_TAG
    uint8_t wrData[DEMO_NFCV_BLOCK_LEN] = {0x11, 0x22, 0x33, 0x99}; /* Write block example */
#endif

    uid = nfcvDev->InvRes.UID;

#if DEMO_NFCV_USE_SELECT_MODE
    /*
	* Activate selected state
	*/
    err = rfalNfcvPollerSelect(RFAL_NFCV_REQ_FLAG_DEFAULT, nfcvDev->InvRes.UID);
    platformLog(" Select %s \r\n", (err != ERR_NONE) ? "FAIL (revert to addressed mode)" : "OK");
    if (err == ERR_NONE)
    {
        uid = NULL;
    }
#endif

    /*
    * Read block using Read Single Block command
    * with addressed mode (uid != NULL) or selected mode (uid == NULL)
    */
    err = rfalNfcvPollerReadSingleBlock(RFAL_NFCV_REQ_FLAG_DEFAULT, uid, blockNum, rxBuf, sizeof(rxBuf), &rcvLen);
    platformLog(" Read Block: %s %s\r\n", (err != ERR_NONE) ? "FAIL" : "OK Data:", (err != ERR_NONE) ? "" : hex2Str(&rxBuf[1], DEMO_NFCV_BLOCK_LEN));

#if DEMO_NFCV_WRITE_TAG /* Writing example */
    err = rfalNfcvPollerWriteSingleBlock(RFAL_NFCV_REQ_FLAG_DEFAULT, uid, blockNum, wrData, sizeof(wrData));
    platformLog(" Write Block: %s Data: %s\r\n", (err != ERR_NONE) ? "FAIL" : "OK", hex2Str(wrData, DEMO_NFCV_BLOCK_LEN));
    err = rfalNfcvPollerReadSingleBlock(RFAL_NFCV_REQ_FLAG_DEFAULT, uid, blockNum, rxBuf, sizeof(rxBuf), &rcvLen);
    platformLog(" Read Block: %s %s\r\n", (err != ERR_NONE) ? "FAIL" : "OK Data:", (err != ERR_NONE) ? "" : hex2Str(&rxBuf[1], DEMO_NFCV_BLOCK_LEN));
#endif
}

/*!
*****************************************************************************
* \brief Demo P2P Exchange
*
* Sends a NDEF URI record 'http://www.ST.com' via NFC-DEP (P2P) protocol.
* 
* This method sends a set of static predefined frames which tries to establish
* a LLCP connection, followed by the NDEF record, and then keeps sending 
* LLCP SYMM packets to maintain the connection.
* 
* 
*****************************************************************************
*/
void demoP2P(rfalNfcDevice *nfcDev)
{
    uint16_t *rxLen;
    uint8_t *rxData;
    ReturnCode err;

    /* In Listen mode retieve the first request from Initiator */
    if (nfcDev->type == RFAL_NFC_POLL_TYPE_AP2P)
    {
        demoTransceiveBlocking(NULL, 0, &rxData, &rxLen, 0);

        /* Initiator request is being ignored/discarded  */
    }

    platformLog(" Initialize device .. ");
    if (rt_mb_recv(NFC_SendMAC_mb, (rt_uint32_t *)&NFC_SendMAC_Addr, RT_WAITING_NO) == RT_EOK)
    {
        strcpy((char *)ndefInit, (char *)NFC_SendMAC_Addr);
    }
    err = demoTransceiveBlocking(ndefInit, sizeof(ndefInit), &rxData, &rxLen, RFAL_FWT_NONE);

    if (err != ERR_NONE)
    {
        platformLog("failed.");
        return;
    }
    platformLog("succeeded.\r\n");

    err = demoTransceiveBlocking(ndefUriSTcom, sizeof(ndefUriSTcom), &rxData, &rxLen, RFAL_FWT_NONE);
    if (err != ERR_NONE)
    {
        platformLog("failed.");
        return;
    }
    platformLog("succeeded.\r\n");

    platformLog(" Device present, maintaining connection ");
    while (err == ERR_NONE)
    {
        err = demoTransceiveBlocking(ndefLLCPSYMM, sizeof(ndefLLCPSYMM), &rxData, &rxLen, RFAL_FWT_NONE);
        platformLog(".");
        platformDelay(50);
    }
    platformLog("\r\n Device removed.\r\n");
}

/*!
*****************************************************************************
* \brief Demo APDUs Exchange
*
* Example how to exchange a set of predefined APDUs with PICC. The NDEF
* application will be selected and then CC will be selected and read.
* 
*****************************************************************************
*/
void demoAPDU(void)
{
    ReturnCode err;
    uint16_t *rxLen;
    uint8_t *rxData;

    /* Exchange APDU: NDEF Tag Application Select command */
    err = demoTransceiveBlocking(ndefSelectApp, sizeof(ndefSelectApp), &rxData, &rxLen, RFAL_FWT_NONE);
    platformLog(" Select NDEF Application: %s Data: %s\r\n", (err != ERR_NONE) ? "FAIL" : "OK", hex2Str(rxData, *rxLen));

    if ((err == ERR_NONE) && rxData[0] == 0x90 && rxData[1] == 0x00)
    {
        /* Exchange APDU: Select Capability Container File */
        err = demoTransceiveBlocking(ccSelectFile, sizeof(ccSelectFile), &rxData, &rxLen, RFAL_FWT_NONE);
        platformLog(" Select CC: %s Data: %s\r\n", (err != ERR_NONE) ? "FAIL" : "OK", hex2Str(rxData, *rxLen));

        /* Exchange APDU: Read Capability Container File  */
        err = demoTransceiveBlocking(readBynary, sizeof(readBynary), &rxData, &rxLen, RFAL_FWT_NONE);
        platformLog(" Read CC: %s Data: %s\r\n", (err != ERR_NONE) ? "FAIL" : "OK", hex2Str(rxData, *rxLen));
    }
}

/*!
*****************************************************************************
* \brief Demo Blocking Transceive 
*
* Helper function to send data in a blocking manner via the rfalNfc module 
*  
* \warning A protocol transceive handles long timeouts (several seconds), 
* transmission errors and retransmissions which may lead to a long period of 
* time where the MCU/CPU is blocked in this method.
* This is a demo implementation, for a non-blocking usage example please 
* refer to the Examples available with RFAL
*
* \param[in]  txBuf      : data to be transmitted
* \param[in]  txBufSize  : size of the data to be transmited
* \param[out] rxData     : location where the received data has been placed
* \param[out] rcvLen     : number of data bytes received
* \param[in]  fwt        : FWT to be used (only for RF frame interface, 
*                                          otherwise use RFAL_FWT_NONE)
*
* 
*  \return ERR_PARAM     : Invalid parameters
*  \return ERR_TIMEOUT   : Timeout error
*  \return ERR_FRAMING   : Framing error detected
*  \return ERR_PROTO     : Protocol error detected
*  \return ERR_NONE      : No error, activation successful
* 
*****************************************************************************
*/
ReturnCode demoTransceiveBlocking(uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxData, uint16_t **rcvLen, uint32_t fwt)
{
    ReturnCode err;

    err = rfalNfcDataExchangeStart(txBuf, txBufSize, rxData, rcvLen, fwt);
    if (err == ERR_NONE)
    {
        do
        {
            rfalNfcWorker();
            err = rfalNfcDataExchangeGetStatus();
        } while (err == ERR_BUSY);
    }
    return err;
}

static void Write_NFC(rfalNfcfListenDevice *nfcfDev)
{
    ReturnCode err;
    uint8_t buf[(RFAL_NFCF_NFCID2_LEN + RFAL_NFCF_CMD_LEN + (3 * RFAL_NFCF_BLOCK_LEN))];
    uint16_t rcvLen;
    rfalNfcfServ srv = RFAL_NFCF_SERVICECODE_RDWR;
    rfalNfcfBlockListElem bl[3];
    rfalNfcfServBlockListParam servBlock;
    uint8_t wrData[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

    servBlock.numServ = 1;     /* Only one Service to be used           */
    servBlock.servList = &srv; /* Service Code: NDEF is Read/Writeable  */
    servBlock.numBlock = 1;    /* Only one block to be used             */
    servBlock.blockList = bl;
    bl[0].conf = RFAL_NFCF_BLOCKLISTELEM_LEN; /* Two-byte Block List Element           */
    bl[0].blockNum = 0x0002;                  /* Block: NDEF Data                      */

    err = rfalNfcfPollerCheck(nfcfDev->sensfRes.NFCID2, &servBlock, buf, sizeof(buf), &rcvLen);
    platformLog(" Check Block: %s Data:  %s \r\n", (err != ERR_NONE) ? "FAIL" : "OK", (err != ERR_NONE) ? "" : hex2Str(&buf[1], RFAL_NFCF_BLOCK_LEN));

#if 1 /* Writing example */
    err = rfalNfcfPollerUpdate(nfcfDev->sensfRes.NFCID2, &servBlock, buf, sizeof(buf), wrData, buf, sizeof(buf));
    platformLog(" Update Block: %s Data: %s \r\n", (err != ERR_NONE) ? "FAIL" : "OK", (err != ERR_NONE) ? "" : hex2Str((uint8_t *)wrData, RFAL_NFCF_BLOCK_LEN));
    err = rfalNfcfPollerCheck(nfcfDev->sensfRes.NFCID2, &servBlock, buf, sizeof(buf), &rcvLen);
    platformLog(" Check Block:  %s Data: %s \r\n", (err != ERR_NONE) ? "FAIL" : "OK", (err != ERR_NONE) ? "" : hex2Str(&buf[1], RFAL_NFCF_BLOCK_LEN));
#endif
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
