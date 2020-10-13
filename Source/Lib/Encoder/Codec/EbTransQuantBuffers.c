/*
* Copyright(c) 2019 Intel Corporation
*
* This source code is subject to the terms of the BSD 2 Clause License and
* the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
* was not distributed with this source code in the LICENSE file, you can
* obtain it at https://www.aomedia.org/license/software-license. If the Alliance for Open
* Media Patent License 1.0 was not distributed with this source code in the
* PATENTS file, you can obtain it at https://www.aomedia.org/license/patent-license.
*/

#include "EbTransQuantBuffers.h"

static void eb_trans_quant_buffers_dctor(EbPtr p) {
    EbTransQuantBuffers* obj = (EbTransQuantBuffers*)p;

#if FEATURE_DCT_DCT_DEDICATED_BUFFER
    for (int i = 0; i < 128; i++)
    EB_DELETE(obj->txb_trans_dct_dct_tx_depth_0[i]);

    EB_DELETE(obj->txb_trans_coeff2_nx2_n_ptr);
#else
    EB_DELETE(obj->txb_trans_coeff2_nx2_n_ptr);
    EB_DELETE(obj->txb_trans_coeff_nxn_ptr);
    EB_DELETE(obj->txb_trans_coeff_n2x_n2_ptr);
    EB_DELETE(obj->txb_quant_coeff_nxn_ptr);
    EB_DELETE(obj->txb_quant_coeff_n2x_n2_ptr);
#endif
}

EbErrorType eb_trans_quant_buffers_ctor(EbTransQuantBuffers* trans_quant_buffers_ptr, uint8_t sb_size) {
#if !FEATURE_DCT_DCT_DEDICATED_BUFFER
    EbPictureBufferDescInitData trans_coeff_init_array;
#endif
    trans_quant_buffers_ptr->dctor            = eb_trans_quant_buffers_dctor;
#if !FEATURE_DCT_DCT_DEDICATED_BUFFER
    trans_coeff_init_array.max_width          = sb_size;
    trans_coeff_init_array.max_height         = sb_size;
    trans_coeff_init_array.bit_depth          = EB_16BIT;
    trans_coeff_init_array.buffer_enable_mask = PICTURE_BUFFER_DESC_FULL_MASK;
    trans_coeff_init_array.color_format       = EB_YUV420;
    trans_coeff_init_array.left_padding       = 0;
    trans_coeff_init_array.right_padding      = 0;
    trans_coeff_init_array.top_padding        = 0;
    trans_coeff_init_array.bot_padding        = 0;
    trans_coeff_init_array.split_mode         = EB_FALSE;
#endif
    EbPictureBufferDescInitData trans_coeff_32bit_init_array;
    trans_coeff_32bit_init_array.max_width          = sb_size;
    trans_coeff_32bit_init_array.max_height         = sb_size;
    trans_coeff_32bit_init_array.bit_depth          = EB_32BIT;
    trans_coeff_32bit_init_array.buffer_enable_mask = PICTURE_BUFFER_DESC_FULL_MASK;
    trans_coeff_32bit_init_array.color_format       = EB_YUV420;
    trans_coeff_32bit_init_array.left_padding       = 0;
    trans_coeff_32bit_init_array.right_padding      = 0;
    trans_coeff_32bit_init_array.top_padding        = 0;
    trans_coeff_32bit_init_array.bot_padding        = 0;
    trans_coeff_32bit_init_array.split_mode         = EB_FALSE;


#if FEATURE_DCT_DCT_DEDICATED_BUFFER
    for(int i=0; i < 128;i++)
    EB_NEW(trans_quant_buffers_ptr->txb_trans_dct_dct_tx_depth_0[i],
        eb_picture_buffer_desc_ctor,
        (EbPtr)&trans_coeff_32bit_init_array);

    EB_NEW(trans_quant_buffers_ptr->txb_trans_coeff2_nx2_n_ptr,
        eb_picture_buffer_desc_ctor,
        (EbPtr)&trans_coeff_32bit_init_array);
#else
    EB_NEW(trans_quant_buffers_ptr->txb_trans_coeff2_nx2_n_ptr,
        eb_picture_buffer_desc_ctor,
        (EbPtr)&trans_coeff_32bit_init_array);
    EB_NEW(trans_quant_buffers_ptr->txb_trans_coeff_nxn_ptr,
           eb_picture_buffer_desc_ctor,
           (EbPtr)&trans_coeff_32bit_init_array);
    EB_NEW(trans_quant_buffers_ptr->txb_trans_coeff_n2x_n2_ptr,
           eb_picture_buffer_desc_ctor,
           (EbPtr)&trans_coeff_init_array);
    EB_NEW(trans_quant_buffers_ptr->txb_quant_coeff_nxn_ptr,
           eb_picture_buffer_desc_ctor,
           (EbPtr)&trans_coeff_init_array);
    EB_NEW(trans_quant_buffers_ptr->txb_quant_coeff_n2x_n2_ptr,
           eb_picture_buffer_desc_ctor,
           (EbPtr)&trans_coeff_init_array);
#endif
    return EB_ErrorNone;
}
