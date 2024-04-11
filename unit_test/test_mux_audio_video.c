#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../thirdparty/codec_sim/ipc.h"

#include "../thirdparty/minimp4/include/minimp4.h"
#include "../thirdparty/codec_sim/ipc.h"
#include "../thirdparty/log/log.h"


static int VideoFrameCallBack(uint8_t *frame, int len, int iskey, int64_t timestamp)
{

    /**TODO - Get frame here and handle */
    // if (iskey == 1)
    log_debug("Timestamp: %ld , frame VIDEO len: %d , is key frame: %d \n", timestamp, len, iskey);

    return 0;
}

static int AudioFrameCallBack(uint8_t *frame, int len, int64_t timestamp)
{
    
    /**TODO - Get frame here and handle */
    log_debug("Timestamp: %ld , frame AUDIO len: %d \n", timestamp, len);

    return 0;
}

int main()
{

    ipc_dev_register(&sim_ipc);
    log_set_level(LOG_DEBUG);

    ipc_param_t param =
        {
            .audio_codec = AUDIO_AAC,
            .video_codec = H264,
            .video_fps = 25,
            .audio_sample = 48000, // Don't need set this param, program auto define base on audio file
            .video_file = "../test_file/big_file.h264", // Test with big_file becase this file have many I frame
            .audio_file = "../test_file/frame_to_file_aac.aac",
            .pic_file = NULL,
            .video_cb = VideoFrameCallBack,
            .audio_cb = AudioFrameCallBack,
            .event_cb = NULL,
        };

    ipc_init(&param);
    ipc_run();

    while (1)
    {
        sleep(2);
    }
}
