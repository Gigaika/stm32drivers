//
// Created by aleksi on 16.8.2020.
//

#ifndef HOMEMONITOR_HTS221_H
#define HOMEMONITOR_HTS221_H

#include <stdint.h>
#include "commons.h"

// IMPORTANT
// Call the exported reg_write_cplt and reg_read_cplt callbacks from their respective interrupt handlers if using IT driven IO

#define HTS221_SAD             0x5F
#define HTS221_WHO_AM_I        0x0F
#define HTS221_AV_CONFR        0x10
#define HTS221_CTRL_REG1       0x20
#define HTS221_CTRL_REG2       0x21
#define HTS221_CTRL_REG3       0x22
#define HTS221_STATUS_REG      0x27
#define HTS221_HUMIDITY_OUT_LR 0x28
#define HTS221_HUMIDITY_OUT_HR 0x29
#define HTS221_TEMP_OUT_L      0x2A
#define HTS221_TEMP_OUT_H      0x2B
#define HTS221_CALIB_0TOF      0x30

typedef enum {
    HTS221_POWEROFF = 0x0,
    HTS221_POWERON  = 0x1
} HTS221_PoweredTypeDef;

typedef enum {
    HTS221_AVGT2   = 0x0,
    HTS221_AVGT4   = 0x1,
    HTS221_AVGT8   = 0x2,
    HTS221_AVGT16  = 0x3,
    HTS221_AVGT32  = 0x4,
    HTS221_AVGT64  = 0x5,
    HTS221_AVGT128 = 0x6,
    HTS221_AVGT256 = 0x7
} HTS221_AVGTTypeDef;

typedef enum {
    HTS221_AVGH4   = 0x0,
    HTS221_AVGH8   = 0x1,
    HTS221_AVGH16  = 0x2,
    HTS221_AVGH32  = 0x3,
    HTS221_AVGH64  = 0x4,
    HTS221_AVGH128 = 0x5,
    HTS221_AVGH256 = 0x6,
    HTS221_AVGH512 = 0x7
} HTS221_AVGTTypeDef;

typedef enum {
    BDU_continuous = 0x0,
    BDU_synced     = 0x1
} HTS221_BDUTypeDef;

typedef enum {
    HTS221_ODR_OS = 0x0,
    HTS221_1HZ    = 0x1,
    HTS221_7HZ    = 0x2,
    HTS221_12HZ   = 0x3
} HTS221_ODRTypeDef;

typedef struct {
    HTS221_DRDY_DISABLED = 0x0,
    HTS221_DRDY_ENABLED = 0x1,
} HTS221_DRDYTypeDef;

typedef struct {
    HTS221_PoweredTypeDef powered;
    HTS221_AVGTTypeDef tempRes;
    HTS221_AVGHTypeDef humRes;
    HTS221_BDUTypeDef bdu;
    HTS221_ODRTypeDef odr;
    HTS221_DRDYTypeDef drdy;
} HTS221_SettingsTypeDef;

typedef struct {
    StatusTypeDef (*read_reg)(uint16_t memAddress, uint8_t memSize, uint8_t *data, uint16_t dataSize);
    StatusTypeDef (*write_reg)(uint16_t memAddress, uint8_t memSize, uint8_t *data, uint16_t dataSize);
    uint8_t read_reg_IT_driven;
    uint8_t write_reg_IT_driven;
} HTS221_IO_Object;

typedef enum {
    HTS221_INITIALIZING = 0x0,
    HTS221_CONFIGURING  = 0x1,
    HTS221_REQUESTING   = 0x2,
    HTS221_READING      = 0x3,
    HTS221_READY        = 0x4
} HTS221_StateTypeDef;

typedef struct {
    int16_t T0_degC;
    int16_t T1_degC;
    int16_t T0_out;
    int16_t T1_out;
    uint16_t H0_rH;
    uint16_t H1_rH;
    int16_T H0_out;
    int16_t H1_out;
} HTS221_CalibrationValuesTypeDef;

typedef struct {
    uint8_t HUMIDITY_OUT_L;
    uint8_t HUMIDITY_OUT_H;
    uint8_t TEMP_OUT_L;
    uint8_t TEMP_OUT_H;
    uint8_t CALIB_0TOF[16];
} HTS221_RegisterTypeDef;

typedef struct {
    uint8_t SAD;
    HTS221_StateTypeDef state;
    HTS221_SettingsTypeDef settings;
    HTS221_RegisterTypeDef registers;
    Peripheral_IO_Object *IO;
    HTS221_CalibrationValuesTypeDef calibrations;
    int16_t temperature;
    uint16_t humidity;
} HTS221_Obj;

void HTS221_Init(HTS221_Obj *obj, Peripheral_IO_Object *io);
void HTS221_SetPowered(HTS221_Obj *obj, HTS221_PoweredTypeDef powered);
void HTS221_SetResolution(HTS221_Obj *obj, HTS221_AVGTTypeDef tempRes, HTS221_AVGHTypeDef humRes);
void HTS221_SetBDU(HTS221_Obj *obj, HTS221_BDUTypeDef bdu);
void HTS221_SetODR(HTS221_Obj *obj, HTS221_ODRTypeDef odr);
void HTS221_SetDRDY(HTS221_Obj *obj, HTS221_DRDYTypeDef drdy);
void HTS221_RequestReading(HTS221_Obj *obj);
void HTS221_Read(HTS221_Obj *obj);

void HTS221_Read_Reg_Cplt_Callback(HTS221_Obj *obj);
void HTS221_Write_Reg_Cplt_Callback(HTS221_Obj *obj);

#endif //HOMEMONITOR_HTS221_H
