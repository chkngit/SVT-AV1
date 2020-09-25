/*
* Copyright(c) 2019 Intel Corporation
* Copyright (c) 2016, Alliance for Open Media. All rights reserved
*
* This source code is subject to the terms of the BSD 2 Clause License and
* the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
* was not distributed with this source code in the LICENSE file, you can
* obtain it at https://www.aomedia.org/license/software-license. If the Alliance for Open
* Media Patent License 1.0 was not distributed with this source code in the
* PATENTS file, you can obtain it at https://www.aomedia.org/license/patent-license.
*/

#include <stdlib.h>
#include "aom_dsp_rtcd.h"
#include "EbDefinitions.h"
#include "EbEncHandle.h"
#include "EbCdefProcess.h"
#include "EbEncDecResults.h"
#include "EbThreads.h"
#include "EbReferenceObject.h"
#include "EbEncCdef.h"
#include "EbEncDecProcess.h"
#include "EbPictureBufferDesc.h"
#include "EbSequenceControlSet.h"
#include "EbUtility.h"
#include "EbPictureControlSet.h"

void copy_sb8_16(uint16_t *dst, int32_t dstride, const uint8_t *src, int32_t src_voffset,
                 int32_t src_hoffset, int32_t sstride, int32_t vsize, int32_t hsize);

void *  eb_aom_memalign(size_t align, size_t size);
void    eb_aom_free(void *memblk);
void *  eb_aom_malloc(size_t size);
int32_t eb_sb_all_skip(PictureControlSet *pcs_ptr, const Av1Common *const cm, int32_t mi_row,
                       int32_t mi_col);
int32_t eb_sb_compute_cdef_list(PictureControlSet *pcs_ptr, const Av1Common *const cm,
                                int32_t mi_row, int32_t mi_col, CdefList *dlist, BlockSize bs);
void    finish_cdef_search(EncDecContext *context_ptr, PictureControlSet *pcs_ptr,
                           int32_t selected_strength_cnt[64]);
void    av1_cdef_frame16bit(EncDecContext *context_ptr, SequenceControlSet *scs_ptr,
                            PictureControlSet *pCs);
void    eb_av1_cdef_frame(EncDecContext *context_ptr, SequenceControlSet *scs_ptr,
                          PictureControlSet *pCs);
void    eb_av1_loop_restoration_save_boundary_lines(const Yv12BufferConfig *frame, Av1Common *cm,
                                                    int32_t after_cdef);

/**************************************
 * Cdef Context
 **************************************/
typedef struct CdefContext {
    EbFifo *cdef_input_fifo_ptr;
    EbFifo *cdef_output_fifo_ptr;
} CdefContext;

static void cdef_context_dctor(EbPtr p) {
    EbThreadContext *thread_context_ptr = (EbThreadContext *)p;
    CdefContext *    obj                = (CdefContext *)thread_context_ptr->priv;
    EB_FREE_ARRAY(obj);
}

/******************************************************
 * Cdef Context Constructor
 ******************************************************/
EbErrorType cdef_context_ctor(EbThreadContext *  thread_context_ptr,
                              const EbEncHandle *enc_handle_ptr, int index) {
    CdefContext *context_ptr;
    EB_CALLOC_ARRAY(context_ptr, 1);
    thread_context_ptr->priv  = context_ptr;
    thread_context_ptr->dctor = cdef_context_dctor;

    // Input/Output System Resource Manager FIFOs
    context_ptr->cdef_input_fifo_ptr =
        eb_system_resource_get_consumer_fifo(enc_handle_ptr->dlf_results_resource_ptr, index);
    context_ptr->cdef_output_fifo_ptr =
        eb_system_resource_get_producer_fifo(enc_handle_ptr->cdef_results_resource_ptr, index);

    return EB_ErrorNone;
}

void cdef_seg_search(PictureControlSet *pcs_ptr, SequenceControlSet *scs_ptr,
                     uint32_t segment_index) {
    struct PictureParentControlSet *ppcs    = pcs_ptr->parent_pcs_ptr;
    FrameHeader *                   frm_hdr = &ppcs->frm_hdr;
    Av1Common *                     cm      = pcs_ptr->parent_pcs_ptr->av1_cm;
    uint32_t                        x_seg_idx;
    uint32_t                        y_seg_idx;
    uint32_t picture_width_in_b64  = (pcs_ptr->parent_pcs_ptr->aligned_width + 64 - 1) / 64;
    uint32_t picture_height_in_b64 = (pcs_ptr->parent_pcs_ptr->aligned_height + 64 - 1) / 64;
    SEGMENT_CONVERT_IDX_TO_XY(
        segment_index, x_seg_idx, y_seg_idx, pcs_ptr->cdef_segments_column_count);
    uint32_t x_b64_start_idx =
        SEGMENT_START_IDX(x_seg_idx, picture_width_in_b64, pcs_ptr->cdef_segments_column_count);
    uint32_t x_b64_end_idx =
        SEGMENT_END_IDX(x_seg_idx, picture_width_in_b64, pcs_ptr->cdef_segments_column_count);
    uint32_t y_b64_start_idx =
        SEGMENT_START_IDX(y_seg_idx, picture_height_in_b64, pcs_ptr->cdef_segments_row_count);
    uint32_t y_b64_end_idx =
        SEGMENT_END_IDX(y_seg_idx, picture_height_in_b64, pcs_ptr->cdef_segments_row_count);

    int32_t mi_rows = ppcs->av1_cm->mi_rows;
    int32_t mi_cols = ppcs->av1_cm->mi_cols;

    uint32_t fbr, fbc;
    uint8_t *src[3];
    uint8_t *ref_coeff[3];
    CdefList dlist[MI_SIZE_128X128 * MI_SIZE_128X128];
    int32_t  dir[CDEF_NBLOCKS][CDEF_NBLOCKS] = {{0}};
    int32_t  var[CDEF_NBLOCKS][CDEF_NBLOCKS] = {{0}};
    int32_t  stride_src[3];
    int32_t  stride_ref[3];
    int32_t  bsize[3];
    int32_t  mi_wide_l2[3];
    int32_t  mi_high_l2[3];
    int32_t  xdec[3];
    int32_t  ydec[3];
    int32_t  pli;
    int32_t  cdef_count;
    int32_t  coeff_shift = AOMMAX(scs_ptr->static_config.encoder_bit_depth - 8, 0);
    int32_t  nvfb        = (mi_rows + MI_SIZE_64X64 - 1) / MI_SIZE_64X64;
    int32_t  nhfb        = (mi_cols + MI_SIZE_64X64 - 1) / MI_SIZE_64X64;
    int32_t  pri_damping = 3 + (frm_hdr->quantization_params.base_q_idx >> 6);
    int32_t  sec_damping = pri_damping;

    const int32_t num_planes      = 3;
    const int32_t total_strengths = TOTAL_STRENGTHS;
    DECLARE_ALIGNED(32, uint16_t, inbuf[CDEF_INBUF_SIZE]);
    uint16_t *in;
    DECLARE_ALIGNED(32, uint8_t, tmp_dst[1 << (MAX_SB_SIZE_LOG2 * 2)]);

    int32_t gi_step;
    int32_t mid_gi;
    int32_t start_gi;
    int32_t end_gi;
#if CDEF_OPT
    CDEF_PICK_METHOD pick_method = pcs_ptr->parent_pcs_ptr->cdef_level == 6 ? CDEF_FAST_SEARCH_LVL1
                                 : pcs_ptr->parent_pcs_ptr->cdef_level == 7 ?  CDEF_FAST_SEARCH_LVL2 
                                 : pcs_ptr->parent_pcs_ptr->cdef_level == 8 ?  CDEF_FAST_SEARCH_LVL3 : 0;
#endif

    EbPictureBufferDesc *input_picture_ptr =
        (EbPictureBufferDesc *)pcs_ptr->parent_pcs_ptr->enhanced_picture_ptr;
    EbPictureBufferDesc *recon_picture_ptr;
    if (pcs_ptr->parent_pcs_ptr->is_used_as_reference_flag == EB_TRUE)
        recon_picture_ptr = ((EbReferenceObject *)
                                 pcs_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)
                                ->reference_picture;
    else
        recon_picture_ptr = pcs_ptr->recon_picture_ptr;

    for (pli = 0; pli < num_planes; pli++) {
        int32_t subsampling_x = (pli == 0) ? 0 : 1;
        int32_t subsampling_y = (pli == 0) ? 0 : 1;
        xdec[pli]             = subsampling_x;
        ydec[pli]             = subsampling_y;
        bsize[pli] =
            ydec[pli] ? (xdec[pli] ? BLOCK_4X4 : BLOCK_8X4) : (xdec[pli] ? BLOCK_4X8 : BLOCK_8X8);
        mi_wide_l2[pli] = MI_SIZE_LOG2 - subsampling_x;
        mi_high_l2[pli] = MI_SIZE_LOG2 - subsampling_y;

        src[pli]       = (uint8_t *)pcs_ptr->src[pli];
        ref_coeff[pli] = (uint8_t *)pcs_ptr->ref_coeff[pli];
        stride_src[pli] =
            pli == 0 ? recon_picture_ptr->stride_y
                     : (pli == 1 ? recon_picture_ptr->stride_cb : recon_picture_ptr->stride_cr);
        stride_ref[pli] =
            pli == 0 ? input_picture_ptr->stride_y
                     : (pli == 1 ? input_picture_ptr->stride_cb : input_picture_ptr->stride_cr);
    }

    in = inbuf + CDEF_VBORDER * CDEF_BSTRIDE + CDEF_HBORDER;

    for (fbr = y_b64_start_idx; fbr < y_b64_end_idx; ++fbr) {
        for (fbc = x_b64_start_idx; fbc < x_b64_end_idx; ++fbc) {
            int32_t nvb, nhb;
            int32_t gi;
            int32_t dirinit    = 0;
            nhb                = AOMMIN(MI_SIZE_64X64, cm->mi_cols - MI_SIZE_64X64 * fbc);
            nvb                = AOMMIN(MI_SIZE_64X64, cm->mi_rows - MI_SIZE_64X64 * fbr);
            int32_t    hb_step = 1; //these should be all time with 64x64 SBs
            int32_t    vb_step = 1;
            BlockSize  bs      = BLOCK_64X64;
            ModeInfo **mi =
                pcs_ptr->mi_grid_base + MI_SIZE_64X64 * fbr * cm->mi_stride + MI_SIZE_64X64 * fbc;
            const MbModeInfo *mbmi = &mi[0]->mbmi;

            if (((fbc & 1) && (mbmi->block_mi.sb_type == BLOCK_128X128 ||
                               mbmi->block_mi.sb_type == BLOCK_128X64)) ||
                ((fbr & 1) && (mbmi->block_mi.sb_type == BLOCK_128X128 ||
                               mbmi->block_mi.sb_type == BLOCK_64X128)))
                continue;
            if (mbmi->block_mi.sb_type == BLOCK_128X128 || mbmi->block_mi.sb_type == BLOCK_128X64 ||
                mbmi->block_mi.sb_type == BLOCK_64X128)
                bs = mbmi->block_mi.sb_type;

            if (bs == BLOCK_128X128 || bs == BLOCK_128X64) {
                nhb     = AOMMIN(MI_SIZE_128X128, cm->mi_cols - MI_SIZE_64X64 * fbc);
                hb_step = 2;
            }
            if (bs == BLOCK_128X128 || bs == BLOCK_64X128) {
                nvb     = AOMMIN(MI_SIZE_128X128, cm->mi_rows - MI_SIZE_64X64 * fbr);
                vb_step = 2;
            }

            // No filtering if the entire filter block is skipped
            if (eb_sb_all_skip(pcs_ptr, cm, fbr * MI_SIZE_64X64, fbc * MI_SIZE_64X64)) continue;

            cdef_count = eb_sb_compute_cdef_list(
                pcs_ptr, cm, fbr * MI_SIZE_64X64, fbc * MI_SIZE_64X64, dlist, bs);

            for (pli = 0; pli < num_planes; pli++) {
                for (int i = 0; i < CDEF_INBUF_SIZE; i++) inbuf[i] = CDEF_VERY_LARGE;

                int32_t yoff  = CDEF_VBORDER * (fbr != 0);
                int32_t xoff  = CDEF_HBORDER * (fbc != 0);
                int32_t ysize = (nvb << mi_high_l2[pli]) +
                                CDEF_VBORDER * ((int32_t)fbr + vb_step < nvfb) + yoff;
                int32_t xsize = (nhb << mi_wide_l2[pli]) +
                                CDEF_HBORDER * ((int32_t)fbc + hb_step < nhfb) + xoff;

                copy_sb8_16(&in[(-yoff * CDEF_BSTRIDE - xoff)],
                            CDEF_BSTRIDE,
                            src[pli],
                            (fbr * MI_SIZE_64X64 << mi_high_l2[pli]) - yoff,
                            (fbc * MI_SIZE_64X64 << mi_wide_l2[pli]) - xoff,
                            stride_src[pli],
                            ysize,
                            xsize);
                gi_step = get_cdef_gi_step(ppcs->cdef_level);
                mid_gi   = ppcs->cdf_ref_frame_strength;
                start_gi = ppcs->use_ref_frame_cdef_strength && ppcs->cdef_level == 5
                               ? (AOMMAX(0, mid_gi - gi_step))
                               : 0;
#if CDEF_OPT
                end_gi = ppcs->cdef_level > 5 ? nb_cdef_strengths[pick_method] : ppcs->use_ref_frame_cdef_strength
#else
                end_gi = ppcs->use_ref_frame_cdef_strength
#endif
                             ? AOMMIN(total_strengths, mid_gi + gi_step)
                             : ppcs->cdef_level == 5 ? 8 : total_strengths;

                for (gi = start_gi; gi < end_gi; gi++) {
                    int32_t  threshold;
                    uint64_t curr_mse;
                    int32_t  sec_strength;
                    threshold = gi / CDEF_SEC_STRENGTHS;
                    /* We avoid filtering the pixels for which some of the pixels to
                    average are outside the frame. We could change the filter instead, but it would add special cases for any future vectorization. */
                    sec_strength = gi % CDEF_SEC_STRENGTHS;
#if CDEF_OPT
                    get_cdef_filter_strengths(pick_method, &threshold, &sec_strength,
                                    gi);
#endif
                    eb_cdef_filter_fb(tmp_dst,
                                      NULL,
                                      CDEF_BSTRIDE,
                                      in,
                                      xdec[pli],
                                      ydec[pli],
                                      dir,
                                      &dirinit,
                                      var,
                                      pli,
                                      dlist,
                                      cdef_count,
                                      threshold,
                                      sec_strength + (sec_strength == 3),
                                      pri_damping,
                                      sec_damping,
                                      coeff_shift);

                    curr_mse = eb_compute_cdef_dist_8bit(
                        ref_coeff[pli] +
                            (fbr * MI_SIZE_64X64 << mi_high_l2[pli]) * stride_ref[pli] +
                            (fbc * MI_SIZE_64X64 << mi_wide_l2[pli]),
                        stride_ref[pli],
                        tmp_dst,
                        dlist,
                        cdef_count,
                        (BlockSize)bsize[pli],
                        coeff_shift,
                        pli);

                    if (pli < 2)
                        pcs_ptr->mse_seg[pli][fbr * nhfb + fbc][gi] = curr_mse;
                    else
                        pcs_ptr->mse_seg[1][fbr * nhfb + fbc][gi] += curr_mse;
                }

                //if (ppcs->picture_number == 15)
                //    SVT_LOG(" bs:%i count:%i  mse:%I64i\n", bs, cdef_count,pcs_ptr->mse_seg[0][fbr*nhfb + fbc][4]);
            }
        }
    }
}
void cdef_seg_search16bit(PictureControlSet *pcs_ptr, SequenceControlSet *scs_ptr,
                          uint32_t segment_index) {
    EbPictureBufferDesc *input_pic_ptr = pcs_ptr->input_frame16bit;
    EbPictureBufferDesc *recon_pic_ptr =
        (pcs_ptr->parent_pcs_ptr->is_used_as_reference_flag == EB_TRUE)
            ? ((EbReferenceObject *)
                   pcs_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)
                  ->reference_picture16bit
            : pcs_ptr->recon_picture16bit_ptr;

    struct PictureParentControlSet *ppcs    = pcs_ptr->parent_pcs_ptr;
    FrameHeader *                   frm_hdr = &ppcs->frm_hdr;
    Av1Common *                     cm      = pcs_ptr->parent_pcs_ptr->av1_cm;
    uint32_t                        x_seg_idx;
    uint32_t                        y_seg_idx;
    uint32_t picture_width_in_b64  = (pcs_ptr->parent_pcs_ptr->aligned_width + 64 - 1) / 64;
    uint32_t picture_height_in_b64 = (pcs_ptr->parent_pcs_ptr->aligned_height + 64 - 1) / 64;
    SEGMENT_CONVERT_IDX_TO_XY(
        segment_index, x_seg_idx, y_seg_idx, pcs_ptr->cdef_segments_column_count);
    uint32_t x_b64_start_idx =
        SEGMENT_START_IDX(x_seg_idx, picture_width_in_b64, pcs_ptr->cdef_segments_column_count);
    uint32_t x_b64_end_idx =
        SEGMENT_END_IDX(x_seg_idx, picture_width_in_b64, pcs_ptr->cdef_segments_column_count);
    uint32_t y_b64_start_idx =
        SEGMENT_START_IDX(y_seg_idx, picture_height_in_b64, pcs_ptr->cdef_segments_row_count);
    uint32_t y_b64_end_idx =
        SEGMENT_END_IDX(y_seg_idx, picture_height_in_b64, pcs_ptr->cdef_segments_row_count);

    int32_t mi_rows = ppcs->av1_cm->mi_rows;
    int32_t mi_cols = ppcs->av1_cm->mi_cols;

    uint32_t  fbr, fbc;
    uint16_t *src[3];
    uint16_t *ref_coeff[3];
    CdefList  dlist[MI_SIZE_128X128 * MI_SIZE_128X128];
    int32_t   dir[CDEF_NBLOCKS][CDEF_NBLOCKS] = {{0}};
    int32_t   var[CDEF_NBLOCKS][CDEF_NBLOCKS] = {{0}};
    int32_t   stride_src[3];
    int32_t   stride_ref[3];
    int32_t   bsize[3];
    int32_t   mi_wide_l2[3];
    int32_t   mi_high_l2[3];
    int32_t   xdec[3];
    int32_t   ydec[3];
    int32_t   pli;
    int32_t   cdef_count;
    int32_t   coeff_shift = AOMMAX(scs_ptr->static_config.encoder_bit_depth - 8, 0);
    int32_t   nvfb        = (mi_rows + MI_SIZE_64X64 - 1) / MI_SIZE_64X64;
    int32_t   nhfb        = (mi_cols + MI_SIZE_64X64 - 1) / MI_SIZE_64X64;
    int32_t   pri_damping = 3 + (frm_hdr->quantization_params.base_q_idx >> 6);
    int32_t   sec_damping = pri_damping;

    const int32_t num_planes      = 3;
    const int32_t total_strengths = TOTAL_STRENGTHS;
    DECLARE_ALIGNED(32, uint16_t, inbuf[CDEF_INBUF_SIZE]);
    uint16_t *in;
    DECLARE_ALIGNED(32, uint16_t, tmp_dst[1 << (MAX_SB_SIZE_LOG2 * 2)]);
    int32_t gi_step;
    int32_t mid_gi;
    int32_t start_gi;
    int32_t end_gi;

    for (pli = 0; pli < num_planes; pli++) {
        int32_t subsampling_x = (pli == 0) ? 0 : 1;
        int32_t subsampling_y = (pli == 0) ? 0 : 1;
        xdec[pli]             = subsampling_x;
        ydec[pli]             = subsampling_y;
        bsize[pli] =
            ydec[pli] ? (xdec[pli] ? BLOCK_4X4 : BLOCK_8X4) : (xdec[pli] ? BLOCK_4X8 : BLOCK_8X8);

        mi_wide_l2[pli] = MI_SIZE_LOG2 - subsampling_x;
        mi_high_l2[pli] = MI_SIZE_LOG2 - subsampling_y;

        src[pli]        = pcs_ptr->src[pli];
        ref_coeff[pli]  = pcs_ptr->ref_coeff[pli];
        stride_src[pli] = pli == 0
                              ? recon_pic_ptr->stride_y
                              : (pli == 1 ? recon_pic_ptr->stride_cb : recon_pic_ptr->stride_cr);
        stride_ref[pli] = pli == 0
                              ? input_pic_ptr->stride_y
                              : (pli == 1 ? input_pic_ptr->stride_cb : input_pic_ptr->stride_cr);
    }

    in = inbuf + CDEF_VBORDER * CDEF_BSTRIDE + CDEF_HBORDER;

    for (fbr = y_b64_start_idx; fbr < y_b64_end_idx; ++fbr) {
        for (fbc = x_b64_start_idx; fbc < x_b64_end_idx; ++fbc) {
            int32_t nvb, nhb;
            int32_t gi;
            int32_t dirinit    = 0;
            nhb                = AOMMIN(MI_SIZE_64X64, cm->mi_cols - MI_SIZE_64X64 * fbc);
            nvb                = AOMMIN(MI_SIZE_64X64, cm->mi_rows - MI_SIZE_64X64 * fbr);
            int32_t    hb_step = 1; //these should be all time with 64x64 SBs
            int32_t    vb_step = 1;
            BlockSize  bs      = BLOCK_64X64;
            ModeInfo **mi =
                pcs_ptr->mi_grid_base + MI_SIZE_64X64 * fbr * cm->mi_stride + MI_SIZE_64X64 * fbc;
            const MbModeInfo *mbmi = &mi[0]->mbmi;

            if (((fbc & 1) && (mbmi->block_mi.sb_type == BLOCK_128X128 ||
                               mbmi->block_mi.sb_type == BLOCK_128X64)) ||
                ((fbr & 1) && (mbmi->block_mi.sb_type == BLOCK_128X128 ||
                               mbmi->block_mi.sb_type == BLOCK_64X128)))
                continue;
            if (mbmi->block_mi.sb_type == BLOCK_128X128 || mbmi->block_mi.sb_type == BLOCK_128X64 ||
                mbmi->block_mi.sb_type == BLOCK_64X128)
                bs = mbmi->block_mi.sb_type;
            if (bs == BLOCK_128X128 || bs == BLOCK_128X64) {
                nhb     = AOMMIN(MI_SIZE_128X128, cm->mi_cols - MI_SIZE_64X64 * fbc);
                hb_step = 2;
            }
            if (bs == BLOCK_128X128 || bs == BLOCK_64X128) {
                nvb     = AOMMIN(MI_SIZE_128X128, cm->mi_rows - MI_SIZE_64X64 * fbr);
                vb_step = 2;
            }

            // No filtering if the entire filter block is skipped
            if (eb_sb_all_skip(pcs_ptr, cm, fbr * MI_SIZE_64X64, fbc * MI_SIZE_64X64)) continue;

            cdef_count = eb_sb_compute_cdef_list(
                pcs_ptr, cm, fbr * MI_SIZE_64X64, fbc * MI_SIZE_64X64, dlist, bs);

            for (pli = 0; pli < num_planes; pli++) {
                for (int i = 0; i < CDEF_INBUF_SIZE; i++) inbuf[i] = CDEF_VERY_LARGE;

                int32_t yoff  = CDEF_VBORDER * (fbr != 0);
                int32_t xoff  = CDEF_HBORDER * (fbc != 0);
                int32_t ysize = (nvb << mi_high_l2[pli]) +
                                CDEF_VBORDER * ((int32_t)fbr + vb_step < nvfb) + yoff;
                int32_t xsize = (nhb << mi_wide_l2[pli]) +
                                CDEF_HBORDER * ((int32_t)fbc + hb_step < nhfb) + xoff;

                copy_sb16_16(&in[(-yoff * CDEF_BSTRIDE - xoff)],
                             CDEF_BSTRIDE,
                             src[pli],
                             (fbr * MI_SIZE_64X64 << mi_high_l2[pli]) - yoff,
                             (fbc * MI_SIZE_64X64 << mi_wide_l2[pli]) - xoff,
                             stride_src[pli],
                             ysize,
                             xsize);
                gi_step = get_cdef_gi_step(ppcs->cdef_level);
                mid_gi = ppcs->cdf_ref_frame_strength;
                start_gi = ppcs->use_ref_frame_cdef_strength && ppcs->cdef_level == 5
                                ? (AOMMAX(0, mid_gi - gi_step))
                                : 0;
                end_gi = ppcs->use_ref_frame_cdef_strength
                                ? AOMMIN(total_strengths, mid_gi + gi_step)
                                : ppcs->cdef_level == 5 ? 8 : total_strengths;

                for (gi = start_gi; gi < end_gi; gi++) {
                    int32_t  threshold;
                    uint64_t curr_mse;
                    int32_t  sec_strength;
                    threshold = gi / CDEF_SEC_STRENGTHS;
                    /* We avoid filtering the pixels for which some of the pixels to
                    average are outside the frame. We could change the filter instead, but it would add special cases for any future vectorization. */
                    sec_strength = gi % CDEF_SEC_STRENGTHS;

                    eb_cdef_filter_fb(NULL,
                                      tmp_dst,
                                      CDEF_BSTRIDE,
                                      in,
                                      xdec[pli],
                                      ydec[pli],
                                      dir,
                                      &dirinit,
                                      var,
                                      pli,
                                      dlist,
                                      cdef_count,
                                      threshold,
                                      sec_strength + (sec_strength == 3),
                                      pri_damping,
                                      sec_damping,
                                      coeff_shift);

                    curr_mse = eb_compute_cdef_dist(
                        ref_coeff[pli] +
                            (fbr * MI_SIZE_64X64 << mi_high_l2[pli]) * stride_ref[pli] +
                            (fbc * MI_SIZE_64X64 << mi_wide_l2[pli]),
                        stride_ref[pli],
                        tmp_dst,
                        dlist,
                        cdef_count,
                        (BlockSize)bsize[pli],
                        coeff_shift,
                        pli);

                    if (pli < 2)
                        pcs_ptr->mse_seg[pli][fbr * nhfb + fbc][gi] = curr_mse;
                    else
                        pcs_ptr->mse_seg[1][fbr * nhfb + fbc][gi] += curr_mse;
                }
            }
        }
    }
}
#if CDEF_OPT
static const int16_t ac_qlookup_QTX[QINDEX_RANGE] = {
  4,    8,    9,    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
  20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
  33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
  46,   47,   48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,
  59,   60,   61,   62,   63,   64,   65,   66,   67,   68,   69,   70,   71,
  72,   73,   74,   75,   76,   77,   78,   79,   80,   81,   82,   83,   84,
  85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,   96,   97,
  98,   99,   100,  101,  102,  104,  106,  108,  110,  112,  114,  116,  118,
  120,  122,  124,  126,  128,  130,  132,  134,  136,  138,  140,  142,  144,
  146,  148,  150,  152,  155,  158,  161,  164,  167,  170,  173,  176,  179,
  182,  185,  188,  191,  194,  197,  200,  203,  207,  211,  215,  219,  223,
  227,  231,  235,  239,  243,  247,  251,  255,  260,  265,  270,  275,  280,
  285,  290,  295,  300,  305,  311,  317,  323,  329,  335,  341,  347,  353,
  359,  366,  373,  380,  387,  394,  401,  408,  416,  424,  432,  440,  448,
  456,  465,  474,  483,  492,  501,  510,  520,  530,  540,  550,  560,  571,
  582,  593,  604,  615,  627,  639,  651,  663,  676,  689,  702,  715,  729,
  743,  757,  771,  786,  801,  816,  832,  848,  864,  881,  898,  915,  933,
  951,  969,  988,  1007, 1026, 1046, 1066, 1087, 1108, 1129, 1151, 1173, 1196,
  1219, 1243, 1267, 1292, 1317, 1343, 1369, 1396, 1423, 1451, 1479, 1508, 1537,
  1567, 1597, 1628, 1660, 1692, 1725, 1759, 1793, 1828,
};

static const int16_t ac_qlookup_10_QTX[QINDEX_RANGE] = {
  4,    9,    11,   13,   16,   18,   21,   24,   27,   30,   33,   37,   40,
  44,   48,   51,   55,   59,   63,   67,   71,   75,   79,   83,   88,   92,
  96,   100,  105,  109,  114,  118,  122,  127,  131,  136,  140,  145,  149,
  154,  158,  163,  168,  172,  177,  181,  186,  190,  195,  199,  204,  208,
  213,  217,  222,  226,  231,  235,  240,  244,  249,  253,  258,  262,  267,
  271,  275,  280,  284,  289,  293,  297,  302,  306,  311,  315,  319,  324,
  328,  332,  337,  341,  345,  349,  354,  358,  362,  367,  371,  375,  379,
  384,  388,  392,  396,  401,  409,  417,  425,  433,  441,  449,  458,  466,
  474,  482,  490,  498,  506,  514,  523,  531,  539,  547,  555,  563,  571,
  579,  588,  596,  604,  616,  628,  640,  652,  664,  676,  688,  700,  713,
  725,  737,  749,  761,  773,  785,  797,  809,  825,  841,  857,  873,  889,
  905,  922,  938,  954,  970,  986,  1002, 1018, 1038, 1058, 1078, 1098, 1118,
  1138, 1158, 1178, 1198, 1218, 1242, 1266, 1290, 1314, 1338, 1362, 1386, 1411,
  1435, 1463, 1491, 1519, 1547, 1575, 1603, 1631, 1663, 1695, 1727, 1759, 1791,
  1823, 1859, 1895, 1931, 1967, 2003, 2039, 2079, 2119, 2159, 2199, 2239, 2283,
  2327, 2371, 2415, 2459, 2507, 2555, 2603, 2651, 2703, 2755, 2807, 2859, 2915,
  2971, 3027, 3083, 3143, 3203, 3263, 3327, 3391, 3455, 3523, 3591, 3659, 3731,
  3803, 3876, 3952, 4028, 4104, 4184, 4264, 4348, 4432, 4516, 4604, 4692, 4784,
  4876, 4972, 5068, 5168, 5268, 5372, 5476, 5584, 5692, 5804, 5916, 6032, 6148,
  6268, 6388, 6512, 6640, 6768, 6900, 7036, 7172, 7312,
};

static const int16_t ac_qlookup_12_QTX[QINDEX_RANGE] = {
  4,     13,    19,    27,    35,    44,    54,    64,    75,    87,    99,
  112,   126,   139,   154,   168,   183,   199,   214,   230,   247,   263,
  280,   297,   314,   331,   349,   366,   384,   402,   420,   438,   456,
  475,   493,   511,   530,   548,   567,   586,   604,   623,   642,   660,
  679,   698,   716,   735,   753,   772,   791,   809,   828,   846,   865,
  884,   902,   920,   939,   957,   976,   994,   1012,  1030,  1049,  1067,
  1085,  1103,  1121,  1139,  1157,  1175,  1193,  1211,  1229,  1246,  1264,
  1282,  1299,  1317,  1335,  1352,  1370,  1387,  1405,  1422,  1440,  1457,
  1474,  1491,  1509,  1526,  1543,  1560,  1577,  1595,  1627,  1660,  1693,
  1725,  1758,  1791,  1824,  1856,  1889,  1922,  1954,  1987,  2020,  2052,
  2085,  2118,  2150,  2183,  2216,  2248,  2281,  2313,  2346,  2378,  2411,
  2459,  2508,  2556,  2605,  2653,  2701,  2750,  2798,  2847,  2895,  2943,
  2992,  3040,  3088,  3137,  3185,  3234,  3298,  3362,  3426,  3491,  3555,
  3619,  3684,  3748,  3812,  3876,  3941,  4005,  4069,  4149,  4230,  4310,
  4390,  4470,  4550,  4631,  4711,  4791,  4871,  4967,  5064,  5160,  5256,
  5352,  5448,  5544,  5641,  5737,  5849,  5961,  6073,  6185,  6297,  6410,
  6522,  6650,  6778,  6906,  7034,  7162,  7290,  7435,  7579,  7723,  7867,
  8011,  8155,  8315,  8475,  8635,  8795,  8956,  9132,  9308,  9484,  9660,
  9836,  10028, 10220, 10412, 10604, 10812, 11020, 11228, 11437, 11661, 11885,
  12109, 12333, 12573, 12813, 13053, 13309, 13565, 13821, 14093, 14365, 14637,
  14925, 15213, 15502, 15806, 16110, 16414, 16734, 17054, 17390, 17726, 18062,
  18414, 18766, 19134, 19502, 19886, 20270, 20670, 21070, 21486, 21902, 22334,
  22766, 23214, 23662, 24126, 24590, 25070, 25551, 26047, 26559, 27071, 27599,
  28143, 28687, 29247,
};
int16_t av1_ac_quant_QTX(int qindex, int delta, aom_bit_depth_t bit_depth) {
  const int q_clamped = clamp(qindex + delta, 0, MAXQ);
  switch (bit_depth) {
    case AOM_BITS_8: return ac_qlookup_QTX[q_clamped];
    case AOM_BITS_10: return ac_qlookup_10_QTX[q_clamped];
    case AOM_BITS_12: return ac_qlookup_12_QTX[q_clamped];
    default:
      assert(0 && "bit_depth should be AOM_BITS_8, AOM_BITS_10 or AOM_BITS_12");
      return -1;
  }
}
static void pick_cdef_from_qp(
        FrameHeader *frm_hdr,   SequenceControlSet *scs_ptr ,PictureControlSet * pcs_ptr) {
  const int bd = scs_ptr->static_config.encoder_bit_depth;
  const int q =
      av1_ac_quant_QTX(frm_hdr->quantization_params.base_q_idx, 0, bd) >> (bd - 8);

  frm_hdr->cdef_params.cdef_bits             = 0;
  pcs_ptr->parent_pcs_ptr->nb_cdef_strengths = 1;
  frm_hdr->cdef_params.cdef_damping = 3 + (frm_hdr->quantization_params.base_q_idx >> 6);

  int predicted_y_f1 = 0;
  int predicted_y_f2 = 0;
  int predicted_uv_f1 = 0;
  int predicted_uv_f2 = 0;
  aom_clear_system_state();
  if (!frame_is_intra_only(pcs_ptr->parent_pcs_ptr)) {
    predicted_y_f1 = clamp((int)roundf(q * q * -0.0000023593946f +
                                       q * 0.0068615186f + 0.02709886f),
                           0, 15);
    predicted_y_f2 = clamp((int)roundf(q * q * -0.00000057629734f +
                                       q * 0.0013993345f + 0.03831067f),
                           0, 3);
    predicted_uv_f1 = clamp((int)roundf(q * q * -0.0000007095069f +
                                        q * 0.0034628846f + 0.00887099f),
                            0, 15);
    predicted_uv_f2 = clamp((int)roundf(q * q * 0.00000023874085f +
                                        q * 0.00028223585f + 0.05576307f),
                            0, 3);
  } else {
    predicted_y_f1 = clamp(
        (int)roundf(q * q * 0.0000033731974f + q * 0.008070594f + 0.0187634f),
        0, 15);
    predicted_y_f2 = clamp(
        (int)roundf(q * q * 0.0000029167343f + q * 0.0027798624f + 0.0079405f),
        0, 3);
    predicted_uv_f1 = clamp(
        (int)roundf(q * q * -0.0000130790995f + q * 0.012892405f - 0.00748388f),
        0, 15);
    predicted_uv_f2 = clamp((int)roundf(q * q * 0.0000032651783f +
                                        q * 0.00035520183f + 0.00228092f),
                            0, 3);
  }
  frm_hdr->cdef_params.cdef_y_strength[0] =
      predicted_y_f1 * CDEF_SEC_STRENGTHS + predicted_y_f2;
  frm_hdr->cdef_params.cdef_uv_strength[0] =
      predicted_uv_f1 * CDEF_SEC_STRENGTHS + predicted_uv_f2;

  const int nvfb = ( pcs_ptr->parent_pcs_ptr->av1_cm->mi_rows + MI_SIZE_64X64 - 1) / MI_SIZE_64X64;
  const int nhfb = (pcs_ptr->parent_pcs_ptr->av1_cm->mi_cols + MI_SIZE_64X64 - 1) / MI_SIZE_64X64;

  ModeInfo **mi_grid_base = pcs_ptr->mi_grid_base;
  for (int r = 0; r < nvfb; ++r) {
    for (int c = 0; c < nhfb; ++c) {
      mi_grid_base[MI_SIZE_64X64 * c]->mbmi.cdef_strength = 0;
    }
    mi_grid_base += MI_SIZE_64X64 * pcs_ptr->parent_pcs_ptr->av1_cm->mi_stride;
  }

}
#endif
/******************************************************
 * CDEF Kernel
 ******************************************************/
void *cdef_kernel(void *input_ptr) {
    // Context & SCS & PCS
    EbThreadContext *   thread_context_ptr = (EbThreadContext *)input_ptr;
    CdefContext *       context_ptr        = (CdefContext *)thread_context_ptr->priv;
    PictureControlSet * pcs_ptr;
    SequenceControlSet *scs_ptr;

    //// Input
    EbObjectWrapper *dlf_results_wrapper_ptr;
    DlfResults *     dlf_results_ptr;

    //// Output
    EbObjectWrapper *cdef_results_wrapper_ptr;
    CdefResults *    cdef_results_ptr;

    // SB Loop variables

    for (;;) {
        FrameHeader *frm_hdr;

        // Get DLF Results
        EB_GET_FULL_OBJECT(context_ptr->cdef_input_fifo_ptr, &dlf_results_wrapper_ptr);

        dlf_results_ptr = (DlfResults *)dlf_results_wrapper_ptr->object_ptr;
        pcs_ptr         = (PictureControlSet *)dlf_results_ptr->pcs_wrapper_ptr->object_ptr;
        scs_ptr         = (SequenceControlSet *)pcs_ptr->scs_wrapper_ptr->object_ptr;

        EbBool     is_16bit = (EbBool)(scs_ptr->static_config.encoder_bit_depth > EB_8BIT);
        Av1Common *cm       = pcs_ptr->parent_pcs_ptr->av1_cm;
        frm_hdr             = &pcs_ptr->parent_pcs_ptr->frm_hdr;
#if CDEF_OPT
        if (pcs_ptr->parent_pcs_ptr->cdef_level < 9)
#endif
        if (scs_ptr->seq_header.cdef_level && pcs_ptr->parent_pcs_ptr->cdef_level) {
            if (scs_ptr->static_config.is_16bit_pipeline || is_16bit)
                cdef_seg_search16bit(pcs_ptr, scs_ptr, dlf_results_ptr->segment_index);
            else
                cdef_seg_search(pcs_ptr, scs_ptr, dlf_results_ptr->segment_index);
        }

        //all seg based search is done. update total processed segments. if all done, finish the search and perfrom application.
        eb_block_on_mutex(pcs_ptr->cdef_search_mutex);

        pcs_ptr->tot_seg_searched_cdef++;
        if (pcs_ptr->tot_seg_searched_cdef == pcs_ptr->cdef_segments_total_count) {
            // SVT_LOG("    CDEF all seg here  %i\n", pcs_ptr->picture_number);
            if (scs_ptr->seq_header.cdef_level && pcs_ptr->parent_pcs_ptr->cdef_level) {
                int32_t selected_strength_cnt[64] = {0};
#if CDEF_OPT
                if (pcs_ptr->parent_pcs_ptr->cdef_level == 9 )
                    pick_cdef_from_qp(frm_hdr,scs_ptr,pcs_ptr);
                else
#endif
                finish_cdef_search(0, pcs_ptr, selected_strength_cnt);

                if (scs_ptr->seq_header.enable_restoration != 0 ||
                    pcs_ptr->parent_pcs_ptr->is_used_as_reference_flag ||
                    scs_ptr->static_config.recon_enabled) {
                    if (scs_ptr->static_config.is_16bit_pipeline || is_16bit)
                        av1_cdef_frame16bit(0, scs_ptr, pcs_ptr);
                    else
                        eb_av1_cdef_frame(0, scs_ptr, pcs_ptr);
                }
            } else {
                frm_hdr->cdef_params.cdef_bits             = 0;
                frm_hdr->cdef_params.cdef_y_strength[0]    = 0;
                pcs_ptr->parent_pcs_ptr->nb_cdef_strengths = 1;
                frm_hdr->cdef_params.cdef_uv_strength[0]   = 0;
            }

            //restoration prep

            if (scs_ptr->seq_header.enable_restoration) {
                eb_av1_loop_restoration_save_boundary_lines(cm->frame_to_show, cm, 1);

                //are these still needed here?/!!!
                eb_extend_frame(cm->frame_to_show->buffers[0],
                                cm->frame_to_show->crop_widths[0],
                                cm->frame_to_show->crop_heights[0],
                                cm->frame_to_show->strides[0],
                                RESTORATION_BORDER,
                                RESTORATION_BORDER,
                                scs_ptr->static_config.is_16bit_pipeline || is_16bit);
                eb_extend_frame(cm->frame_to_show->buffers[1],
                                cm->frame_to_show->crop_widths[1],
                                cm->frame_to_show->crop_heights[1],
                                cm->frame_to_show->strides[1],
                                RESTORATION_BORDER,
                                RESTORATION_BORDER,
                                scs_ptr->static_config.is_16bit_pipeline || is_16bit);
                eb_extend_frame(cm->frame_to_show->buffers[2],
                                cm->frame_to_show->crop_widths[1],
                                cm->frame_to_show->crop_heights[1],
                                cm->frame_to_show->strides[1],
                                RESTORATION_BORDER,
                                RESTORATION_BORDER,
                                scs_ptr->static_config.is_16bit_pipeline || is_16bit);
            }

            pcs_ptr->rest_segments_column_count = scs_ptr->rest_segment_column_count;
            pcs_ptr->rest_segments_row_count    = scs_ptr->rest_segment_row_count;
            pcs_ptr->rest_segments_total_count =
                (uint16_t)(pcs_ptr->rest_segments_column_count * pcs_ptr->rest_segments_row_count);
            pcs_ptr->tot_seg_searched_rest = 0;
            uint32_t segment_index;
            for (segment_index = 0; segment_index < pcs_ptr->rest_segments_total_count;
                 ++segment_index) {
                // Get Empty Cdef Results to Rest
                eb_get_empty_object(context_ptr->cdef_output_fifo_ptr, &cdef_results_wrapper_ptr);
                cdef_results_ptr = (struct CdefResults *)cdef_results_wrapper_ptr->object_ptr;
                cdef_results_ptr->pcs_wrapper_ptr = dlf_results_ptr->pcs_wrapper_ptr;
                cdef_results_ptr->segment_index   = segment_index;
                // Post Cdef Results
                eb_post_full_object(cdef_results_wrapper_ptr);
            }
        }
        eb_release_mutex(pcs_ptr->cdef_search_mutex);

        // Release Dlf Results
        eb_release_object(dlf_results_wrapper_ptr);
    }

    return NULL;
}
