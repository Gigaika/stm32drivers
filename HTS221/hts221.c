//
// Created by aleksi on 16.8.2020.
//

#include "hts221.h"

void HTS221_Read_Reg_Cplt_Callback(HTS221_Obj *obj) {
    switch(obj->state) {
        case HTS221_INITIALIZING:
        {
            obj->calibrations.T0_degC = ((uint16_t)obj->registers.CALIB_0TOF[2] | (((uint16_t)obj->registers.CALIB_0TOF[5] & 0x3) << 8)) >> 3;
            obj->calibrations.T1_degC = ((uint16_t)obj->registers.CALIB_0TOF[3] | ((((uint16_t)obj->registers.CALIB_0TOF[5] >> 2) & 0x3) << 8)) >> 3;
            obj->calibrations.T0_out = ((uint16_t)obj->registers.CALIB_0TOF[12] | ((uint16_t)obj->registers.CALIB_0TOF[13] << 8));
            obj->calibrations.T1_out = ((uint16_t)obj->registers.CALIB_0TOF[13] | ((uint16_t)obj->registers.CALIB_0TOF[14] << 8));
            obj->calibrations.H0_rH = ((uint16_t)obj->registers.CALIB_0TOF[0] / 2);
            obj->calibrations.H1_rH = ((uint16_t)obj->registers.CALIB_0TOF[1] / 2);
            obj->calibrations.H0_out = ((uint16_t)obj->registers.CALIB_0TOF[6] | ((uint16_t)obj->registers.CALIB_0TOF[7] << 8));
            obj->calibrations.H1_out = ((uint16_t)obj->registers.CALIB_0TOF[10] | ((uint16_t)obj->registers.CALIB_0TOF[11] << 8));
            obj->state = HTS221_READY;
            break;
        }
        case HTS221_READING:
        {
            int16_t T0_degC = obj->calibrations.T0_degC;
            int16_t T1_degC = obj->calibrations.T1_degC,
            int16_t T0_out = obj->calibrations.T0_out;
            int16_t T1_out = obj->calibrations.T1_out;
            uint16_t H0_rH = obj->calibrations.H0_rH;
            uint16_t H1_rH = obj->calibrations.H1_rH;
            int16_t H0_out = obj->calibrations.H0_out;
            int16_t H1_out = obj->calibrations.H1_out;

            int16_t raw_H = ((uint16_t)obj->registers.HUMIDITY_OUT_L | ((uint16_t)obj->registers.HUMIDITY_OUT_H << 8));
            obj->humidity = ((obj->H1_rH - obj->H0_rH)*(raw_H-obj->H0_out))/(obj->H1_out-obj->H0_out)+obj->H0_rH;
            int16_t raw_T = ((uint16_t)obj->registers.TEMP_OUT_L | ((uint16_t)obj->registers.TEMP_OUT_H << 8));
            obj->temperature = ((obj->T1_degC - obj->T0_degC) * (raw_T - obj->T0_out)) / (obj->T1_out-obj->T0_out) + obj->T0_degC;
            break;
        }
        default:
            obj->state = HTS221_READY;
            break;
    }
}

void HTS221_Write_Reg_Cplt_Callback(HTS221_Obj *obj) {
    switch(obj->state) {
        default:
            obj->state = HTS221_READY;
            break;
    }
}

void HTS221_Init(HTS221_Obj *obj, Peripheral_IO_Object *io) {
    obj->state = HTS221_INITIALIZING;
    obj->SAD = HTS221_SAD;
    obj->IO = io;

    obj->IO->read_reg(HTS221_CALIB_0TOF, 1, obj->registers.CALIB_0TOF, 16);
    if (!obj->IO.read_reg_IT_driven) {
        HTS221_Read_Reg_Cplt_Callback();
    }
}

void HTS221_SetPowered(HTS221_Obj *obj, HTS221_PoweredTypeDef powered) {
    obj->state = HTS221_CONFIGURING;
    obj->IO->write_reg(HTS221_CTRL_REG1, 1, powered << 7, 1);
    if (!obj->IO.write_reg_IT_driven) {
        HTS221_Write_Reg_Cplt_Callback();
    }
}

void HTS221_SetResolution(HTS221_Obj *obj, HTS221_AVGTTypeDef tempRes, HTS221_AVGHTypeDef humRes) {
    obj->state = HTS221_CONFIGURING;
    uint8_t tempReg = 0;
    tempReg |= tempRes << 3;
    tempReg |= humRes;
    obj->IO->write_reg(HTS221_AV_CONFR, 1, tempReg, 1);
    if (!obj->IO.write_reg_IT_driven) {
        HTS221_Write_Reg_Cplt_Callback();
    }
}

void HTS221_SetBDU(HTS221_Obj *obj, HTS221_BDUTypeDef bdu) {
    obj->state = HTS221_CONFIGURING;
    obj->IO->write_reg(HTS221_CTRL_REG1, 1, bdu << 2, 1);
    if (!obj->IO.write_reg_IT_driven) {
        HTS221_Write_Reg_Cplt_Callback();
    }
}

void HTS221_SetODR(HTS221_Obj *obj, HTS221_ODRTypeDef odr) {
    obj->state = HTS221_CONFIGURING;
    obj->IO->write_reg(HTS221_CTRL_REG1, 1, odr, 1);
    if (!obj->IO.write_reg_IT_driven) {
        HTS221_Write_Reg_Cplt_Callback();
    }
}

void HTS221_SetDRDY(HTS221_Obj *obj, HTS221_DRDYTypeDef drdy) {
    obj->state = HTS221_CONFIGURING;
    obj->IO->write_reg(HTS221_CTRL_REG3, 1, drdy << 2, 1);
    if (!obj->IO.write_reg_IT_driven) {
        HTS221_Write_Reg_Cplt_Callback();
    }
}

void HTS221_RequestReading(HTS221_Obj *obj) {
    obj->state = HTS221_REQUESTING;
    obj->IO->write_reg(HTS221_CTRL_REG2, 1, 0x1, 1);
    if (!obj->IO.write_reg_IT_driven) {
        HTS221_Write_Reg_Cplt_Callback();
    }
}

void HTS221_Read(HTS221_Obj *obj) {
    obj->state = HTS221_READING;
    obj->IO->read_reg(HTS221_HUMIDITY_OUT_L, 1, obj->registers.HUMIDITY_OUT_L, 4);
    if (!obj->IO.read_reg_IT_driven) {
        HTS221_Read_Reg_Cplt_Callback();
    }
}