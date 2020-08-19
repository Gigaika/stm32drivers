//
// Created by aleksi on 16.8.2020.
//

#include "hts221.h"

HTS221_Init(HTS221_Obj *obj, Peripheral_IO_Object *io) {
    obj->SAD = 0x5F;
    obj->registers = 0x00;
    obj->IO = io;

    // Todo: write proper config functions
    obj->registers->AV_CONF = 0x3F;
    obj->IO->write_reg(HTS221_AV_CONFR, 1, obj->registers->AV_CONF, 1);
    obj->registers->CTRL_REG1 |= 1 << 7;
    obj->IO->write_reg(HTS_221_CTRL_REG1, 1, obj->registers->CTRL_REG1, 1);

    obj->IO->read_reg(HTS221_CALIB_0TOF, 1, obj->registers->CALIB_0TOF, 16);

    obj->T0_degC = ((uint16_t)obj->registers->CALIB_0TOF[2] | (((uint16_t)obj->registers->CALIB_0TOF[5] & 0x3) << 8)) >> 3;
    obj->T1_degC = ((uint16_t)obj->registers->CALIB_0TOF[3] | ((((uint16_t)obj->registers->CALIB_0TOF[5] >> 2) & 0x3) << 8)) >> 3;
    obj->T0_out = ((uint16_t)obj->registers->CALIB_0TOF[12] | ((uint16_t)obj->registers->CALIB_0TOF[13] << 8));
    obj->T1_out = ((uint16_t)obj->registers->CALIB_0TOF[13] | ((uint16_t)obj->registers->CALIB_0TOF[14] << 8));
    obj->H0_rH = ((uint16_t)obj->registers->CALIB_0TOF[0] / 2);
    obj->H1_rH = ((uint16_t)obj->registers->CALIB_0TOF[1] / 2);
    obj->H0_out = ((uint16_t)obj->registers->CALIB_0TOF[6] | ((uint16_t)obj->registers->CALIB_0TOF[7] << 8));
    obj->H1_out = ((uint16_t)obj->registers->CALIB_0TOF[10] | ((uint16_t)obj->registers->CALIB_0TOF[11] << 8));
}

HTS221_Read(HTS221_Obj *obj) {
    obj->IO->read_reg(HTS221_HUMIDITY_OUT_L, 1, obj->registers->HUMIDITY_OUT_L, 4);

    int16_t raw_H = ((uint16_t)obj->registers->HUMIDITY_OUT_L | ((uint16_t)obj->registers->HUMIDITY_OUT_H << 8));
    obj->humidity = ((obj->H1_rH - obj->H0_rH)*(raw_H-obj->H0_out))/(obj->H1_out-obj->H0_out)+obj->H0_rH;
    int16_t raw_T = ((uint16_t)obj->registers->TEMP_OUT_L | ((uint16_t)obj->registers->TEMP_OUT_H << 8));
    obj->temperature = ((obj->T1_degC - obj->T0_degC) * (raw_T - obj->T0_out)) / (obj->T1_out-obj->T0_out) + obj->T0_degC;
}