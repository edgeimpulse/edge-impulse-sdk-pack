/*
 * Copyright (c) 2024 EdgeImpulse Inc.
 *
 * Generated by Edge Impulse and licensed under the applicable Edge Impulse
 * Terms of Service. Community and Professional Terms of Service
 * (https://edgeimpulse.com/legal/terms-of-service) or Enterprise Terms of
 * Service (https://edgeimpulse.com/legal/enterprise-terms-of-service),
 * according to your product plan subscription (the “License”).
 *
 * This software, documentation and other associated files (collectively referred
 * to as the “Software”) is a single SDK variation generated by the Edge Impulse
 * platform and requires an active paid Edge Impulse subscription to use this
 * Software for any purpose.
 *
 * You may NOT use this Software unless you have an active Edge Impulse subscription
 * that meets the eligibility requirements for the applicable License, subject to
 * your full and continued compliance with the terms and conditions of the License,
 * including without limitation any usage restrictions under the applicable License.
 *
 * If you do not have an active Edge Impulse product plan subscription, or if use
 * of this Software exceeds the usage limitations of your Edge Impulse product plan
 * subscription, you are not permitted to use this Software and must immediately
 * delete and erase all copies of this Software within your control or possession.
 * Edge Impulse reserves all rights and remedies available to enforce its rights.
 *
 * Unless required by applicable law or agreed to in writing, the Software is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing
 * permissions, disclaimers and limitations under the License.
 */
#ifndef __EI_UTILS__H__
#define __EI_UTILS__H__

#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

// Stringify
#define ei_xstr(a) ei_str(a)
#define ei_str(a) #a

// Bit manipulation

//Set bit y (0-indexed) of x to '1' by generating a a mask with a '1' in the proper bit location and ORing x with the mask.
#define SET_BIT_POS(x,y) (x |= (1 << y))

//Set bit y (0-indexed) of x to '0' by generating a mask with a '0' in the y position and 1's elsewhere then ANDing the mask with x.
#define CLEAR_BIT_POS(x,y) (x &= ~(1<< y))

//Return '1' if the bit value at position y within x is '1' and '0' if it's 0 by ANDing x with a bit mask where the bit in y's position is '1' and '0' elsewhere and comparing it to all 0's.  Returns '1' in least significant bit position if the value of the bit is '1', '0' if it was '0'.
#define TEST_BIT_POS(x,y) ((0u == (x & (1<<y)))?0u:1u)

//Toggle bit y (0-index) of x to the inverse: '0' becomes '1', '1' becomes '0' by XORing x with a bitmask where the bit in position y is '1' and all others are '0'.
#define TOGGLE_BIT_POS(x,y) (x ^= (1<<y))

// Set the flag bits in word.
#define 	SET_BIT_MASK(y, flag)   ( y |= (flag) )

// Clear the flag bits in word.
#define 	CLEAR_BIT_MASK(y, flag)   ( y &= ~(flag) )

// Flip the flag bits in word.
#define 	TOGGLE_BIT_MASK(y, flag)   ( y ^= (flag) )

// Test whether all the flag bits in word are set.
#define 	TEST_BIT_MASK(y, flag)   ( ((y)&(flag)) == (flag) )

#define EI_TRY(x) { auto res = (x); if(res != EIDSP_OK) { return res; } }

#endif  //!__EI_UTILS__H__