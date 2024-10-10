/*************�ʼ�****************
1��CubeMX ���������������ţ���Ϊ��λ�ź�Ƭѡ�ţ��������������������ã�
   GPlO output level       ����High
   GPIO mode               ����Output Open Drain
   GPIO Pull-up/Pull-down  ����No pull-up and no pull-down
   Maximum output speed    ����LOW
   User label              ����RC522_RST/RC522_CS
   ---------------------------------------------------------
   ����SPI���ܣ�ģʽѡ��-->Full-Duplex Master(ȫ˫��)�������������£�
   Basic Parameters
��     Frame format������Motorola
��     Data size   ������8 Bits
��     First bit   ������MSB First
   Clock Parameters
       Prescaler(for Baud Rate)������32
       Baud rate               ������1.125MBits/s
       Clock Polarity(CPOL)    ������LOW
       Clock Phase(CPHA)       ������1 Edge
   Advanced Parameters
       CRC Calculation   ������Disabled
       NSS Signal Type   ������Software 

2�����߷�ʽ��
   SPI_MISO��MUC�������� MISO(����)
   SPI_MOSI��MUC�������� MOSI(����)
   ��������һһ��Ӧ

3��Ӧ�ú���
    MFRC_Init();//��ʼ��
    PCD_Reset();//������λ
    PCD_Request(PICC_REQALL, RxBuffer);//����ֵΪ0������Ѱ���ɹ������ѿ����ʹ���RxBuffer��
    PCD_Anticoll(RxBuffer);   //�ѣ�ʮ�����ƣ���4���ֽڿ��Ŵ洢������RxBuffer��

***********************************/

//#define RC522_CS GPIO_Port		GPIOB
//#define RC522_CS Pin					GPIO_	PIN_12//cs nss sda��һ������
//#define RC522_RST GPIO_Port		GPIOB
//#define RC522_RST Pin					GPIO_	PIN_1
//#include "cmsis_os.h"
//#include "main.h"
#include "stm32f4xx_hal.h"
#include "rc522.h"
#include "stdio.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include "sys.h"


//��ȡ���ţ������ݻ���ɹ���ȡ�����ž��˳� ,��һ���ǻص����ܺ���
uint8_t readCard(uint8_t *readUid,void(*funCallBack)(void))
{
	uint8_t Temp[5];
	if (PCD_Request(0x52, Temp) == 0)
	{
		if (PCD_Anticoll(readUid) == 0)
		{
			if(funCallBack != NULL)
				funCallBack();
			return 0;
		}	
	}
	return 1;
}
//����ض���
int fputc(int ch,FILE *f){
 uint8_t temp[1]={ch};
 HAL_UART_Transmit(&huart1,temp,1,2);
 return ch;
}


#define osDelay HAL_Delay
//RS522_RST��RS522_NSS��������STM32407ZGT6������

#define RS522_RST(N) HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, N==1?GPIO_PIN_SET:GPIO_PIN_RESET)
#define RS522_NSS(N) HAL_GPIO_WritePin(SDA_NSS_GPIO_Port, SDA_NSS_Pin, N==1?GPIO_PIN_SET:GPIO_PIN_RESET)

extern SPI_HandleTypeDef hspi2;

/**************************************************************************************
* �������ƣ�MFRC_Init
* ����������MFRC��ʼ��
* ��ڲ�������
* ���ڲ�������
* �� �� ֵ����
* ˵    ����MFRC��SPI�ӿ�����Ϊ0~10Mbps
***************************************************************************************/
void MFRC_Init(void)
{
    RS522_NSS(1);
    RS522_RST(1);
}


/**************************************************************************************
* ��������: SPI_RW_Byte
* ��������: ģ��SPI��дһ���ֽ�
* ��ڲ���: -byte:Ҫ���͵�����
* ���ڲ���: -byte:���յ�������
***************************************************************************************/
static uint8_t ret;  //��Щ������HAL���׼�ⲻͬ�ĵط�����д������
uint8_t SPI2_RW_Byte(uint8_t byte)
{
    HAL_SPI_TransmitReceive(&hspi2, &byte, &ret, 1, 10);//��byte д�룬������һ��ֵ����������ret
    return   ret;//�����byte �ĵ�ַ����ȡʱ�õ�Ҳ��ret��ַ��һ��ֻд��һ��ֵ10
}


/**************************************************************************************
* �������ƣ�MFRC_WriteReg
* ����������дһ���Ĵ���
* ��ڲ�����-addr:��д�ļĴ�����ַ
*           -data:��д�ļĴ�������
* ���ڲ�������
* �� �� ֵ����
* ˵    ������
***************************************************************************************/
void MFRC_WriteReg(uint8_t addr, uint8_t data)
{
    uint8_t AddrByte;
    AddrByte = (addr << 1 ) & 0x7E; //�����ַ�ֽ�
    RS522_NSS(0);                   //NSS����
    SPI2_RW_Byte(AddrByte);         //д��ַ�ֽ�
    SPI2_RW_Byte(data);             //д����
    RS522_NSS(1);                   //NSS����
}


/**************************************************************************************
* �������ƣ�MFRC_ReadReg
* ������������һ���Ĵ���
* ��ڲ�����-addr:�����ļĴ�����ַ
* ���ڲ�������
* �� �� ֵ��-data:�����Ĵ���������
* ˵    ������
***************************************************************************************/
uint8_t MFRC_ReadReg(uint8_t addr)
{
    uint8_t AddrByte, data;
    AddrByte = ((addr << 1 ) & 0x7E ) | 0x80;   //�����ַ�ֽ�
    RS522_NSS(0);                               //NSS����
    SPI2_RW_Byte(AddrByte);                     //д��ַ�ֽ�
    data = SPI2_RW_Byte(0x00);                  //������
    RS522_NSS(1);                               //NSS����
    return data;
}


/**************************************************************************************
* �������ƣ�MFRC_SetBitMask
* �������������üĴ�����λ
* ��ڲ�����-addr:�����õļĴ�����ַ
*           -mask:�����üĴ�����λ(��ͬʱ���ö��bit)
* ���ڲ�������
* �� �� ֵ����
* ˵    ������
***************************************************************************************/
void MFRC_SetBitMask(uint8_t addr, uint8_t mask)
{
    uint8_t temp;
    temp = MFRC_ReadReg(addr);                  //�ȶ��ؼĴ�����ֵ
    MFRC_WriteReg(addr, temp | mask);           //�������������д��Ĵ���
}


/**************************************************************************************
* �������ƣ�MFRC_ClrBitMask
* ��������������Ĵ�����λ
* ��ڲ�����-addr:������ļĴ�����ַ
*           -mask:������Ĵ�����λ(��ͬʱ������bit)
* ���ڲ�������
* �� �� ֵ����
* ˵    ������
***************************************************************************************/
void MFRC_ClrBitMask(uint8_t addr, uint8_t mask)
{
    uint8_t temp;
    temp = MFRC_ReadReg(addr);                  //�ȶ��ؼĴ�����ֵ
    MFRC_WriteReg(addr, temp & ~mask);          //�������������д��Ĵ���
}


/**************************************************************************************
* �������ƣ�MFRC_CalulateCRC
* ������������MFRC����CRC���
* ��ڲ�����-pInData��������CRC���������
*           -len��������CRC��������ݳ���
*           -pOutData��CRC������
* ���ڲ�����-pOutData��CRC������
* �� �� ֵ����
* ˵    ������
***************************************************************************************/
void MFRC_CalulateCRC(uint8_t *pInData, uint8_t len, uint8_t *pOutData)
{
    //0xc1 1        2           pInData[2]
    uint8_t temp;
    uint32_t i;
    MFRC_ClrBitMask(MFRC_DivIrqReg, 0x04);                  //ʹ��CRC�ж�
    MFRC_WriteReg(MFRC_CommandReg, MFRC_IDLE);              //ȡ����ǰ�����ִ��
    MFRC_SetBitMask(MFRC_FIFOLevelReg, 0x80);               //���FIFO�����־λ
    for(i = 0; i < len; i++)                                //����CRC���������д��FIFO
    {
        MFRC_WriteReg(MFRC_FIFODataReg, *(pInData + i));
    }
    MFRC_WriteReg(MFRC_CommandReg, MFRC_CALCCRC);           //ִ��CRC����
    i = 100000;
    do
    {
        temp = MFRC_ReadReg(MFRC_DivIrqReg);                //��ȡDivIrqReg�Ĵ�����ֵ
        i--;
    }
    while((i != 0) && !(temp & 0x04));                      //�ȴ�CRC�������
    pOutData[0] = MFRC_ReadReg(MFRC_CRCResultRegL);         //��ȡCRC������
    pOutData[1] = MFRC_ReadReg(MFRC_CRCResultRegM);
}


/**************************************************************************************
* �������ƣ�MFRC_CmdFrame
* ����������MFRC522��ISO14443A��ͨѶ������֡����
* ��ڲ�����-cmd��MFRC522������
*           -pIndata��MFRC522���͸�MF1�������ݵĻ������׵�ַ
*           -InLenByte���������ݵ��ֽڳ���
*           -pOutdata�����ڽ���MF1��Ƭ�������ݵĻ������׵�ַ
*           -pOutLenBit��MF1���������ݵ�λ����
* ���ڲ�����-pOutdata�����ڽ���MF1��Ƭ�������ݵĻ������׵�ַ
*           -pOutLenBit������MF1����������λ���ȵ��׵�ַ
* �� �� ֵ��-status���������(MFRC_OK��MFRC_NOTAGERR��MFRC_ERR)
* ˵    ������
***************************************************************************************/
char MFRC_CmdFrame(uint8_t cmd, uint8_t *pInData, uint8_t InLenByte, uint8_t *pOutData, uint16_t *pOutLenBit)
{
    uint8_t lastBits;
    uint8_t n;
    uint32_t i;
    char status = MFRC_ERR;
    uint8_t irqEn   = 0x00;
    uint8_t waitFor = 0x00;

    /*�����������ñ�־λ*/
    switch(cmd)
    {
        case MFRC_AUTHENT:                  //Mifare��֤
            irqEn = 0x12;
            waitFor = 0x10;                 //idleIRq�жϱ�־
            break;
        case MFRC_TRANSCEIVE:               //���Ͳ���������
            irqEn = 0x77;
            waitFor = 0x30;                 //RxIRq��idleIRq�жϱ�־
            break;
    }

    /*��������֡ǰ׼��*/
    MFRC_WriteReg(MFRC_ComIEnReg, irqEn | 0x80);    //���ж�
    MFRC_ClrBitMask(MFRC_ComIrqReg, 0x80);          //����жϱ�־λSET1
    MFRC_WriteReg(MFRC_CommandReg, MFRC_IDLE);      //ȡ����ǰ�����ִ��
    MFRC_SetBitMask(MFRC_FIFOLevelReg, 0x80);       //���FIFO�����������־λ

    /*��������֡*/
    for(i = 0; i < InLenByte; i++)                  //д���������
    {
        MFRC_WriteReg(MFRC_FIFODataReg, pInData[i]);
    }
    MFRC_WriteReg(MFRC_CommandReg, cmd);            //ִ������
    if(cmd == MFRC_TRANSCEIVE)
    {
        MFRC_SetBitMask(MFRC_BitFramingReg, 0x80);  //��������
    }
    i = 300000;                                     //����ʱ��Ƶ�ʵ���,����M1�����ȴ�ʱ��25ms
    do
    {
        n = MFRC_ReadReg(MFRC_ComIrqReg);
        i--;
    }
    while((i != 0) && !(n & 0x01) && !(n & waitFor));     //�ȴ��������
    MFRC_ClrBitMask(MFRC_BitFramingReg, 0x80);          //ֹͣ����

    /*������յ�����*/
    if(i != 0)
    {
        if(!(MFRC_ReadReg(MFRC_ErrorReg) & 0x1B))
        {
            status = MFRC_OK;
            if(n & irqEn & 0x01)
            {
                status = MFRC_NOTAGERR;
            }
            if(cmd == MFRC_TRANSCEIVE)
            {
                n = MFRC_ReadReg(MFRC_FIFOLevelReg);
                lastBits = MFRC_ReadReg(MFRC_ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n * 8;
                }
                if(n == 0)
                {
                    n = 1;
                }
                if(n > MFRC_MAXRLEN)
                {
                    n = MFRC_MAXRLEN;
                }
                for(i = 0; i < n; i++)
                {
                    pOutData[i] = MFRC_ReadReg(MFRC_FIFODataReg);
                }
            }
        }
        else
        {
            status = MFRC_ERR;
        }
    }

    MFRC_SetBitMask(MFRC_ControlReg, 0x80);               //ֹͣ��ʱ������
    MFRC_WriteReg(MFRC_CommandReg, MFRC_IDLE);            //ȡ����ǰ�����ִ��

    return status;
}


/**************************************************************************************
* �������ƣ�PCD_Reset
* ����������PCD��λ
* ��ڲ�������
* ���ڲ�������
* �� �� ֵ����
* ˵    ������
***************************************************************************************/
void PCD_Reset(void)
{
    /*Ӳ��λ*/
    RS522_RST(1);//�õ���λ����
    osDelay(2);
    RS522_RST(0);
    osDelay(2);
    RS522_RST(1);
    osDelay(2);

    /*��λ*/
    MFRC_WriteReg(MFRC_CommandReg, MFRC_RESETPHASE);
    osDelay(2);

    /*��λ��ĳ�ʼ������*/
    MFRC_WriteReg(MFRC_ModeReg, 0x3D);              //CRC��ʼֵ0x6363
    MFRC_WriteReg(MFRC_TReloadRegL, 30);            //��ʱ����װֵ
    MFRC_WriteReg(MFRC_TReloadRegH, 0);
    MFRC_WriteReg(MFRC_TModeReg, 0x8D);             //��ʱ������
    MFRC_WriteReg(MFRC_TPrescalerReg, 0x3E);        //��ʱ��Ԥ��Ƶֵ
    MFRC_WriteReg(MFRC_TxAutoReg, 0x40);            //100%ASK

    PCD_AntennaOff();                               //������
    osDelay(2);
    PCD_AntennaOn();                                //������
}


/**************************************************************************************
* �������ƣ�PCD_AntennaOn
* ������������������,ʹ��PCD���������ز��ź�
* ��ڲ�������
* ���ڲ�������
* �� �� ֵ����
* ˵    ����ÿ�ο�����ر�����֮��Ӧ������1ms�ļ��
***************************************************************************************/
void PCD_AntennaOn(void)
{
    uint8_t temp;
    temp = MFRC_ReadReg(MFRC_TxControlReg);
    if (!(temp & 0x03))
    {
        MFRC_SetBitMask(MFRC_TxControlReg, 0x03);
    }
}


/**************************************************************************************
* �������ƣ�PCD_AntennaOff
* �����������ر�����,ʧ��PCD���������ز��ź�
* ��ڲ�������
* ���ڲ�������
* �� �� ֵ����
* ˵    ����ÿ�ο�����ر�����֮��Ӧ������1ms�ļ��
***************************************************************************************/
void PCD_AntennaOff(void)
{
    MFRC_ClrBitMask(MFRC_TxControlReg, 0x03);
}


/***************************************************************************************
* �������ƣ�PCD_Init
* ������������д����ʼ��
* ��ڲ�������
* ���ڲ�������
* �� �� ֵ����
* ˵    ������
***************************************************************************************/
void PCD_Init(void)
{
    MFRC_Init();      //MFRC�ܽ�����
    PCD_Reset();      //PCD��λ  ����ʼ������
    PCD_AntennaOff(); //�ر�����
    PCD_AntennaOn();   //��������
}


/***************************************************************************************
* �������ƣ�PCD_Request
* ����������Ѱ��
* ��ڲ����� -RequestMode��Ѷ����ʽ
*                             PICC_REQIDL��Ѱ��������δ��������״̬
*                 PICC_REQALL��Ѱ��������ȫ����
*               -pCardType�����ڱ��濨Ƭ����
* ���ڲ�����-pCardType����Ƭ����
*                               0x4400��Mifare_UltraLight
*                       0x0400��Mifare_One(S50)
*                       0x0200��Mifare_One(S70)
*                       0x0800��Mifare_Pro(X)
*                       0x4403��Mifare_DESFire
* �� �� ֵ��-status���������(PCD_OK��PCD_NOTAGERR��PCD_ERR)
* ˵    ������
***************************************************************************************/
char PCD_Request(uint8_t RequestMode, uint8_t *pCardType)
{
    int status;
    uint16_t unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    MFRC_ClrBitMask(MFRC_Status2Reg, 0x08);//���ڲ��¶ȴ�����
    MFRC_WriteReg(MFRC_BitFramingReg, 0x07); //�洢ģʽ������ģʽ���Ƿ��������͵�
    MFRC_SetBitMask(MFRC_TxControlReg, 0x03);//���õ����ź�13.56MHZ

    CmdFrameBuf[0] = RequestMode;

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 1, CmdFrameBuf, &unLen);

    if((status == PCD_OK) && (unLen == 0x10))
    {
        *pCardType = CmdFrameBuf[0];
        *(pCardType + 1) = CmdFrameBuf[1];
    }

    return status;
}


/***************************************************************************************
* �������ƣ�PCD_Anticoll
* ��������������ͻ,��ȡ����
* ��ڲ�����-pSnr�����ڱ��濨Ƭ���к�,4�ֽ�
* ���ڲ�����-pSnr����Ƭ���к�,4�ֽ�
* �� �� ֵ��-status���������(PCD_OK��PCD_NOTAGERR��PCD_ERR)
* ˵    ������
***************************************************************************************/
char PCD_Anticoll(uint8_t *pSnr)
{
    char status;
    uint8_t i, snr_check = 0;
    uint16_t  unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    MFRC_ClrBitMask(MFRC_Status2Reg, 0x08);
    MFRC_WriteReg(MFRC_BitFramingReg, 0x00);
    MFRC_ClrBitMask(MFRC_CollReg, 0x80);

    CmdFrameBuf[0] = PICC_ANTICOLL1;
    CmdFrameBuf[1] = 0x20;

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 2, CmdFrameBuf, &unLen);

    if(status == PCD_OK)
    {
        for(i = 0; i < 4; i++)
        {
            *(pSnr + i)  = CmdFrameBuf[i];
            snr_check ^= CmdFrameBuf[i];
        }
        if(snr_check != CmdFrameBuf[i])
        {
            status = PCD_ERR;
        }
    }

    MFRC_SetBitMask(MFRC_CollReg, 0x80);
    return status;
}


/***************************************************************************************
* �������ƣ�PCD_Select
* ����������ѡ��
* ��ڲ�����-pSnr����Ƭ���к�,4�ֽ�
* ���ڲ�������
* �� �� ֵ��-status���������(PCD_OK��PCD_NOTAGERR��PCD_ERR)
* ˵    ������
***************************************************************************************/
char PCD_Select(uint8_t *pSnr)
{
    char status;
    uint8_t i;
    uint16_t unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = PICC_ANTICOLL1;
    CmdFrameBuf[1] = 0x70;
    CmdFrameBuf[6] = 0;
    for(i = 0; i < 4; i++)
    {
        CmdFrameBuf[i + 2] = *(pSnr + i);
        CmdFrameBuf[6]  ^= *(pSnr + i);
    }
    MFRC_CalulateCRC(CmdFrameBuf, 7, &CmdFrameBuf[7]);

    MFRC_ClrBitMask(MFRC_Status2Reg, 0x08);

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 9, CmdFrameBuf, &unLen);

    if((status == PCD_OK) && (unLen == 0x18))
    {
        status = PCD_OK;
    }
    else
    {
        status = PCD_ERR;
    }
    return status;
}


/***************************************************************************************
* �������ƣ�PCD_AuthState
* ������������֤��Ƭ����
* ��ڲ�����-AuthMode����֤ģʽ
*                   PICC_AUTHENT1A����֤A����
*                   PICC_AUTHENT1B����֤B����
*           -BlockAddr�����ַ(0~63)
*           -pKey������
*           -pSnr����Ƭ���к�,4�ֽ�
* ���ڲ�������
* �� �� ֵ��-status���������(PCD_OK��PCD_NOTAGERR��PCD_ERR)
* ˵    ������֤����ʱ,������Ϊ��λ,BlockAddr����������ͬһ�������������
***************************************************************************************/
char PCD_AuthState(uint8_t AuthMode, uint8_t BlockAddr, uint8_t *pKey, uint8_t *pSnr)
{
    char status;
    uint16_t unLen;
    uint8_t i, CmdFrameBuf[MFRC_MAXRLEN];
    CmdFrameBuf[0] = AuthMode;
    CmdFrameBuf[1] = BlockAddr;
    for(i = 0; i < 6; i++)
    {
        CmdFrameBuf[i + 2] = *(pKey + i);
    }
    for(i = 0; i < 4; i++)
    {
        CmdFrameBuf[i + 8] = *(pSnr + i);
    }

    status = MFRC_CmdFrame(MFRC_AUTHENT, CmdFrameBuf, 12, CmdFrameBuf, &unLen);
    if((status != PCD_OK) || (!(MFRC_ReadReg(MFRC_Status2Reg) & 0x08)))
    {
        status = PCD_ERR;
    }

    return status;
}


/***************************************************************************************
* �������ƣ�PCD_WriteBlock
* ������������MF1�����ݿ�
* ��ڲ�����-BlockAddr�����ַ
*           -pData: ���ڱ����д�������,16�ֽ�
* ���ڲ�������
* �� �� ֵ��-status���������(PCD_OK��PCD_NOTAGERR��PCD_ERR)
* ˵    ������
***************************************************************************************/
char PCD_WriteBlock(uint8_t BlockAddr, uint8_t *pData)
{
    char status;
    uint16_t unLen;
    uint8_t i, CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = PICC_WRITE;
    CmdFrameBuf[1] = BlockAddr;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);

    if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
    {
        status = PCD_ERR;
    }

    if(status == PCD_OK)
    {
        for(i = 0; i < 16; i++)
        {
            CmdFrameBuf[i] = *(pData + i);
        }
        MFRC_CalulateCRC(CmdFrameBuf, 16, &CmdFrameBuf[16]);

        status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 18, CmdFrameBuf, &unLen);

        if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
        {
            status = PCD_ERR;
        }
    }

    return status;
}


/***************************************************************************************
* �������ƣ�PCD_ReadBlock
* ������������MF1�����ݿ�
* ��ڲ�����-BlockAddr�����ַ
*           -pData: ���ڱ������������,16�ֽ�
* ���ڲ�����-pData: ���ڱ������������,16�ֽ�
* �� �� ֵ��-status���������(PCD_OK��PCD_NOTAGERR��PCD_ERR)
* ˵    ������
***************************************************************************************/
char PCD_ReadBlock(uint8_t BlockAddr, uint8_t *pData)
{
    char status;
    uint16_t unLen;
    uint8_t i, CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = PICC_READ;
    CmdFrameBuf[1] = BlockAddr;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);
    if((status == PCD_OK) && (unLen == 0x90))
    {
        for(i = 0; i < 16; i++)
        {
            *(pData + i) = CmdFrameBuf[i];
        }
    }
    else
    {
        status = PCD_ERR;
    }

    return status;
}


/***************************************************************************************
* �������ƣ�PCD_Value
* ������������MF1�����ݿ�����ֵ����
* ��ڲ�����
*           -BlockAddr�����ַ
*           -pValue�����ֽ���ֵ��ֵ,��λ��ǰ
*           -mode����ֵ�����ģʽ
*                  PICC_INCREMENT����ֵ
*                PICC_DECREMENT����ֵ
* ���ڲ�������
* �� �� ֵ��-status���������(PCD_OK��PCD_NOTAGERR��PCD_ERR)
* ˵    ������
***************************************************************************************/
char PCD_Value(uint8_t mode, uint8_t BlockAddr, uint8_t *pValue)
{
    //0XC1        1           Increment[4]={0x03, 0x01, 0x01, 0x01};
    char status;
    uint16_t unLen;
    uint8_t i, CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = mode;
    CmdFrameBuf[1] = BlockAddr;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);

    if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
    {
        status = PCD_ERR;
    }

    if(status == PCD_OK)
    {
        for(i = 0; i < 16; i++)
        {
            CmdFrameBuf[i] = *(pValue + i);
        }
        MFRC_CalulateCRC(CmdFrameBuf, 4, &CmdFrameBuf[4]);
        unLen = 0;
        status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 6, CmdFrameBuf, &unLen);
        if(status != PCD_ERR)
        {
            status = PCD_OK;
        }
    }

    if(status == PCD_OK)
    {
        CmdFrameBuf[0] = PICC_TRANSFER;
        CmdFrameBuf[1] = BlockAddr;
        MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

        status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);

        if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
        {
            status = PCD_ERR;
        }
    }
    return status;
}


/***************************************************************************************
* �������ƣ�PCD_BakValue
* ��������������Ǯ��(��ת��)
* ��ڲ�����-sourceBlockAddr��Դ���ַ
*                -goalBlockAddr   ��Ŀ����ַ
* ���ڲ�������
* �� �� ֵ��-status���������(PCD_OK��PCD_NOTAGERR��PCD_ERR)
* ˵    ����ֻ����ͬһ��������ת��
***************************************************************************************/
char PCD_BakValue(uint8_t sourceBlockAddr, uint8_t goalBlockAddr)
{
    char status;
    uint16_t  unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = PICC_RESTORE;
    CmdFrameBuf[1] = sourceBlockAddr;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);
    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);
    if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
    {
        status = PCD_ERR;
    }

    if(status == PCD_OK)
    {
        CmdFrameBuf[0] = 0;
        CmdFrameBuf[1] = 0;
        CmdFrameBuf[2] = 0;
        CmdFrameBuf[3] = 0;
        MFRC_CalulateCRC(CmdFrameBuf, 4, &CmdFrameBuf[4]);
        status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 6, CmdFrameBuf, &unLen);
        if(status != PCD_ERR)
        {
            status = PCD_OK;
        }
    }

    if(status != PCD_OK)
    {
        return PCD_ERR;
    }

    CmdFrameBuf[0] = PICC_TRANSFER;
    CmdFrameBuf[1] = goalBlockAddr;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);
    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);
    if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
    {
        status = PCD_ERR;
    }

    return status;
}


/***************************************************************************************
* �������ƣ�PCD_Halt
* �������������Ƭ��������״̬
* ��ڲ�������
* ���ڲ�������
* �� �� ֵ��-status���������(PCD_OK��PCD_NOTAGERR��PCD_ERR)
* ˵    ������
***************************************************************************************/
char PCD_Halt(void)
{
    char status;
    uint16_t unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = PICC_HALT;
    CmdFrameBuf[1] = 0;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);

    return status;
}


///*********************************************
//��������StartIDCardTask
//��  �ܣ�����ID����ع���
//��  �Σ�
//����ֵ��
//��  ע����ID��������
//�����룺06
//**********************************************/
//void StartIDcardTask(void const * argument)
//{
//    extern uint8_t RxBuffer[32]; 
//		uint8_t TxBuffer[32];
//    char IDcard[10];
//    char __IDcard[10] = "4ba18a25";
//    char IDcard__[10] = "3957ee5c";
//    static int IDCard_Err_Num = 0;
//    char status = 1;
//    /*RS522��ʼ��*/
//    MFRC_Init();
//    PCD_Reset();
//    osDelay(350);

//    for(;;)
//    {
//        /*��ȡ����*/
//        status = PCD_Request(PICC_REQALL, RxBuffer);//����ֵΪ0������Ѱ���ɹ������ѿ����ʹ���tempbuf��

//        if(status == PCD_OK)
//        {
//            /*����ͻ,���ؿ���*/
//            PCD_Anticoll(RxBuffer);   //�ѣ�ʮ�����ƣ���4���ֽڿ��Ŵ洢������tempbuf��
//            sprintf(IDcard, "%x%x%x%x", RxBuffer[0], RxBuffer[1], RxBuffer[2], RxBuffer[3]);
//            /*****��ʼ���п����ж�******/
//            if(strcmp(__IDcard, IDcard) == 0 || strcmp(IDcard__, IDcard) == 0) //�����ж���ȷ
//            {
////                sendmsg(huart1, TxBuffer, "%d,02,06,%s,\r\n", Address, IDcard); //�ѿ��ŷ�������1�������
//							
//							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
//							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
//							HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
//                IDcard[0] = 0x00; //����ַ���
////                beep_play();//����������
////                Open_Door();//����
//            }
//            else
//            {
//                IDCard_Err_Num++;//���Ų�ƥ�䣬��־λ����
//														
//							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
//							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
//							HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
////                sendmsg(huart1, TxBuffer, "%d,01,06,01,%s,%d\r\n", Address, IDcard, IDCard_Err_Num); //���󣬿��Ų�ƥ��
////                beep_ring(1000);//����������
//                if(IDCard_Err_Num >= 5) //5�α���
//                {
////                    beep_ring(1000);//����������
////                    SIM800L_CallNum (my_phone);//����绰
////                    sendmsg(huart1, TxBuffer, "%d,02,06,01,Call phone\r\n", Address); //���󣬿��Ų�ƥ�䣬������
//                    IDCard_Err_Num = 0;
//                }

//            }
//        }
//        if(status != PCD_OK)
//        {
//            PCD_Reset();
//        }
////        osMessagePut(QueWdtHandle, 0x08, 10);
//        osDelay(300);		GPIOB->CRL &=~ (0XF00000);GPIOB->CRL |=~ (0X100000);  		GPIOB->ODR ^=(1<<5);
//    }
//}

//void read_card()
//{
//    for(;;)
//    {
//        /*��ȡ����*/
//        status = PCD_Request(PICC_REQALL, RxBuffer);//����ֵΪ0������Ѱ���ɹ������ѿ����ʹ���tempbuf��

//        if(status == PCD_OK)
//        {
//            /*����ͻ,���ؿ���*/
//            PCD_Anticoll(RxBuffer);   //�ѣ�ʮ�����ƣ���4���ֽڿ��Ŵ洢������tempbuf��
//            sprintf(IDcard, "%x%x%x%x", RxBuffer[0], RxBuffer[1], RxBuffer[2], RxBuffer[3]);
//            /*****��ʼ���п����ж�******/
//            if(strcmp(__IDcard, IDcard) == 0 || strcmp(IDcard__, IDcard) == 0) //�����ж���ȷ
//            {
////                sendmsg(huart1, TxBuffer, "%d,02,06,%s,\r\n", Address, IDcard); //�ѿ��ŷ�������1�������
////							HAL_UART_Transmit(&huart1,RxBuffer,8, 100);
//							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
//							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
//							HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
//                IDcard[0] = 0x00; //����ַ���
////                beep_play();//����������
////                Open_Door();//����
//            }
//            else
//            {
//                IDCard_Err_Num++;//���Ų�ƥ�䣬��־λ����
//							HAL_UART_Transmit(&huart1,RxBuffer,8, 100);							
//							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
//							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
//							HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
////                sendmsg(huart1, TxBuffer, "%d,01,06,01,%s,%d\r\n", Address, IDcard, IDCard_Err_Num); //���󣬿��Ų�ƥ��
////                beep_ring(1000);//����������
//                if(IDCard_Err_Num >= 5) //5�α���
//                {
////                    beep_ring(1000);//����������
////                    SIM800L_CallNum (my_phone);//����绰
////                    sendmsg(huart1, TxBuffer, "%d,02,06,01,Call phone\r\n", Address); //���󣬿��Ų�ƥ�䣬������
//                    IDCard_Err_Num = 0;
//                }

//            }
//        }
//        if(status != PCD_OK)
//        {
//            PCD_Reset();
//        }
//			}
//}
