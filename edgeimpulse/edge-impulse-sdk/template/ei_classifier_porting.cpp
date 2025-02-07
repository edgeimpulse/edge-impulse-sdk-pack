/*
 * Copyright (c) 2024 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS
 * IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Includes */
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"

#include <stdarg.h>
#include <stdlib.h>
#include <cstdio>
#include <string.h>
#include <math.h>

/**
 * @brief 
 * 
 * @return EI_IMPULSE_ERROR 
 */
EI_IMPULSE_ERROR ei_run_impulse_check_canceled() 
{
    // TODO
    return EI_IMPULSE_OK;
}

/**
 * @brief sleep for the given time in ms
 * 
 * @param time_ms time in ms
 * 
 */
EI_IMPULSE_ERROR ei_sleep(int32_t time_ms) {

    uint64_t start_time = ei_read_timer_ms();

    while ((start_time + time_ms) > ei_read_timer_ms()){};

    return EI_IMPULSE_OK;
}

/**
 * @brief Read the time in ms
 * 
 * @return uint64_t time in ms
 */
uint64_t ei_read_timer_ms()
{
    // TODO
    return 0;
}

/**
 * @brief Read the time in us
 * 
 * 
 * @return uint64_t time in us
 */
uint64_t ei_read_timer_us()
{
    // TODO
    return 0;
}

/**
 * @brief Read a character from the standard input.
 * 
 */
char ei_getchar()
{
    // TODO
    char ch = 0;
    return ch;
}

/**
 * @brief Wrapper for printf function. The string is to be printed on the console. 
 * 
 * @note This function is weakly linked and can be overridden by the user.
 * 
 */
void ei_printf(const char *format, ...) 
{
    // TODO
}

/**
 * @brief Print a float on console. Needed if printf of float number is not supported by the platform
 * 
 * @note This function is weakly linked and can be overridden by the user.
 * 
 * @param f float number to be printed
 */
void ei_printf_float(float f) {
    // TODO
    ei_printf("%f", f);
}

/**
 * @brief Print char on console
 * 
 * @param c 
 */
void ei_putchar(char c)
{
    // TODO
}

/**
 * @brief malloc wrapper
 * 
 */
void *ei_malloc(size_t size) {
    return malloc(size);
}

/**
 * @brief calloc wrapper
 * 
 */
void *ei_calloc(size_t nitems, size_t size) {
    return calloc(nitems, size);
}

/**
 * @brief free wrapper
 * 
 */
void ei_free(void *ptr) {
    free(ptr);
}

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C"
#endif
void DebugLog(const char* s) {
    ei_printf("%s", s);
}

