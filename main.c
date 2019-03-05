#include <stdio.h>
#include <stdlib.h>
#include "libavcodec/avcodec.h" //编码
#include "libavformat/avformat.h" //封装格式处理
#include "libswscale/swscale.h"  //像素处理
#include "libswresample/swresample.h" //重采样

#include "SDL2/SDL.h"
int main() {
    //输入输出文件名

    //封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //打开音频流文件
    if(avformat_open_input(&pFormatCtx, "../file/demo2.mp3", NULL, NULL) != 0)
    {
        printf("无法打开视频文件\n");
    }

    //if(pFormatCtx->streams[0]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
    if(pFormatCtx->streams[0]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        printf("输入的文件是音频类型\n");
    }

    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        printf("获取音频信息失败\n");
    }

    //拿到对应流的解码器
    AVCodecContext *pCodeCtx = pFormatCtx->streams[0]->codec;
    //通过上下文拿到编解码id,通过id拿到解码器
    AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
    if(pCodec == NULL)
    {
        printf("获取解码器失败！\n");
    }

    //打开解码器
    if(avcodec_open2(pCodeCtx, pCodec, NULL) < 0)
    {
        printf("编码器无法打开\n");
    }

    //申请压缩数据空间
    AVPacket *packet =av_malloc(sizeof(AVPacket));

    ////解压缩数据
    AVFrame *frame = av_frame_alloc();

    ////重采样
    SwrContext *swrCtx = swr_alloc();
    ////设置输入的采样格式
    enum AVSampleFormat in_sample_fmt = pCodeCtx->sample_fmt;
    ////设置输出的采样格式
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    ////输入的采样率
    int in_sample_rate = pCodeCtx->sample_rate;
    ////输出的采样率
    int out_sample_rate = 44100;
    ////输入声道布局
    uint64_t int_ch_layout = pCodeCtx->channel_layout;
    ////输出声道布局
    uint64_t out_ch_layout = AV_CH_LAYOUT_MONO;

    swr_alloc_set_opts(swrCtx, out_ch_layout, out_sample_fmt, out_sample_rate,
                       int_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);

    swr_init(swrCtx);

    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    ////存储PCM数据
    uint8_t *out_buffer = (uint8_t *) av_malloc(2 * 44100);
    FILE *fp_pcm = fopen("../file/out.pcm", "wb");
    if(NULL == fp_pcm)
    {
        printf("打开输出文件失败！\n");
    }
    int ret, got_frame, framecount=0;

    while (av_read_frame(pFormatCtx, packet) >= 0)
    {
        //ret = avcodec_decode_audio4(pCodeCtx, frame, &got_frame, packet);
        ret = avcodec_send_packet(pCodeCtx, packet);
        ret = avcodec_receive_frame(pCodeCtx, frame);
        if(ret < 0)
        {
            printf("解码完成\n");
        }
        printf("解码%d帧\n", framecount++);
        swr_convert(swrCtx, &out_buffer, 2*44100, &frame->data, frame->nb_samples);
        ////获取sample的大小
        int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb,
                                                         frame->nb_samples, out_sample_fmt, 1);
        ////写入文件测试
        fwrite(out_buffer, 1, out_buffer_size, fp_pcm);
        av_packet_unref(packet);
    }
    fclose(fp_pcm);
    av_frame_free(&frame);
    av_free(out_buffer);
    swr_free(&swrCtx);
    avcodec_close(pCodeCtx);
    avformat_close_input(&pFormatCtx);
    return 0;

}