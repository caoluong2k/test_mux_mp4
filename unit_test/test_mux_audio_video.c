#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../thirdparty/codec_sim/ipc.h"

#include "../thirdparty/minimp4/include/minimp4.h"
#include "../thirdparty/codec_sim/ipc.h"
#include "../thirdparty/log/log.h"

#include "../thirdparty/libAACdec/include/aacdecoder_lib.h"
#include "../thirdparty/libAACenc/include/aacenc_lib.h"

typedef struct MP4_mux_ctx
{
    MP4E_mux_t *mp4_mux;
    FILE *mp4_file;
    mp4_h26x_writer_t mp4wr;
    int audio_track_id;
    int sequential_mode;    // 1
    int fragmentation_mode; // 0
    int is_hevc;            // use for h265
    int count_frame_in_file;
} MP4_mux_ctx_t;

static MP4_mux_ctx_t *mux_ctx = NULL;

static int write_callback(int64_t offset, const void *buffer, size_t size, void *token)
{
    FILE *f = (FILE *)token;
    fseek(f, offset, SEEK_SET);
    return fwrite(buffer, 1, size, f) != size;
}

static void stop_mux(void)
{
    MP4E_close(mux_ctx->mp4_mux);
    mp4_h26x_write_close(&mux_ctx->mp4wr);
    fclose(mux_ctx->mp4_file);
    log_info("=============> Stop mux mp4");
}

static int VideoFrameCallBack(uint8_t *frame, int len, int iskey, int64_t timestamp)
{
    log_debug("Timestamp: %ld , frame VIDEO len: %d , is key frame: %d", timestamp, len, iskey);

    if (MP4E_STATUS_OK != mp4_h26x_write_nal(&mux_ctx->mp4wr, frame, len, timestamp))
    {
        log_error("mp4_h26x_write_nal failed");
        return -1;
    }
    else
    {
        log_debug("h26x_write_nal OK");
    }
    mux_ctx->count_frame_in_file++;
    log_debug("=============> count = %d\n", mux_ctx->count_frame_in_file);

    return 0;
}

static int AudioFrameCallBack(uint8_t *frame, int len, int64_t timestamp)
{
    // log_debug("Timestamp: %ld , frame AUDIO len: %d", timestamp, len);

    // int ret = MP4E_put_sample(mp4_mux, audio_track_num, frame,
    //                           len, timestamp, MP4E_SAMPLE_DEFAULT);

    return 0;
}

int main()
{
    log_set_level(LOG_DEBUG);

    ipc_dev_register(&sim_ipc); // Please run this function before run other function !

    ipc_param_t param =
        {
            .audio_codec = AUDIO_AAC,
            .video_codec = H264,
            .video_fps = 25,
            .audio_sample = 48000,                  // Don't need set this param, program auto define base on audio file
            .video_file = "../test_file/test.h264", // Test with big_file becase this file have many I frame
            .audio_file = "../test_file/aac-sample.aac",
            .pic_file = NULL,
            .video_cb = VideoFrameCallBack,
            .audio_cb = AudioFrameCallBack,
            .event_cb = NULL,
        };

    ipc_init(&param);
    ipc_run();

    /* Init mp4 context */
    mux_ctx = (MP4_mux_ctx_t *)malloc(sizeof(MP4_mux_ctx_t));
    if (mux_ctx == NULL)
    {
        log_error("Can't malloc mux_ctx");
        return -1;
    }

    mux_ctx->sequential_mode = 1;
    mux_ctx->fragmentation_mode = 0;
    mux_ctx->is_hevc = 0;
    mux_ctx->mp4_mux = NULL;
    mux_ctx->mp4_file = NULL;

    mux_ctx->mp4_file = fopen("../test_file/test_mp4.mp4", "wb");
    if (mux_ctx->mp4_file == NULL)
    {
        log_error("Can't open file mp4");
        return -1;
    }

    mux_ctx->mp4_mux = MP4E_open(mux_ctx->sequential_mode, mux_ctx->fragmentation_mode, mux_ctx->mp4_file, write_callback);
    if (mux_ctx->mp4_mux != NULL)
    {
        log_debug("Create mp4 file ok\n");
    }

    // audio track info

#define AUDIO_RATE 12000

    // HANDLE_AACENCODER aacenc;
    // AACENC_InfoStruct info;
    // aacEncOpen(&aacenc, 0, 0);
    // aacEncoder_SetParam(aacenc, AACENC_TRANSMUX, 0);
    // aacEncoder_SetParam(aacenc, AACENC_AFTERBURNER, 1);
    // aacEncoder_SetParam(aacenc, AACENC_BITRATE, 64000);
    // aacEncoder_SetParam(aacenc, AACENC_SAMPLERATE, AUDIO_RATE);
    // aacEncoder_SetParam(aacenc, AACENC_CHANNELMODE, 1);
    // aacEncEncode(aacenc, NULL, NULL, NULL, NULL);
    // aacEncInfo(aacenc, &info);

    // MP4E_track_t tr;
    // tr.track_media_kind = e_audio;
    // tr.language[0] = 'u';
    // tr.language[1] = 'n';
    // tr.language[2] = 'd';
    // tr.language[2] = 0;
    // tr.object_type_indication = MP4_OBJECT_TYPE_AUDIO_ISO_IEC_14496_3;
    // tr.time_scale = 90000;
    // tr.default_duration = 0;
    // tr.u.a.channelcount = 1;

    // audio_track_num = MP4E_add_track(mp4_mux, &tr);
    // MP4E_set_dsi(mp4_mux, audio_track_num, info.confBuf, info.confSize);

    if (MP4E_STATUS_OK != mp4_h26x_write_init(&mux_ctx->mp4wr, mux_ctx->mp4_mux, 1920, 1080, mux_ctx->is_hevc))
    {
        log_error("mp4_h26x_write_init failed");
        return -1;
    }
    else
        log_info("init mp4_h26x_write_init ok");

    while (mux_ctx->count_frame_in_file < 100)
    {
        log_debug("------------------>");
        // if (mux_ctx->count_frame_in_file == 100)
        // {
        //     log_info("=================> Count frame: %d", mux_ctx->count_frame_in_file);
        //     break;
        // }

        sleep(2);
    }

    stop_mux();
    free(mux_ctx);

    return 0;
}
