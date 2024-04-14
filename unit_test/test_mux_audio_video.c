#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../thirdparty/codec_sim/ipc.h"

#include "../thirdparty/minimp4/include/minimp4.h"
#include "../thirdparty/codec_sim/ipc.h"
#include "../thirdparty/log/include/log.h"

static MP4E_mux_t *mp4_mux = NULL;
static FILE *mp4_file_mux = NULL;
static mp4_h26x_writer_t mp4wr;
static int count = 0;

static int write_callback(int64_t offset, const void *buffer, size_t size, void *token)
{
    FILE *f = (FILE *)token;
    fseek(f, offset, SEEK_SET);
    return fwrite(buffer, 1, size, f) != size;
}

static void _close()
{
    MP4E_close(mp4_mux);
    mp4_h26x_write_close(&mp4wr);
    fclose(mp4_file_mux);
    log_debug("close\n");

    

}

static int VideoFrameCallBack(uint8_t *frame, int len, int iskey, int64_t timestamp)
{

    count ++;

    if (MP4E_STATUS_OK != mp4_h26x_write_nal(&mp4wr, frame, len, timestamp)) 
    {
        log_error("mp4_h26x_write_nal failed\n");
        return -1;
    }
    else
    {
        log_debug("h26x_write_nal OK\n");
        log_debug("Timestamp: %ld , frame VIDEO len: %d , is key frame: %d \n",timestamp, len, iskey);
    }
    
    log_debug("count = %d\n", count);
    if(count > 100)
        {
            _close();
        }
    return 0;
}

static int AudioFrameCallBack(uint8_t *frame, int len, int64_t timestamp)
{
    

    // int MP4E_put_sample(MP4E_mux_t *mux, int track_num, const void *data,
    //                     int data_bytes, int duration, int kind);

    log_debug("Timestamp: %ld , frame AUDIO len: %d \n",timestamp, len);

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
            .video_file = "/home/ndp/Documents/test_mp4/test_file/test.h264", // Test with big_file becase this file have many I frame
            .audio_file = "/home/ndp/Documents/test_mp4/test_file/aac-sample.aac",
            .pic_file = NULL,
            .video_cb = VideoFrameCallBack,
            .audio_cb = NULL, //AudioFrameCallBack,
            .event_cb = NULL,
        };

    ipc_init(&param);
    ipc_run();

    mp4_file_mux = fopen("/home/ndp/Documents/test_mp4/test_file/test_mp4.mp4", "wb");
    if (mp4_file_mux == NULL)
    {
        log_error("Can't open file mp4");
        return -1;
    }

    
    mp4_mux = MP4E_open(1, 0, mp4_file_mux, write_callback);
    if(mp4_mux != NULL)
    {
        log_debug("Create mp4 file ok\n");
    }


    if (MP4E_STATUS_OK != mp4_h26x_write_init(&mp4wr, mp4_mux, 1920, 1080, 0))
    {
        log_error("mp4_h26x_write_init failed\n");
        return -1;
    }
    else
        log_debug("init ok\n");
    
    
   
    while (1)
    {
        log_debug("%d", count);
        if(count > 100)
            break;
        sleep(2);
        
    }
    
   
    return 0;

}
