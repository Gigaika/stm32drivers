//
// Created by aleksi on 16.8.2020.
//

#ifndef HOMEMONITOR_HTS221_H
#define HOMEMONITOR_HTS221_H

#include <stdint.h>
#include "commons.h"

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

typedef struct {
    uint8_t RESERVED00[15];
    uint8_t WHO_AM_I;
    uint8_t AV_CONF;
    uint8_t RESERVED11[12];
    uint8_t CTRL_REG1;
    uint8_t CTRL_REG2;
    uint8_t CTRL_REG3;
    uint8_t RESERVED23[4];
    uint8_t STATUS_REG;
    uint8_t HUMIDITY_OUT_L;
    uint8_t HUMIDITY_OUT_H;
    uint8_t TEMP_OUT_L;
    uint8_t TEMP_OUT_H;
    uint8_t RESERVED2C[4];
    uint8_t CALIB_0TOF[16];
} HTS221_Reg;

typedef struct {
    StatusTypeDef (*read_reg)(uint16_t memAddress, uint8_t memSize, uint8_t *data, uint16_t dataSize);
    StatusTypeDef (*write_reg)(uint16_t memAddress, uint8_t memSize, uint8_t *data, uint16_t dataSize);
} HTS221_IO_Object;

typedef struct {
    uint8_t SAD;
    HTS221_Reg *registers;
    Peripheral_IO_Object *IO;
    int16_t T0_degC;
    int16_t T1_degC;
    int16_t T0_out;
    int16_t T1_out;
    uint16_t H0_rH;
    uint16_t H1_rH;
    int16_T H0_out;
    int16_t H1_out;
    int16_t temperature;
    uint16_t humidity;
} HTS221_Obj;

HTS221_Init(HTS221_Obj *obj, Peripheral_IO_Object *io);
HTS221_Read(HTS221_Obj *obj);

#endif //HOMEMONITOR_HTS221_H
