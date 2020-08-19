#ifndef __i2c_H
#define __i2c_H


/*-------------------------IMPORTANT-------------------------*/
// Import the correct STM32 CMSIS header for your device
// Call the I2Cx_EV_Handler from the function that overwrites/implements the IVT entry for I2Cx interrupt events
/* The following callbacks can be overwritten to implement functionality dependent on transmission completion:
 * I2C_WriteCpltCallBack
 * I2C_ReadCpltCallBack
 * I2C_MemTxCpltCallBack
 * I2C_MemRxCpltCallBack
 * I2C_NackReceivedCallBack
 * /


/** @defgroup I2C_reloadEndMode_definition
  * @{
  */
#define  I2C_Reload_Mode               I2C_CR2_RELOAD
#define  I2C_AutoEnd_Mode              I2C_CR2_AUTOEND
#define  I2C_SoftEnd_Mode              ((uint32_t)0x00000000)

/** @defgroup I2C_startStopMode_definition
  * @{
  */
#define  I2C_No_StartStop              ((uint32_t)0x00000000)
#define  I2C_Generate_Stop             I2C_CR2_STOP
#define  I2C_Generate_Start_Read       (uint32_t)(I2C_CR2_START | I2C_CR2_RD_WRN)
#define  I2C_Generate_Start_Write      I2C_CR2_START


typedef enum {
    I2C_ERRROR_NONE   = 0x00,
    I2C_ERROR_BUSY    = 0x01,
    I2C_ERROR_TIMEOUT = 0x02,
    I2C_ERROR_NACK    = 0x03
} I2C_ErrorTypeDef;

typedef enum {
    I2C_NONE      = 0x00,
    I2C_WRITE     = 0x01,
    I2C_READ      = 0x02,
    I2C_WRITE_IT  = 0x03,
    I2C_READ_IT   = 0x04,
    I2C_MEM_WRITE = 0x05,
    I2C_MEM_READ  = 0x06
} I2C_OperationTypeDef;

typedef enum {
    I2C_READY              = 0x00,
    I2C_BUSY_TX_SUBADDRESS = 0x01,
    I2C_BUSY_TX            = 0x02,
    I2C_BUSY_RX            = 0x03,
} I2C_StateTypeDef;

typedef enum {
     I2C_WriteCplt    = 0x00,
     I2C_ReadCplt     = 0x01,
     I2C_MemTxCplt    = 0x02,
     I2C_MemRxCplt    = 0x03,
     I2C_NackReceived = 0x04,
} I2C_CallBackTypeDef;

typedef I2C_CallBackHandleStruct I2C_CallBackHandleTypeDef;
typedef I2C_HandleStruct I2C_HandleTypeDef;

struct I2C_CallBackHandleStruct {
    void (*I2C_WriteCpltCallBack)(I2C_HandleTypeDef *handle);
    void (*I2C_ReadCpltCallBack)(I2C_HandleTypeDef *handle);
    void (*I2C_MemTxCpltCallBack)(I2C_HandleTypeDef *handle);
    void (*I2C_MemRxCpltCallBack)(I2C_HandleTypeDef *handle);
    void (*I2C_NackReceivedCallBack)(I2C_HandleTypeDef *handle);
};

struct I2C_HandleStruct {
    I2C_TypeDef *instance;
    volatile I2C_StateTypeDef state;
    volatile I2C_StateTypeDef previousState;
    volatile I2C_ErrorTypeDef error;
    volatile I2C_OperationTypeDef operation;
    uint16_t memAddress;
    uint8_t memSize;
    uint8_t *dataBuffer;
    uint16_t dataSize;
    uint8_t devAddress;
    uint16_t dataBytesTransmitted;
    uint16_t memBytesSent;
    uint8_t callBacksEnabled[5];
    I2C_CallBackHandleTypeDef callBacks;
};


/* Global functions */
void I2Cx_Send7BitAddress(I2C_TypeDef *instance, uint8_t devAddress, uint8_t numBytes, uint32_t reloadEndMode, uint32_t startStopMode);
void I2Cx_Init(I2C_HandleTypeDef *handle, I2C_TypeDef *instance);
void I2Cx_AddCallBacks(I2C_HandleTypeDef *handle, I2C_CallBackHandle *callBacks, uint8_t callBacksEnabled[5]);
StatusTypeDef I2Cx_Write(I2C_HandleTypeDef *handle, uint8_t devAddress, uint8_t *data, uint16_t dataSize, uint32_t timeout);
StatusTypeDef I2Cx_Read(I2C_HandleTypeDef *handle, uint8_t devAddress, uint8_t *data, uint16_t dataSize, uint32_t timeout);
StatusTypeDef I2Cx_Write_IT(I2C_HandleTypeDef *handle, uint8_t devAddress, uint8_t *data, uint16_t dataSize);
StatusTypeDef I2Cx_MemWrite_IT(I2C_HandleTypeDef *handle, uint8_t devAddress, uint16_t memAddress, uint8_t memSize, uint8_t *data, uint16_t dataSize);
StatusTypeDef I2Cx_MemRead_IT(I2C_HandleTypeDef *handle, uint8_t devAddress, uint16_t memAddress, uint8_t memSize, uint8_t *data, uint16_t dataSize);
void I2Cx_EV_Handler(I2C_HandleTypeDef *handle);


#endif
