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

// START  svt-03 /////////////////////////////////////////////////////////
#define FEATURE_MDS2 1 // TXT @ MDS2 if CLASS_0_3, and TXS/RDOQ @ MDS2 if CLASS_1_2
#define PR1481       1 //Fix memory leaks from valgrind
#define PR1485       1 //Fix mismatch C/AVX2 kernel svt_av1_apply_temporal_filter_planewise()

#define FIX_RC_BUG 1 // PR:1484 Fix the one pass QP assignment using frames_to_be_encoded
#define FIX_VBR_BUG 1 // PR:1484 Fix 1st pass bug (bug from rebasing the branch)
#define FIX_10BIT     1 // PR:1484 fix 1st pass for 10bit input
#define FIX_RC_TOKEN     1 // PR:1484 fix RC token check to include double dash
//***************************************************************//
// Jing to rename the flags to follow the above guidelines
#define PAD_CHROMA_AFTER_MCTF 1 // Padding chroma after altref
#define NEW_DELAY             1 //Change delay some sorts of I in PicDecision
#if NEW_DELAY
#define FIX_LAD_DEADLOCK      1 //fix deadlock when lad>0 + iperiod>0
#define NEW_DELAY_DBG_MSG     0
#define SCD_LAD            6  //number of future frames
#define PD_WINDOW_SIZE     (SCD_LAD +2) //adding previous+current to future
#define MAX_TPL_GROUP_SIZE 64 //enough to cover 6L gop
#endif

#define INL_ME 1 //In loop me
#if INL_ME
#define IME_REUSE_TPL_RESULT 1 // Reuse TPL results for iLoopME
#define INL_TPL_ENHANCEMENT 1 // Refinement for TPL
#define INL_TPL_ME_DBG_MSG  0 // Debug message
#define INL_ME_DBG 1 //In loop me for debug
#if INL_ME_DBG
#define INL_ME_ON_INPUT_DBG 1 //ME on original debug
#define INL_GM_ON_INPUT_DBG 1 //GM on original debug
#define INL_TPL_ME_ON_INPUT_DBG 1 // use original in TPL
#define INL_ME_DBG_MEM_OPT  1
#endif
#if !IME_REUSE_TPL_RESULT
#define SIGNAL_TPL_ME_OQ 1 // A separate signal_xxx_oq for TPL ME
#endif
#endif
//***************************************************************//

#define FEATURE_IN_LOOP_TPL 1 // Moving TPL to in loop
#if FEATURE_IN_LOOP_TPL
#define ENABLE_TPL_ZERO_LAD 1 // Enable TPL in loop to work with zero LAD
#define TUNE_TPL 1   // Tuning TPL algorithm for QP assignment
#endif
// END  svt-03 /////////////////////////////////////////////////////////

//FOR DEBUGGING - Do not remove
#define NO_ENCDEC         0 // bypass encDec to test cmpliance of MD. complained achieved when skip_flag is OFF. Port sample code from VCI-SW_AV1_Candidate1 branch

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // EbDebugMacros_h
