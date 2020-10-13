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

#define FIX_RC_BUG 1 // Fix the one pass QP assignment using frames_to_be_encoded
#define FIX_VBR_BUG 1 // Fix 1st pass bug (bug from rebasing the branch)
#define FIX_10BIT     1 // fix 1st pass for 10bit input
#define FIX_RC_TOKEN     1 // fix RC token check to include double dash

#define FIX_PAD_CHROMA_AFTER_MCTF     1 // Padding chroma after altref
#define FEATURE_NEW_DELAY             1 // Change delay some sorts of I in PicDecision
#if FEATURE_NEW_DELAY
#define FIX_LAD_DEADLOCK              1 // Fix deadlock when lad>0 + iperiod>0
#endif
#define FEATURE_INL_ME                1 //Enable in-loop ME
#if FEATURE_INL_ME
#define TUNE_IME_REUSE_TPL_RESULT     1 // Reuse TPL results for iLoopME
#define TUNE_INL_TPL_ENHANCEMENT      1 // Refinement for TPL
#define TUNE_INL_TPL_ME_DBG_MSG       0 // Turn off debug message
#define TUNE_INL_ME_RECON_INPUT       1 // Perform ME on input/recon: 1 on input, 0 on recon
#if TUNE_INL_ME_RECON_INPUT
#define TUNE_INL_ME_ON_INPUT          1 // Perform ME on input
#define TUNE_INL_GM_ON_INPUT          1 // Perform GM on input
#define TUNE_INL_TPL_ON_INPUT         1 // Perform TPL on input
#define TUNE_INL_ME_MEM_OPT           1 // Optimize memory usage when perform ME on input, only use 8bit luma
#define TUNE_INL_ME_DECODE_ORDER      1 // Force decode order for inloopME
#endif
#if !TUNE_IME_REUSE_TPL_RESULT
#define TUNE_SIGNAL_TPL_ME_OQ         1 // A separate signal_xxx_oq for TPL ME
#endif
#endif


#define FEATURE_IN_LOOP_TPL 1 // Moving TPL to in loop
#if FEATURE_IN_LOOP_TPL
#define ENABLE_TPL_ZERO_LAD 1 // Enable TPL in loop to work with zero LAD
#define TUNE_TPL 1   // Tuning TPL algorithm for QP assignment
#define ENABLE_TPL_TRAILING 1 //enable trailing pictures for TPL
#define FIX_TPL_TRAILING_FRAME_BUG          1 // fix bug related to ENABLE_TPL_TRAILING
#define FIX_GM_BUG                                   1 // FIX GM r2r difference

#endif


#define FEATURE_MDS2 1 // TXT @ MDS2 if CLASS_0_3, and TXS/RDOQ @ MDS2 if CLASS_1_2
#define FEATURE_NIC_SCALING_PER_STAGE            1 // Add ability to scale NICs per stage; improve current trade-offs
#define TUNE_NICS                                1 // Tune settings for NIC scaling/pruning/# of stages to improve trade-offs with new scaling
#define PARTIAL_FREQUENCY                        1 //Calculate partial frequency transforms N2 or N4
#define TUNE_SC_QPS_IMP                          1 // Improve QP assignment for SC
#define FEATURE_REMOVE_CIRCULAR                  1 // Remove circular actions from current NSQ feautres; replace them with non-circular levels
#define FEATURE_NEW_INTER_COMP_LEVELS            1 // Add new levels and controls for inter compound; remove old levels
#define FEATURE_NEW_OBMC_LEVELS                  1 // Add new levels and controls for OBMC
#define TUNE_CDF                                 1 // Update CDF Levels
#define TUNE_TX_TYPE_LEVELS                      1 // Add Tx Type Grouping Levels
#define TUNE_INIT_FAST_LOOP_OPT                  0 // Fast loop optimizations
#define TUNE_REMOVE_UNUSED_NEIG_ARRAY            1 // Removes unused neighbor array
#define INIT_BLOCK_OPT                           1 // optimize block initialization
#define ME_IDX_LUPT                              1 // get index using lookuptable
#define REFACTOR_MD_BLOCK_LOOP                   0 // Refactor the loop that iterates over all blocks at MD
#define FEATURE_INTER_INTRA_LEVELS               1 // Cleanup and modify inter-intra levels
#define TUNE_QPS_QPM                             1 // Improve the QPS settings for Keyframe. Improve QPM for nonI base frames
#define TUNE_CDEF_FILTER                         1 // Added new fast search for CDEF
#define FIX_ME_IDX_LUPT                          1 // bug fix stops encoder from deadlocking on >=360p clips
#define FIX_OPTIMIZE_BUILD_QUANTIZER                 1 // Optimize eb_av1_build_quantizer():  called for each single frame (while the generated data does not change per frame). Moved buffer to sps, and performed 1 @ 1st frame only.
#define FIX_REMOVE_UNUSED_CODE                       1 // Remove unused code

//FOR DEBUGGING - Do not remove
#define NO_ENCDEC         0 // bypass encDec to test cmpliance of MD. complained achieved when skip_flag is OFF. Port sample code from VCI-SW_AV1_Candidate1 branch

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // EbDebugMacros_h
