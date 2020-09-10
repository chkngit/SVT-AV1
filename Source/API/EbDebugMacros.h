/*
* Copyright(c) 2020 Intel Corporation
*
* This source code is subject to the terms of the BSD 2 Clause License and
* the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
* was not distributed with this source code in the LICENSE file, you can
* obtain it at https://www.aomedia.org/license/software-license. If the Alliance for Open
* Media Patent License 1.0 was not distributed with this source code in the
* PATENTS file, you can obtain it at https://www.aomedia.org/license/patent-license.
*/


/*
* This file contains only debug macros that are used during the development
* and are supposed to be cleaned up every tag cycle
* all macros must have the following format:
* - enabling a feature should be prefixed by ENABLE_
* - disableing a feature should be prefixed by DISABLE_
* - tuning a feature should be prefixed by TUNE_
* - adding a new feature should be prefixed by FEATURE_
* - bug fixes should be prefixed by FIX_
* - all macros must have a coherent comment explaining what the MACRO is doing
* - #if 0 / #if 1 are not to be used
*/

#ifndef EbDebugMacros_h
#define EbDebugMacros_h

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// undefining this macro would allow the AVX512 optimization to be enabled by default
#ifndef NON_AVX512_SUPPORT
#define NON_AVX512_SUPPORT
#endif

//FOR DEBUGGING - Do not remove
#define NO_ENCDEC         0 // bypass encDec to test cmpliance of MD. complained achieved when skip_flag is OFF. Port sample code from VCI-SW_AV1_Candidate1 branch

// SVT-03-FASTER-SVT
#define PRED_ONLY_B_SLICE 0
#define END_ZERO          0
#define START_ZERO        0
#define PRED_ONLY_ALL     1
#define BYPASS_PD0        0
#define SHUT_RDOQ         1
#define SHUT_PME          0
#define SHUT_SUBPEL_ME    0
#define SHUT_SUBPEL_PME   0

#define SHUT_TXT_B_SLICE  0
#define SHUT_TXT_ALL      1

#define NIC_1_MDS1        0
#define BYPASS_MDS1       0

#define SHUT_ADAPT_ME     0 

#define SHUT_CDEF         1 

#define SHUT_TXS          1
#define SHUT_WARP         1
#define SHUT_TF           1

#define SHUT_MRP          0 // Missing code in svt-03
#define SHUT_GM           1
#define SHUT_CFL          1
#define SHUT_REST         1
#define SHUT_MFMV         1
#define SHUT_OBMC         1
#define SHUT_HIGH_PREC    1
#define SHUT_FILTER_INTRA 1
#define SHUT_COMPOUND     1
#define SHUT_PALETTE      1
#define SHUT_RATE_EST     1
#define SHUT_IFS          1

#define SET_NIC_1         1

#define FIX_HME_REF_COUNT 0 // Missing code in svt-03

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // EbDebugMacros_h
