/* The Clear BSD License
 *
 * Copyright (c) 2025 EdgeImpulse Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "systick_handler.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#ifndef SYSTICK_IRQ_PRIORITY
#define SYSTICK_IRQ_PRIORITY    0xFFU
#endif

#ifndef SYSTICK_CORRECTION_FACTOR
#define SYSTICK_CORRECTION_FACTOR    (1.0)
#endif

static uint32_t _ms_count;
static uint32_t fact;

/**
 * @brief 
 * 
 */
void systick_handler_init(void)
{
    _ms_count = 0;

    // Set SysTick Interrupt Priority
    NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
    fact = (SystemCoreClock / 1000000);
    
    SysTick->LOAD = 0xFFFFFF;   // 24 bit timer
    SysTick->VAL  =  0U;    //
    SysTick->CTRL =  SysTick_CTRL_CLKSOURCE_Msk // processor clock source
                //| SysTick_CTRL_TICKINT_Msk  // Enables SysTick exception request - not handled in AVH
                | SysTick_CTRL_ENABLE_Msk;  // Enable the counter
}

/**
 * @brief 
 * 
 * @return uint32_t 
 */
uint32_t systick_handler_get(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  // stop

    _ms_count += ((SysTick->LOAD - SysTick->VAL) / fact);
    SysTick->VAL  =  0U;    // reset

    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;   // restart

    return (_ms_count * SYSTICK_CORRECTION_FACTOR);
}
