// Copyright (c) 2025 Arduino SA
// SPDX-License-Identifier: MPL-2.0

#include "Modulino.h"

// The only singleton that needs to exist
// Build before other objects to fix the Wire object
ModulinoClass Modulino __attribute__ ((init_priority (101)));

ModulinoColor RED(255, 0, 0);
ModulinoColor BLUE(0, 0, 255);
ModulinoColor GREEN(0, 255, 0);
ModulinoColor VIOLET(255, 0, 255);
ModulinoColor WHITE(255, 255, 255);

#if __has_include("Arduino_LED_Matrix.h") && defined(ARDUINO_UNOR4_WIFI)
void __increaseI2CPriority() {
    for (int i = 0; i < 96; i++) {
        if (R_ICU->IELSR[i] == BSP_PRV_IELS_ENUM(EVENT_IIC0_TXI)) {
            NVIC_SetPriority(IRQn_Type(i), 6);
            NVIC_SetPriority(IRQn_Type(i+1), 6);
            NVIC_SetPriority(IRQn_Type(i+2), 6);
            NVIC_SetPriority(IRQn_Type(i+3), 6);
        }
    }
}
#else
void __increaseI2CPriority() {}
#endif