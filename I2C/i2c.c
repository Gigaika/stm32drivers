#include "i2c.h"

static void I2Cx_ChangeState(I2C_HandleTypeDef *handle, I2C_StateTypeDef newState);
static void I2Cx_PrepareHandle(I2C_HandleTypeDef *handle, I2C_OperationTypeDef operation, uint8_t devAddress, uint16_t memAddress, uint8_t memSize, uint8_t *data, uint16_t dataSize);
static void I2Cx_ResetHandle(I2C_HandleTypeDef *handle, I2C_OperationTypeDef operation, uint8_t devAddress, uint16_t memAddress, uint8_t memSize, uint8_t *data, uint16_t dataSize);

StatusTypeDef I2Cx_Init(I2C_HandleTypeDef *handle, I2C_TypeDef *instance) {
    I2Cx_ResetHandle(handle);
    handle->instance = instance;
}

/**
 * @brief Helper function to simplify recording the previous states
 */
static void I2Cx_ChangeState(I2C_HandleTypeDef *handle, I2C_StateTypeDef newState) {
	I2C_StateTypeDef tmp = handle->state;
	handle->state = newState;
	handle->previousState = tmp;
}

/**
 * @brief Helper function to prepare the TX handle
 */
static void I2Cx_PrepareHandle(I2C_HandleTypeDef *handle, I2C_OperationTypeDef operation, uint8_t devAddress, uint16_t memAddress, uint8_t memSize, uint8_t *data, uint16_t dataSize) {
	handle->devAddress = devAddress;
	handle->operation = operation;
	handle->memAddress = memAddress;
	handle->memSize = memSize;
	handle->dataBuffer = data;
	handle->dataSize = dataSize;
	handle->error = I2C_ERRROR_NONE;
}

static void I2Cx_ResetHandle(I2C_HandleTypeDef *handle) {
    handle->operation = I2C_NONE;
    handle->state = I2C_READY
    handle->error = I2C_ERRROR_NONE;
    handle->devAddress = 0;
    handle->memAddress = 0;
    handle->memSize = 0;
    handle->dataBuffer = NULL;
    handle->dataSize = 0;
    handle->dataBytesTransmitted = 0;
    handle->memBytesSent = 0;
}

/**
  * @brief Sends a 7-bit slave address using the specified I2C peripheral
  */
void I2Cx_Send7BitAddress(I2C_TypeDef *instance, uint8_t devAddress, uint8_t numBytes, uint32_t reloadEndMode, uint32_t startStopMode)
{
    uint32_t tmprg = instance->CR2;
    
    /* clear the necessary bits from CR2 */
    tmprg &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP);
    
    /* set the necessary bits to CR2 */
    tmprg |= (uint8_t)(devAddress << 1);
    tmprg |= (uint32_t)numBytes << I2C_CR2_NBYTES_Pos;
    tmprg |= (reloadEndMode | startStopMode);
    
    instance->CR2 = tmprg;
}

/**
  *@brief Writes data to a specified I2C peripheral in polling mode
  */
StatusTypeDef I2Cx_Write(I2C_HandleTypeDef *handle, uint8_t devAddress, uint8_t *data, uint16_t dataSize)
{
   I2C_TypeDef *instance = handle->instance;
   uint32_t timeout = 0;
  
   if (instance->ISR & I2C_ISR_BUSY || handle->state != I2C_READY)
    {
      handle->error = I2C_ERROR_BUSY;
      return STATUS_BUSY;
    }
   
   handle->operation = I2C_WRITE;
   I2Cx_ChangeState(handle, I2C_BUSY_TX_SUBADDRESS);
   handle->error = I2C_ERRROR_NONE;
   
   // Disable I2C interrupts since we are polling
   instance->CR1 &= ~(I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE);
   
   I2Cx_Send7BitAddress(instance, devAddress, dataSize, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);
   
   I2Cx_ChangeState(handle, I2C_BUSY_TX);
   
   uint16_t numbytesSent = 0;
   while (numbytesSent < dataSize)
   {
     timeout = I2C_TIMEOUT;
     while (!(instance->ISR & I2C_ISR_TXIS))
     {
       if ((timeout--) == 0)
       {
         handle->error = I2C_ERROR_TIMEOUT;
         return STATUS_TIMEOUT;
       }
     }
     
     instance->TXDR = *(data++);
     numbytesSent++;
   }
   
   handle->operation = I2C_NONE;
   I2Cx_ChangeState(handle, I2C_READY);
   
   return STATUS_OK;
}

/**
  *@brief Reads data from a specified I2C peripheral in polling mode
  */
StatusTypeDef I2Cx_Read(I2C_HandleTypeDef *handle, uint8_t devAddress, uint8_t *data, uint16_t dataSize, uint32_t timeout)
{
   I2C_TypeDef *instance = handle->instance;
   uint32_t timeout = 0;
  
   if (instance->ISR & I2C_ISR_BUSY || handle->state != I2C_READY)
    {
      handle->error = I2C_ERROR_BUSY;
      return STATUS_BUSY;
    }
   
   handle->operation = I2C_READ;
   I2Cx_ChangeState(handle, I2C_BUSY_TX_SUBADDRESS);
   handle->error = I2C_ERRROR_NONE;
   
   // Disable I2C interrupts since we are polling
   instance->CR1 &= ~(I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE);
   
   I2Cx_Send7BitAddress(instance, devAddress, dataSize, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
   
   I2Cx_ChangeState(handle, I2C_BUSY_RX);
   
   uint16_t numbytesRead = 0;
   while (numbytesRead < dataSize)
   {
     while (!(instance->ISR & I2C_ISR_RXNE))
     {
       if ((timeout--) == 0)
       {
         handle->error = I2C_ERROR_TIMEOUT;
         return STATUS_TIMEOUT;
       }
     }
     
     *(data++) = instance->RXDR;
     numbytesRead++;
   }
   
   handle->operation = I2C_NONE;
   I2Cx_ChangeState(handle, I2C_READY);
   
   return STATUS_OK;
}

/**
  *@brief Writes data to a specified I2C peripheral in interrupt driven mode
  */
StatusTypeDef I2Cx_Write_IT(I2C_HandleTypeDef *handle, uint8_t devAddress, uint8_t *data, uint16_t dataSize)
{
    I2C_TypeDef *instance = handle->instance;

    // Check if the peripheral or handle indicates I2C is busy
    if (instance->ISR & I2C_ISR_BUSY || handle->state != I2C_READY)
    {
      handle->error = I2C_ERROR_BUSY;
      return STATUS_BUSY;
    }

    // Prepare the handle with transmission parameters
    I2Cx_PrepareHandle(handle, I2C_WRITE_IT, devAddress, 0x00, 0x00, data, dataSize);
    I2Cx_ChangeState(handle, I2C_BUSY_TX);

    // Enable needed I2C interrupts
    instance->CR1 |= (I2C_CR1_TXIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE);

    // Address phase
    I2Cx_Send7BitAddress(instance, devAddress, dataSize, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);

    return STATUS_OK;
}

/**
  *@brief Writes data to a specified I2C peripheral in interrupt driven mode
  */
StatusTypeDef I2Cx_Read_IT(I2C_HandleTypeDef *handle, uint8_t devAddress, uint8_t *data, uint16_t dataSize)
{
    I2C_TypeDef *instance = handle->instance;

    // Check if the peripheral or handle indicates I2C is busy
    if (instance->ISR & I2C_ISR_BUSY || handle->state != I2C_READY)
    {
      handle->error = I2C_ERROR_BUSY;
      return STATUS_BUSY;
    }

    // Prepare the handle with transmission parameters
    I2Cx_PrepareHandle(handle, I2C_READ_IT, devAddress, 0x00, 0x00, data, dataSize);
    I2Cx_ChangeState(handle, I2C_BUSY_RX);

    // Enable needed I2C interrupts
    instance->CR1 |= (I2C_CR1_RXIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE);

    // Address phase
    I2Cx_Send7BitAddress(instance, devAddress, dataSize, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);

    return STATUS_OK;
}

/**
  * @brief Writes data to a device register using interrupt driven I2C
  * @param handle: Pointer to the I2C handle that interrupts use
  */
StatusTypeDef I2Cx_MemWrite_IT(I2C_HandleTypeDef *handle, uint8_t devAddress, uint16_t memAddress, uint8_t memSize, uint8_t *data, uint16_t dataSize)
{
    I2C_TypeDef *instance = handle->instance;

    // Check if the peripheral or handle indicates I2C is busy
    if (instance->ISR & I2C_ISR_BUSY || handle->state != I2C_READY)
    {
      handle->error = I2C_ERROR_BUSY;
      return STATUS_BUSY;
    }
    
    // Prepare the handle with transmission parameters
    I2Cx_PrepareHandle(handle, I2C_MEM_WRITE, devAddress, memAddress, memSize, data, dataSize);
	I2Cx_ChangeState(handle, I2C_BUSY_TX_SUBADDRESS);
    
    // Enable needed I2C interrupts
    instance->CR1 |= (I2C_CR1_TXIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE);
    
    // Address phase
    I2Cx_Send7BitAddress(instance, devAddress, memSize, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);

    return STATUS_OK;
}

/**
  * @brief Writes data to a device register using interrupt driven I2C
  * @param handle: Pointer to the I2C handle that interrupts use
  */
StatusTypeDef I2Cx_MemRead_IT(I2C_HandleTypeDef *handle, uint8_t devAddress, uint16_t memAddress, uint8_t memSize, uint8_t *data, uint16_t dataSize)
{
    I2C_TypeDef *instance = handle->instance;
    
    // Check if the peripheral or handle indicates I2C is busy
    if (instance->ISR & I2C_ISR_BUSY || handle->state != I2C_READY)
    {
      handle->error = I2C_ERROR_BUSY;
      return STATUS_BUSY;
    }
    
    // Prepare the handle with transmission parameters
    I2Cx_PrepareHandle(handle, I2C_MEM_READ, devAddress, memAddress, memSize, data, dataSize);
	I2Cx_ChangeState(handle, I2C_BUSY_TX_SUBADDRESS);
    
    // Enable needed I2C interrupts
    instance->CR1 |= (I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE);
    
    // Address phase
    I2Cx_Send7BitAddress(instance, devAddress, memSize, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);

    return STATUS_OK;
}

/**
  *@brief Weak callback that gets called when a I2C transfers results in a NACK
  *       Should be overwritten in user code.
  */
__weak void I2C_NackReceivedCallBack(I2C_HandleTypeDef *handle) {

/**
  *@brief Handles I2C NACK interrupts
  */
void I2Cx_NACKF_CallBack(I2C_HandleTypeDef *handle) {
  handle->error = I2C_ERROR_NACK;
  I2C_NackReceivedCallBack(handle);
}

/**
  *@brief Handles I2C transmit interrupts
  */
void I2Cx_TXIS_CallBack(I2C_HandleTypeDef *handle) {
  I2C_TypeDef *instance = handle->instance;
  
  switch (handle->state) 
  {
    case I2C_BUSY_TX_SUBADDRESS:
    {
      uint8_t bytesSent = handle->memBytesSent++;
      uint8_t memSize = handle->memSize;
      // Sending address MSB first
      instance->TXDR = (uint8_t)(handle->memAddress >> (((memSize - 1) - bytesSent) * 8)); 
      break;
    }
    case I2C_BUSY_TX:
    {
      uint8_t bytesSent = handle->dataBytesTransmitted++;
      uint8_t dataSize = handle->dataSize;
      // Sending data MSB first
      uint8_t arrSize = dataSize - 1;
      instance->TXDR = handle->dataBuffer[arrSize - (arrSize - bytesSent)];
      break;
    }
  }
}

/**
  *@brief Handles I2C reception interrupts
  */
void I2Cx_RXNE_CallBack(I2C_HandleTypeDef *handle) {
  I2C_TypeDef *instance = handle->instance;
  
  switch (handle->state) 
  {
    case I2C_BUSY_RX:
    {
      uint8_t bytesRead = handle->dataBytesTransmitted++;
      uint8_t dataSize = handle->dataSize;
      // Reading data MSB first
      handle->dataBuffer[(dataSize - 1) - bytesRead++] = instance->RXDR;    
      break;
    }
  }
}

/**
  *@brief Callback that gets called when a I2C write is completed.
  *  Should be overwritten in user code
  */
__weak void I2C_WriteCpltCallBack(I2C_HandleTypeDef *handle) {
}

/**
  *@brief Callback that gets called when a I2C read is completed.
  *  Should be overwritten in user code
  */
__weak void I2C_ReadCpltCallBack(I2C_HandleTypeDef *handle) {
}

/**
  *@brief Callback that gets called when a I2C memory write is completed.
  *  Should be overwritten in user code
  */
__weak void I2C_MemTxCpltCallBack(I2C_HandleTypeDef *handle) {
}

/**
  *@brief Callback that gets called when a I2C memory read is completed.
  *  Should be overwritten in user code
  */
__weak void I2C_MemRxCpltCallBack(I2C_HandleTypeDef *handle) {
}

/**
  *@brief Handles I2C transmission completed and stop generated interrupts
  */
void I2Cx_TC_CallBack(I2C_HandleTypeDef *handle) {
   I2C_TypeDef *instance = handle->instance;
  
  switch (handle->state)
  {
    case I2C_BUSY_TX_SUBADDRESS:
    {
      if (handle->operation == I2C_MEM_WRITE) {
    	I2Cx_ChangeState(handle, I2C_BUSY_TX);
        I2Cx_Send7BitAddress(instance, handle->devAddress, handle->dataSize, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);
      } else if (handle->operation == I2C_MEM_READ) {
    	I2Cx_ChangeState(handle, I2C_BUSY_RX);
        I2Cx_Send7BitAddress(instance, handle->devAddress, handle->dataSize, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
      }
    }
      
      break;
    }
    case I2C_BUSY_TX:
    {
      if (handle->operation == I2C_MEM_WRITE) {
        I2C_MemTxCpltCallBack(handle);
        I2Cx_ResetHandle(handle);
      } else if (handle->operation == I2C_READ_IT) {
        I2C_ReadCpltCallBack(handle);
        I2Cx_ResetHandle(handle);
      }
      
      break;
    }
    case I2C_BUSY_RX:
    {
      if (handle->operation == I2C_MEM_READ) {
        I2C_MemRxCpltCallBack(handle);
        I2Cx_ResetHandle(handle);
      } else if (handle->operation == I2C_READ_IT) {
        I2C_ReadCpltCallBack(handle);
        I2Cx_ResetHandle(handle);
      }
      
      break;
    }
  }
}

/**
  *@brief Distributes the I2C interrupts to their respective callbacks.
  *       Should be called from the ARM I2CX_EV_IRQHandler function.

  *@param handle: Pointer to a I2C handle
  */
void I2Cx_EV_Handler(I2C_HandleTypeDef *handle)
{
    I2C_TypeDef *instance = handle->instance;
    uint32_t itflags = instance->ISR;
    uint32_t itsources = instance->CR1;
    
    if (itflags & I2C_ISR_NACKF) {
      I2Cx_NACKF_CallBack(handle);
    } else if (itflags & I2C_ISR_RXNE) {
      I2Cx_RXNE_CallBack(handle);
    } else if (itflags & I2C_ISR_TXIS) {
      I2Cx_TXIS_CallBack(handle);
    } else if (itflags & (I2C_ISR_STOPF)) {
      I2Cx_TC_CallBack(handle);
    }
}
