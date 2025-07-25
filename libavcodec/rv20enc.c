/*
 * RV20 encoder
 * Copyright (c) 2000,2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * RV20 encoder
 */

#include "codec_internal.h"
#include "mpegvideo.h"
#include "mpegvideodata.h"
#include "mpegvideoenc.h"
#include "h263data.h"
#include "h263enc.h"
#include "put_bits.h"
#include "rv10enc.h"

int ff_rv20_encode_picture_header(MPVMainEncContext *const m)
{
    MPVEncContext *const s = &m->s;

    put_bits_assume_flushed(&s->pb);

    put_bits(&s->pb, 2, s->c.pict_type); //I 0 vs. 1 ?
    put_bits(&s->pb, 1, 0);     /* unknown bit */
    put_bits(&s->pb, 5, s->c.qscale);

    put_sbits(&s->pb, 8, s->picture_number); //FIXME wrong, but correct is not known
    s->c.mb_x = s->c.mb_y = 0;
    ff_h263_encode_mba(s);

    put_bits(&s->pb, 1, s->c.no_rounding);

    av_assert1(s->f_code == 1);
    av_assert1(!s->me.unrestricted_mv);
    av_assert1(!s->alt_inter_vlc);
    av_assert1(!s->umvplus);
    av_assert1(s->modified_quant == 1);
    av_assert1(s->loop_filter == 1);

    s->c.h263_aic = s->c.pict_type == AV_PICTURE_TYPE_I;
    if (s->c.h263_aic) {
        s->c.y_dc_scale_table =
        s->c.c_dc_scale_table = ff_aic_dc_scale_table;
    }else{
        s->c.y_dc_scale_table =
        s->c.c_dc_scale_table = ff_mpeg1_dc_scale_table;
    }
    return 0;
}

const FFCodec ff_rv20_encoder = {
    .p.name         = "rv20",
    CODEC_LONG_NAME("RealVideo 2.0"),
    .p.type         = AVMEDIA_TYPE_VIDEO,
    .p.id           = AV_CODEC_ID_RV20,
    .p.priv_class   = &ff_mpv_enc_class,
    .p.capabilities = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE,
    .priv_data_size = sizeof(MPVMainEncContext),
    .init           = ff_mpv_encode_init,
    FF_CODEC_ENCODE_CB(ff_mpv_encode_picture),
    .close          = ff_mpv_encode_end,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    CODEC_PIXFMTS(AV_PIX_FMT_YUV420P),
    .color_ranges   = AVCOL_RANGE_MPEG,
};
