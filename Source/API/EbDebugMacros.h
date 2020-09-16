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

#define COST_BASED_PRED_ONLY 1
#define ADD_LEVELS 1
#define PARENT_COST 0
/************************************/
#define REMOVE_USELESS_0 1
#define REMOVE_USELESS_1 1
#define LOSSLESS_OPT 1
#define PD0_SHUT_SKIP_DC_SIGN_UPDATE 1 //---->
/************************************/
#define PD0_REDUCE_ME_INTER_CAND 0
/************************************/
#define PRED_ONLY 0
#define FASTER_PD0 0
#define ENHANCED_FASTER_PD0 0
/************************************/
#define TF_CHROMA_BLIND 1
#define SHUT_TF 0
/************************************/
#define OPT_4           0
/************************************/

#define SHUT_TXT_STATS 1 // Set0: REF = DCT - DCT + H - DCT + V - DCT(what you have added)
#define REMOVE_TXT_STATS 1
#define TX_TYPE_GROUPING 1

#define COST_BASED_TXT 0
#define DCT_VS_DST  0 // to test for only INTRA CLASS
#define TXT_OFF 0
/************************************/
#define TEST_BLOCK_BASED 0
/************************************/
#define SHUT_RDOQ 0 
#define SHUT_FP_QUANT 0 
#define SHUT_FP_QUANT_TX_SIZE 0
#define SHUT_FP_QUANT_CHROMA 0
#define SHUT_FP_QUANT_TX_TYPE 0
#define SHUT_FP_QUANT_INTER 0 
#define SHUT_FP_QUANT_INTRA 0

#define FAST_RDOQ 0 
#define FAST_RDOQ_INTER 0
#define FAST_RDOQ_EOB 0
#define FAST_RDOQ_CHROMA 1
#define FAST_RDOQ_TH 0
/************************************/
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // EbDebugMacros_h
