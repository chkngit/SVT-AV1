/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbCodingLoop_h
#define EbCodingLoop_h

#include "EbCodingUnit.h"
#include "EbSequenceControlSet.h"
#include "EbModeDecisionProcess.h"
#include "EbEncDecProcess.h"

#ifdef __cplusplus
extern "C" {
#endif
/*******************************************
     * ModeDecisionSb
     *   performs CL (SB)
     *******************************************/
extern EbErrorType mode_decision_sb(SequenceControlSet *scs_ptr, PictureControlSet *pcs_ptr,
                                    const MdcSbData *const mdcResultTbPtr, SuperBlock *sb_ptr,
                                    uint16_t sb_origin_x, uint16_t sb_origin_y, uint32_t sb_addr,
                                    ModeDecisionContext *context_ptr);

uint8_t get_skip_tx_search_flag(int32_t sq_size, uint64_t ref_fast_cost, uint64_t cu_cost,
                                uint64_t weight);

extern void av1_encode_pass(SequenceControlSet *scs_ptr, PictureControlSet *pcs_ptr,
    SuperBlock *sb_ptr, uint32_t sb_addr, uint32_t sb_origin_x,
    uint32_t sb_origin_y, EncDecContext *context_ptr);
#if NO_ENCDEC
void no_enc_dec_pass(SequenceControlSet *scs_ptr, PictureControlSet *pcs_ptr, SuperBlock *sb_ptr,
                     uint32_t sb_addr, uint32_t sb_origin_x, uint32_t sb_origin_y, uint32_t sb_qp,
                     EncDecContext *context_ptr);
#endif

void store16bit_input_src(EbPictureBufferDesc *input_sample16bit_buffer, PictureControlSet *pcs_ptr,
                          uint32_t sb_x, uint32_t sb_y, uint32_t sb_w, uint32_t sb_h);

void residual_kernel(uint8_t *input, uint32_t input_offset, uint32_t input_stride, uint8_t *pred,
                     uint32_t pred_offset, uint32_t pred_stride, int16_t *residual,
                     uint32_t residual_offset, uint32_t residual_stride, EbBool hbd,
                     uint32_t area_width, uint32_t area_height);
#if ADAPTIVE_NSQ_CR
static const uint16_t block_prob_tab[5][9][3][2] = {
{
{{ 75 , 75 },{ 43 , 43 },{ 17 , 17 }},
{{ 8 , 9 },{ 29 , 29 },{ 17 , 17 }},
{{ 6 , 7 },{ 7 , 7 },{ 17 , 17 }},
{{ 2 , 2 },{ 5 , 6 },{ 0 , 0 }},
{{ 3 , 3 },{ 4 , 5 },{ 31 , 33 }},
{{ 1 , 1 },{ 2 , 2 },{ 0 , 0 }},
{{ 2 , 2 },{ 7 , 8 },{ 14 , 17 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }}
},
{
{{ 60 , 60 },{ 44 , 44 },{ 4 , 4 }},
{{ 5 , 5 },{ 1 , 1 },{ 3 , 3 }},
{{ 9 , 10 },{ 17 , 17 },{ 7 , 7 }},
{{ 2 , 2 },{ 2 , 2 },{ 5 , 5 }},
{{ 3 , 3 },{ 2 , 2 },{ 5 , 5 }},
{{ 2 , 2 },{ 1 , 1 },{ 4 , 5 }},
{{ 2 , 2 },{ 2 , 2 },{ 4 , 5 }},
{{ 4 , 5 },{ 5 , 6 },{ 41 , 51 }},
{{ 10 , 11 },{ 21 , 26 },{ 9 , 13 }}
},
{
{{ 59 , 59 },{ 14 , 14 },{ 19 , 19 }},
{{ 6 , 7 },{ 11 , 11 },{ 11 , 11 }},
{{ 7 , 8 },{ 7 , 7 },{ 8 , 8 }},
{{ 3 , 3 },{ 9 , 10 },{ 8 , 9 }},
{{ 3 , 3 },{ 11 , 11 },{ 11 , 11 }},
{{ 4 , 4 },{ 7 , 8 },{ 7 , 8 }},
{{ 4 , 4 },{ 8 , 9 },{ 11 , 12 }},
{{ 4 , 4 },{ 15 , 18 },{ 10 , 12 }},
{{ 6 , 7 },{ 11 , 13 },{ 8 , 10 }}
},
{
{{ 65 , 65 },{ 13 , 13 },{ 15 , 15 }},
{{ 8 , 10 },{ 16 , 17 },{ 15 , 16 }},
{{ 10 , 12 },{ 17 , 18 },{ 15 , 16 }},
{{ 3 , 3 },{ 8 , 9 },{ 8 , 9 }},
{{ 3 , 3 },{ 10 , 10 },{ 11 , 12 }},
{{ 3 , 3 },{ 10 , 11 },{ 9 , 10 }},
{{ 3 , 3 },{ 11 , 11 },{ 12 , 12 }},
{{ 1 , 1 },{ 5 , 5 },{ 5 , 5 }},
{{ 1 , 1 },{ 7 , 7 },{ 5 , 5 }}
},
{
{{ 87 , 87 },{ 54 , 54 },{ 59 , 59 }},
{{ 4 , 6 },{ 13 , 21 },{ 11 , 18 }},
{{ 6 , 7 },{ 21 , 26 },{ 18 , 22 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }}
}
};
static const uint16_t block_prob_intra_tab[5][9][3][2] = {
{
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }}
},
{
{{ 149 , 0 },{ 19 , 0 },{ 6 , 0 }},
{{ 5 , 29 },{ 1 , 7 },{ 0 , 3 }},
{{ 4 , 23 },{ 1 , 6 },{ 1 , 3 }},
{{ 0 , 2 },{ 0 , 1 },{ 0 , 0 }},
{{ 0 , 3 },{ 0 , 1 },{ 0 , 0 }},
{{ 0 , 2 },{ 0 , 1 },{ 0 , 0 }},
{{ 0 , 3 },{ 0 , 1 },{ 0 , 0 }},
{{ 0 , 12 },{ 0 , 7 },{ 0 , 3 }},
{{ 0 , 13 },{ 0 , 6 },{ 0 , 1 }}
},
{
{{ 165 , 17 },{ 60 , 3 },{ 1 , 0 }},
{{ 5 , 54 },{ 3 , 28 },{ 0 , 0 }},
{{ 5 , 52 },{ 3 , 21 },{ 0 , 0 }},
{{ 0 , 5 },{ 0 , 2 },{ 0 , 0 }},
{{ 1 , 6 },{ 0 , 2 },{ 0 , 0 }},
{{ 0 , 5 },{ 0 , 3 },{ 0 , 0 }},
{{ 1 , 6 },{ 0 , 2 },{ 0 , 0 }},
{{ 0 , 20 },{ 0 , 20 },{ 0 , 0 }},
{{ 0 , 19 },{ 0 , 12 },{ 0 , 0 }}
},
{
{{ 139 , 17 },{ 143 , 8 },{ 5 , 0 }},
{{ 3 , 38 },{ 10 , 115 },{ 1 , 4 }},
{{ 3 , 33 },{ 9 , 92 },{ 0 , 3 }},
{{ 0 , 2 },{ 1 , 11 },{ 0 , 0 }},
{{ 0 , 1 },{ 1 , 11 },{ 0 , 0 }},
{{ 0 , 2 },{ 1 , 11 },{ 0 , 0 }},
{{ 0 , 1 },{ 1 , 10 },{ 0 , 0 }},
{{ 0 , 6 },{ 0 , 59 },{ 0 , 5 }},
{{ 0 , 5 },{ 0 , 43 },{ 0 , 3 }}
},
{
{{ 25 , 4 },{ 96 , 8 },{ 20 , 1 }},
{{ 0 , 2 },{ 2 , 38 },{ 0 , 8 }},
{{ 0 , 1 },{ 2 , 34 },{ 0 , 7 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }},
{{ 0 , 0 },{ 0 , 0 },{ 0 , 0 }}
}
};
#else
#if COEFF_BASED_BYPASS_NSQ
#if MERGED_COEFF_BAND
#if SSE_BASED_SPLITTING
static const uint64_t sse_grad_weight[5][9][3] = {
{
{ 0,0,0},
{ 12,0,0},
{ 9,1,2},
{ 11,13,6},
{ 10,13,7},
{ 11,14,12},
{ 11,16,16},
{ 11,16,19},
{ 13,27,38},
},
{
{ 0,0,0},
{ 10,5,0},
{ 8,1,5},
{ 10,8,7},
{ 10,9,10},
{ 11,12,14},
{ 11,14,17},
{ 12,17,19},
{ 12,19,27},
},
{
{ 0,0,0},
{ 11,0,0},
{ 10,3,3},
{ 11,6,7},
{ 11,3,4},
{ 11,10,11},
{ 10,9,9},
{ 10,16,18},
{ 9,16,18},
},
{
{ 0,0,0},
{ 22,8,7},
{ 16,6,6},
{ 10,10,11},
{ 8,5,5},
{ 6,5,7},
{ 4,0,1},
{ 3,5,6},
{ 2,4,5},
},
{
{ 0,0,0},
{ 39,38,38},
{ 20,20,20},
{ 0,0,0},
{ 0,0,0},
{ 0,0,0},
{ 0,0,0},
{ 0,0,0},
{ 0,0,0}
}
};
#endif
static const uint64_t allowed_part_weight[5][9][3] = {
{
{ 75,43,17},
{ 9,29,17},
{ 7,7,17},
{ 2,6,0},
{ 3,5,33},
{ 1,2,0},
{ 2,8,17},
{ 0,0,0},
{ 0,0,0}
},
{
{ 60,44,4},
{ 5,1,3},
{ 10,17,7},
{ 2,2,5},
{ 3,2,5},
{ 2,1,5},
{ 2,2,5},
{ 5,6,51},
{ 11,26,13}
},
{
{ 59,14,19},
{ 7,11,11},
{ 8,7,8},
{ 3,10,9},
{ 3,11,11},
{ 4,8,8},
{ 4,9,12},
{ 4,18,12},
{ 7,13,10}
},
{
{ 65,13,15},
{ 10,17,16},
{ 12,18,16},
{ 3,9,9},
{ 3,10,12},
{ 3,11,10},
{ 3,11,12},
{ 1,5,5},
{ 1,7,5}
},
{
{ 87,54,59},
{ 6,21,18},
{ 7,26,22},
{ 0,0,0},
{ 0,0,0},
{ 0,0,0},
{ 0,0,0},
{ 0,0,0},
{ 0,0,0}
}
};
#endif
static const uint64_t allowed_part_weight_1080p[5][9][10] = {
{
{ 80, 38, 0, 0, 0, 0, 0, 0, 0, 0},
{ 8, 13, 0, 0, 0, 0, 0, 0, 0, 0},
{ 6, 2, 0, 0, 0, 0, 0, 0, 0, 0},
{ 2, 10, 0, 0, 0, 0, 0, 0, 0, 0},
{ 2, 19, 0, 0, 0, 0, 0, 0, 0, 0},
{ 1, 4, 0, 0, 0, 0, 0, 0, 0, 0},
{ 2, 13, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
},
{
{ 58, 14, 22, 10, 5, 0, 0, 0, 0, 0},
{ 9, 13, 11, 10, 8, 0, 0, 0, 0, 0},
{ 9, 7, 7, 19, 27, 0, 0, 0, 0, 0},
{ 3, 7, 8, 5, 6, 0, 0, 0, 0, 0},
{ 3, 7, 9, 6, 7, 0, 0, 0, 0, 0},
{ 3, 4, 6, 12, 12, 0, 0, 0, 0, 0},
{ 3, 5, 7, 11, 15, 0, 0, 0, 0, 0},
{ 6, 26, 21, 22, 16, 0, 0, 0, 0, 0},
{ 7, 17, 9, 5, 3, 0, 0, 0, 0, 0}
},
{
{ 52, 16, 16, 19, 28, 31, 34, 25, 18, 13},
{ 9, 10, 11, 9, 8, 8, 7, 7, 7, 7},
{ 10, 8, 6, 7, 9, 6, 6, 6, 6, 6},
{ 5, 11, 11, 12, 10, 10, 9, 11, 10, 6},
{ 5, 11, 11, 12, 10, 10, 9, 10, 13, 17},
{ 5, 9, 9, 9, 8, 8, 8, 8, 9, 5},
{ 5, 10, 9, 9, 9, 7, 8, 9, 10, 15},
{ 4, 13, 16, 15, 12, 14, 13, 15, 16, 18},
{ 6, 12, 10, 8, 7, 5, 7, 8, 10, 13}
},
{
{ 63, 26, 22, 19, 17, 17, 15, 11, 8, 8},
{ 12, 18, 19, 17, 16, 16, 16, 17, 17, 15},
{ 12, 20, 15, 14, 13, 11, 10, 8, 7, 8},
{ 3, 8, 10, 11, 12, 12, 13, 16, 17, 15},
{ 2, 7, 9, 11, 12, 12, 14, 15, 18, 19},
{ 3, 8, 9, 10, 10, 10, 10, 10, 10, 10},
{ 3, 8, 9, 10, 10, 10, 11, 11, 11, 13},
{ 1, 3, 3, 4, 6, 7, 8, 9, 10, 9},
{ 1, 3, 3, 4, 4, 4, 3, 3, 2, 3}
},
{
{ 91, 73, 66, 63, 59, 62, 64, 67, 71, 71},
{ 5, 13, 16, 22, 19, 21, 21, 18, 17, 17},
{ 4, 13, 18, 15, 22, 17, 16, 14, 12, 12},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
}
};
static const uint64_t allowed_part_weight_720p[5][9][10] = {
{
{ 78, 30, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 6, 6, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 12, 55, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 1, 9, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
},
{
{ 54, 6, 10, 0, 1, 0, 0, 0, 0, 0 },
{ 9, 14, 13, 21, 13, 0, 0, 0, 0, 0 },
{ 9, 5, 5, 20, 25, 0, 0, 0, 0, 0 },
{ 3, 5, 7, 5, 2, 0, 0, 0, 0, 0 },
{ 3, 5, 8, 3, 4, 0, 0, 0, 0, 0 },
{ 3, 3, 4, 1, 1, 0, 0, 0, 0, 0 },
{ 3, 3, 5, 1, 1, 0, 0, 0, 0, 0 },
{ 7, 41, 37, 42, 48, 0, 0, 0, 0, 0 },
{ 8, 18, 12, 6, 6, 0, 0, 0, 0, 0 }
},
{
{ 54, 14, 13, 13, 14, 14, 13, 13, 14, 14 },
{ 9, 11, 13, 12, 12, 13, 10, 9, 9, 10 },
{ 10, 7, 6, 6, 9, 6, 7, 8, 9, 10 },
{ 4, 11, 11, 10, 9, 10, 11, 11, 8, 5 },
{ 4, 11, 10, 9, 7, 10, 10, 11, 12, 14 },
{ 4, 9, 7, 8, 7, 7, 7, 7, 8, 6 },
{ 4, 9, 7, 7, 7, 6, 7, 8, 10, 13 },
{ 5, 15, 22, 22, 22, 26, 24, 21, 18, 16 },
{ 7, 14, 12, 14, 15, 9, 10, 11, 11, 14 }
},
{
{ 66, 21, 14, 10, 9, 10, 9, 10, 10, 10 },
{ 11, 18, 19, 17, 17, 17, 15, 14, 13, 12 },
{ 11, 21, 16, 15, 14, 12, 12, 12, 12, 12 },
{ 2, 8, 10, 11, 11, 12, 12, 13, 12, 12 },
{ 2, 7, 9, 10, 11, 12, 13, 12, 13, 17 },
{ 3, 8, 10, 11, 11, 11, 11, 12, 12, 11 },
{ 2, 8, 9, 10, 11, 10, 11, 11, 12, 17 },
{ 1, 4, 6, 7, 8, 9, 9, 8, 7, 4 },
{ 1, 6, 7, 8, 9, 8, 8, 9, 8, 5 }
},
{
{ 88, 66, 59, 54, 50, 48, 46, 45, 47, 51 },
{ 6, 16, 18, 25, 21, 26, 26, 25, 23, 23 },
{ 6, 18, 23, 21, 29, 26, 28, 30, 30, 26 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
},
};
static const uint64_t allowed_part_weight_360p[5][9][10] = {
{
{ 80, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 8, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 2, 100, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
},
{
{ 59, 42, 2, 1, 2, 0, 0, 0, 0, 0 },
{ 7, 2, 15, 12, 11, 0, 0, 0, 0, 0 },
{ 10, 18, 6, 8, 6, 0, 0, 0, 0, 0 },
{ 2, 1, 5, 6, 2, 0, 0, 0, 0, 0 },
{ 3, 1, 3, 4, 5, 0, 0, 0, 0, 0 },
{ 2, 1, 4, 3, 4, 0, 0, 0, 0, 0 },
{ 2, 1, 3, 3, 5, 0, 0, 0, 0, 0 },
{ 6, 11, 53, 57, 58, 0, 0, 0, 0, 0 },
{ 10, 23, 9, 5, 8, 0, 0, 0, 0, 0 }
},
{
{ 54, 11, 9, 9, 12, 15, 12, 14, 18, 19 },
{ 8, 9, 10, 10, 11, 15, 13, 11, 10, 10 },
{ 8, 7, 6, 6, 8, 6, 6, 6, 7, 10 },
{ 4, 12, 12, 12, 11, 9, 10, 8, 9, 6 },
{ 4, 11, 12, 12, 10, 10, 10, 10, 9, 13 },
{ 5, 11, 9, 9, 8, 7, 6, 6, 8, 7 },
{ 5, 10, 10, 9, 9, 6, 6, 7, 8, 14 },
{ 5, 15, 18, 21, 20, 26, 30, 31, 23, 11 },
{ 7, 14, 14, 12, 12, 7, 6, 6, 7, 9 }
},
{
{ 61, 16, 12, 10, 10, 14, 13, 14, 14, 16 },
{ 11, 18, 18, 17, 17, 17, 16, 16, 15, 15 },
{ 13, 22, 18, 17, 16, 15, 15, 15, 15, 17 },
{ 3, 8, 10, 10, 10, 10, 11, 10, 11, 9 },
{ 3, 8, 9, 10, 10, 10, 10, 10, 11, 10 },
{ 3, 9, 10, 11, 11, 10, 10, 10, 11, 10 },
{ 3, 9, 10, 11, 11, 10, 10, 11, 11, 12 },
{ 1, 4, 5, 6, 7, 7, 8, 7, 6, 4 },
{ 2, 6, 7, 8, 8, 7, 7, 7, 6, 6 }
},
{
{ 91, 66, 59, 54, 51, 51, 50, 51, 51, 56 },
{ 5, 17, 19, 24, 21, 24, 24, 23, 22, 20 },
{ 4, 18, 23, 22, 28, 25, 26, 26, 27, 24 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
}
};
static const uint64_t allowed_part_weight_240pF[5][9][10] = {
{
{ 75, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 9, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 8, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 3, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 3, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
},
{
{ 61, 42, 3, 10, 5, 0, 0, 0, 0, 0},
{ 5, 2, 2, 2, 7, 0, 0, 0, 0, 0},
{ 10, 18, 5, 10, 5, 0, 0, 0, 0, 0},
{ 2, 1, 4, 2, 5, 0, 0, 0, 0, 0},
{ 3, 2, 5, 12, 7, 0, 0, 0, 0, 0},
{ 2, 1, 5, 5, 7, 0, 0, 0, 0, 0},
{ 2, 1, 4, 2, 7, 0, 0, 0, 0, 0},
{ 5, 9, 57, 48, 49, 0, 0, 0, 0, 0},
{ 11, 24, 16, 10, 9, 0, 0, 0, 0, 0}
},
{
{ 60, 12, 11, 9, 13, 21, 17, 14, 17, 20},
{ 7, 9, 10, 9, 9, 14, 13, 12, 10, 10},
{ 8, 8, 8, 8, 10, 7, 5, 5, 8, 8},
{ 3, 10, 10, 11, 11, 9, 11, 12, 9, 7},
{ 3, 10, 11, 12, 10, 10, 12, 11, 11, 12},
{ 4, 10, 9, 9, 7, 6, 5, 5, 6, 8},
{ 4, 10, 10, 11, 9, 6, 7, 6, 8, 10},
{ 4, 14, 17, 17, 14, 17, 23, 28, 23, 14},
{ 6, 17, 15, 15, 17, 9, 8, 7, 9, 10}
},
{
{ 65, 15, 10, 9, 10, 16, 14, 15, 14, 16},
{ 10, 17, 18, 17, 17, 18, 17, 17, 17, 16},
{ 12, 22, 19, 17, 16, 15, 15, 15, 14, 16},
{ 3, 8, 10, 10, 10, 9, 10, 10, 10, 8},
{ 2, 8, 10, 10, 10, 9, 10, 10, 11, 11},
{ 3, 10, 11, 12, 12, 10, 11, 10, 11, 10},
{ 3, 9, 11, 11, 12, 10, 11, 10, 11, 12},
{ 1, 3, 4, 5, 6, 6, 6, 6, 6, 5},
{ 1, 6, 7, 8, 8, 7, 7, 6, 6, 5}
},
{
{ 88, 63, 57, 51, 50, 50, 50, 52, 53, 60},
{ 6, 17, 18, 24, 21, 24, 22, 22, 20, 18},
{ 6, 20, 24, 25, 29, 26, 28, 27, 26, 22},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
}
};
#endif
#endif
#if ADAPTIVE_TXT_CR
static const uint8_t inter_txt_cycles_reduction_th[2/*depth*/][3/*depth refinement*/][3/*tx_size*/][2/*freq band*/][15/*tx_type*/] =
{
    { // Depth 3
        { // negative refinement
            { // tx_size <8x8
                {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1}, // [0,10]
                {0,0,0,0,0,0,0,0,0,1,0,1,1,1,1} // [10,100]
            },
            { // tx_size <16x16
                {7,9,8,6,7,5,6,7,10,11,7,8,6,7,5}, // [0,10]
                {4,5,5,4,4,4,4,4,8,7,5,6,4,4,3} // [10,100]
            },
            { // tx_size 16x16
                {4,5,4,3,4,3,3,3,6,7,5,1,1,1,1}, // [0,10]
                {1,1,1,1,1,1,1,1,2,2,1,1,1,1,0} // [10,100]
            }
        },
        { // pred depth (no refinement)
            { // tx_size <8x8
                {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // [0,10]
                {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} // [10,100]
            },
            { // tx_size <16x16
                {5,6,5,4,5,4,4,5,5,6,5,5,4,4,3}, // [0,10]
                {4,4,4,4,4,3,3,4,4,4,5,3,4,3,3} // [10,100]
            },
            { // tx_size 16x16
                {5,6,5,4,5,4,4,4,5,10,7,0,0,0,0}, // [0,10]
                {1,1,1,1,1,1,1,1,2,2,2,0,0,0,0} // [10,100]
            }
        },
        { // positive refinement
            { // tx_size <8x8
                {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // [0,10]
                {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} // [10,100]
            },
            { // tx_size <16x16
                {1,1,1,1,1,1,1,1,1,1,2,1,1,1,1}, // [0,10]
                {1,1,2,2,1,1,1,1,1,1,4,1,3,1,3} // [10,100]
            },
            { // tx_size 16x16
                {1,1,1,1,1,1,1,1,1,3,3,0,0,0,0}, // [0,10]
                {1,1,1,1,1,1,1,1,1,1,2,0,0,0,0} // [10,100]
            }
        }
    },
    { // Non-depth 3
        { // negative refinement
            { // tx_size <8x8
                {0,0,1,0,0,0,0,1,1,1,1,1,1,1,1}, // [0,10]
                {1,1,1,0,1,1,1,1,1,1,1,2,2,2,1} // [10,100]
            },
            { // tx_size <16x16
                {1,1,1,1,1,0,1,1,1,1,1,1,1,1,1}, // [0,10]
                {1,1,1,0,0,0,0,0,1,1,1,1,1,1,0} // [10,100]
            },
            { // tx_size 16x16
                {2,2,2,1,1,1,1,1,16,3,2,0,0,0,0}, // [0,10]
                {0,0,0,0,0,0,0,0,3,0,1,0,0,0,0} // [10,100]
            }
        },
        { // pred depth (no refinement)
            { // tx_size <8x8
                {4,5,21,3,7,7,13,15,4,6,5,11,10,7,7}, // [0,10]
                {4,5,12,3,6,5,9,11,6,11,8,20,18,15,14} // [10,100]
            },
            { // tx_size <16x16
                {1,2,1,1,1,1,1,1,2,2,2,2,1,1,1}, // [0,10]
                {2,2,1,1,2,1,1,1,2,3,2,2,2,2,1} // [10,100]
            },
            { // tx_size 16x16
                {1,1,1,0,0,0,0,0,5,1,1,0,0,0,0}, // [0,10]
                {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0} // [10,100]
            }
        },
        { // positive refinement
            { // tx_size <8x8
                {0,0,1,0,0,0,0,1,0,1,0,1,1,1,1}, // [0,10]
                {1,1,1,0,1,1,1,1,1,2,1,3,2,2,2} // [10,100]
            },
            { // tx_size <16x16
                {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0}, // [0,10]
                {1,1,1,1,1,0,0,1,1,1,1,1,1,1,1} // [10,100]
            },
            { // tx_size 16x16
                {0,0,0,0,0,0,0,0,2,0,0,0,0,0,0}, // [0,10]
                {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} // [10,100]
            }
        }
    }
};
static const uint8_t intra_txt_cycles_reduction_th[2/*depth*/][3/*depth refinement*/][3/*tx_size*/][2/*freq band*/][15/*tx_type*/] =
{
    { // Depth 3
        { // negative refinement
            { // tx_size <8x8
                {3,3,5,0,0,0,0,0,0,2,2,0,0,0,0}, // [0,10]
                {2,2,4,0,0,0,0,0,0,1,1,0,0,0,0} // [10,100]
            },
            { // tx_size <16x16
                {16,18,20,0,1,0,0,0,2,6,6,0,0,0,0}, // [0,10]
                {7,8,8,0,0,0,0,0,1,2,2,0,0,0,0} // [10,100]
            },
            { // tx_size 16x16
                {11,13,15,0,0,0,0,0,1,3,3,0,0,0,0}, // [0,10]
                {5,6,8,0,0,0,0,0,0,2,2,0,0,0,0} // [10,100]
            }
        },
        { // pred depth (no refinement)
            { // tx_size <8x8
                {1,2,3,0,0,0,0,0,0,1,1,0,0,0,0}, // [0,10]
                {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0} // [10,100]
            },
            { // tx_size <16x16
                {15,17,16,0,0,0,0,0,0,4,5,0,0,0,0}, // [0,10]
                {5,6,6,0,0,0,0,0,0,1,2,0,0,0,0} // [10,100]
            },
            { // tx_size 16x16
                {17,18,19,0,0,0,0,0,0,2,3,0,0,0,0}, // [0,10]
                {5,6,7,0,0,0,0,0,0,1,3,0,0,0,0} // [10,100]
            }
        },
        { // positive refinement
            { // tx_size <8x8
                {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0}, // [0,10]
                {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} // [10,100]
            },
            { // tx_size <16x16
                {4,5,5,0,0,0,0,0,0,1,2,0,0,0,0}, // [0,10]
                {1,2,2,0,0,0,0,0,0,0,1,0,0,0,0} // [10,100]
            },
            { // tx_size 16x16
                {4,5,5,0,0,0,0,0,0,0,1,0,0,0,0}, // [0,10]
                {2,2,2,0,0,0,0,0,0,0,2,0,0,0,0} // [10,100]
            }
        }
    },
    { // Non-depth 3
        { // negative refinement
            { // tx_size <8x8
                {3,4,5,0,0,0,0,0,0,2,2,0,0,0,0}, // [0,10]
                {3,4,6,0,0,0,0,0,0,2,2,0,0,0,0} // [10,100]
            },
            { // tx_size <16x16
                {3,3,3,0,0,0,0,0,0,1,1,0,0,0,0}, // [0,10]
                {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0} // [10,100]
            },
            { // tx_size 16x16
                {8,9,11,0,0,0,0,0,1,0,0,0,0,0,0}, // [0,10]
                {0,1,1,0,0,0,0,0,0,0,0,0,0,0,0} // [10,100]
            }
        },
        { // pred depth (no refinement)
            { // tx_size <8x8
                {37,42,78,0,0,0,0,0,2,18,20,0,0,0,0}, // [0,10]
                {30,33,60,0,0,0,0,0,1,17,18,0,0,0,0} // [10,100]
            },
            { // tx_size <16x16
                {5,6,6,0,0,0,0,0,0,2,3,0,0,0,0}, // [0,10]
                {4,4,5,0,0,0,0,0,0,2,2,0,0,0,0} // [10,100]
            },
            { // tx_size 16x16
                {4,4,4,0,0,0,0,0,0,0,0,0,0,0,0}, // [0,10]
                {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} // [10,100]
            }
        },
        { // positive refinement
            { // tx_size <8x8
                {7,8,13,0,0,0,0,0,0,4,5,0,0,0,0}, // [0,10]
                {7,7,13,0,0,0,0,0,0,4,5,0,0,0,0} // [10,100]
            },
            { // tx_size <16x16
                {3,3,3,0,0,0,0,0,0,1,1,0,0,0,0}, // [0,10]
                {2,2,2,0,0,0,0,0,0,0,1,0,0,0,0} // [10,100]
            },
            { // tx_size 16x16
                {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0}, // [0,10]
                {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} // [10,100]
            }
        }
    }
};
#endif
#if SOFT_CYCLES_REDUCTION
static const uint32_t intra_adaptive_md_cycles_reduction_th[DEPTH_DELTA_NUM][NUMBER_OF_SHAPES - 1] = {
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
{630 ,453 ,303 ,99 ,78 ,17 ,17 ,672 ,85},
{1552 ,606 ,202 ,28 ,35 ,5 ,11 ,461 ,58},
{1875 ,962 ,222 ,144 ,171 ,5 ,17 ,1272 ,15},
{3 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
};
#endif
#ifdef __cplusplus
}
#endif
#endif // EbCodingLoop_h
