#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>
#include <stdio.h>
#define LOG_TAG "ffmpegLog"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
extern "C"{
#include "include/libavfilter/avfilter.h"
#include "android/native_window.h"
#include "include/libavcodec/avcodec.h"
#include "include/libavcodec/jni.h"
#include "include/libavformat/avformat.h"
#include "include/libavcodec/codec.h"
#include "include/libavutil/log.h"
#include "include/libavfilter/buffersink.h"
#include "include/libavfilter/buffersrc.h"
#include "include/libswscale/swscale.h"
#include "include/libswresample/swresample.h"
#include "include/libavutil/imgutils.h"
#include <ctime>
#include "include/libswscale/swscale.h"
#include "include/libavutil/imgutils.h"
    static volatile int isPause = 0;
    static volatile int64_t pauseIndex = 0;
    static double duration = 0;
JavaVM *javaVm;
bool init_subtitle_filter(AVFilterContext * &buffersrcContext, AVFilterContext * &buffersinkContext,
                          const char* args, const char* filterDesc);
}
static void log_callback_test2(void *ptr, int level, const char *fmt, va_list vl)
{
    va_list vl2;
    char *line = static_cast<char *>(malloc(128 * sizeof(char)));
    static int print_prefix = 1;
    va_copy(vl2, vl);
    av_log_format_line(ptr, level, fmt, vl2, line, 128, &print_prefix);
    va_end(vl2);
    line[127] = '\0';
    LOGE("%s", line);
    free(line);
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_ffmpegr4_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
jint JNI_OnLoad(JavaVM* vm, void* reserved){
    av_jni_set_java_vm(vm, NULL);
    return JNI_VERSION_1_6;
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_ffmpegr4_MainActivity_stringFromFfmpeg(JNIEnv *env, jobject thiz) {
    // TODO: implement stringFromFfmpeg()
    //std::string hello = "Hello from C++";
    //return env->NewStringUTF(hello.c_str());
    return env->NewStringUTF(av_version_info());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegr4_activity_TransActivity_transVideo(JNIEnv *env, jobject thiz, jstring path,jstring output) {
    AVFormatContext *avFormatContext;
    int i , videoindex;
    AVCodecContext  *avCodecContext;
    AVCodec *avCodec;
    AVFrame *avFrame, *yuvFrame;
    AVPacket *packet;
    av_log_set_callback(log_callback_test2);
    avformat_network_init();
    avFormatContext = avformat_alloc_context();
    const char* filepath = env->GetStringUTFChars(path,0);
    const char* outputpath =  env->GetStringUTFChars(output,0);
    avformat_open_input(&avFormatContext,filepath,NULL,NULL);

    avformat_find_stream_info(avFormatContext, nullptr);

    videoindex = -1;
    for(int i  =0; i<avFormatContext->nb_streams; i++){
        if(avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            videoindex = i;
            break;
        }
    }
    avCodecContext = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(avCodecContext,avFormatContext->streams[videoindex]->codecpar);
    avCodec = (AVCodec *)(avcodec_find_decoder(avCodecContext->codec_id));

    avcodec_open2(avCodecContext,avCodec, nullptr);

    avFrame = av_frame_alloc();
    yuvFrame = av_frame_alloc();
    uint8_t *outbuffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, avCodecContext->width,avCodecContext->height,1));
    av_image_fill_arrays(yuvFrame->data,yuvFrame->linesize,outbuffer,AV_PIX_FMT_YUV420P,avCodecContext->width,avCodecContext->height,1);

    packet = av_packet_alloc();
    SwsContext *img_convert_ctx = sws_getContext(
            avCodecContext->width,avCodecContext->height,avCodecContext->pix_fmt,
            avCodecContext->width,avCodecContext->height,AV_PIX_FMT_YUV420P,SWS_BICUBIC,NULL,NULL,NULL
            );
    FILE  *fp_yuv =  fopen(outputpath,"wb+");

    int frame_cnt = 0;
    int time_start = clock();
    int ret = 0;
    int y_size;
    while(av_read_frame(avFormatContext,packet)>=0){
        ret=  avcodec_send_packet(avCodecContext,packet);
        if(ret < 0&&(ret != AVERROR(EAGAIN)&& ret !=AVERROR_EOF)) {
            return ;
        }
        ret = avcodec_receive_frame(avCodecContext,avFrame);
        if(ret <0) {
            return ;
        }
        y_size = avCodecContext->width*avCodecContext->height;
        sws_scale(img_convert_ctx,
                  (const uint8_t* const*)avFrame->data,avFrame->linesize,
                  0,avCodecContext->height,yuvFrame->data,yuvFrame->linesize);
        fwrite(yuvFrame->data[0],1,y_size,fp_yuv);
        fwrite(yuvFrame->data[1],1,y_size/4,fp_yuv);
        fwrite(yuvFrame->data[2],1,y_size/4,fp_yuv);
    }
        av_packet_free(&packet);

    sws_freeContext(img_convert_ctx);
    av_free(outbuffer);
    av_frame_free(&avFrame);
    av_frame_free(&yuvFrame);
    avcodec_close(avCodecContext);
    avformat_close_input(&avFormatContext);

    env->ReleaseStringUTFChars(path,filepath);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegr4_activity_SurfaceViewActivity_playVideo(JNIEnv *env, jobject thiz,
                                                                 jstring path, jobject surface,jobject seekbar) {
    av_log_set_callback(log_callback_test2);
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVPacket *pPacket;
    AVCodec *pCodec;
    AVFrame *pFrame,*rgbaFrame;
    int ret ,video_index;
    av_log_set_level(AV_LOG_INFO);
    av_log(nullptr,AV_LOG_INFO,"LOG: begin!");
    pFormatCtx = avformat_alloc_context();
    const char *filepath = env->GetStringUTFChars(path,0);
    av_log(nullptr,AV_LOG_INFO,"filepath: %s",filepath);
    ret = avformat_open_input(&pFormatCtx, filepath,NULL, NULL);
    if(ret < 0){
        av_log(NULL,AV_LOG_ERROR,"can;t open input");
        return ;
    }
    //av_dump_format(pFormatCtx,0,filepath,0);
    av_log(NULL,AV_LOG_INFO,"video duration: %" PRId64,pFormatCtx->duration);
    ret = avformat_find_stream_info(pFormatCtx,NULL);
    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "can;t find stream info");
        return;
    }
    video_index = -1;
    for(int i = 0;i<pFormatCtx->nb_streams; i++){
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            video_index = i;
    }
    if(video_index == -1){
        av_log(NULL,AV_LOG_ERROR,"can;t find video stream");
        return ;
    }
    pCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecCtx,pFormatCtx->streams[video_index]->codecpar);
    pCodec = (AVCodec *)(avcodec_find_decoder(pCodecCtx->codec_id));
    if(pCodec == NULL){
        av_log(NULL,AV_LOG_ERROR,"can;t find decoder");
        return ;
    }
    ret = avcodec_open2(pCodecCtx,pCodec,NULL);
    if(ret <0){
        av_log(NULL,AV_LOG_ERROR,"can;t open video decoder");
        return ;
    }
    int vWidth = pCodecCtx->width;
    int vHeight = pCodecCtx->height;
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env,surface);
    if(nativeWindow == NULL){
        av_log(NULL,AV_LOG_ERROR,"can;t create native window");
        return ;
    }
    ret = ANativeWindow_setBuffersGeometry(nativeWindow,vWidth,vHeight,WINDOW_FORMAT_RGBA_8888);
    if(ret <0){
        av_log(NULL,AV_LOG_ERROR,"can;t set native window buffer");
        return ;
    }
    ANativeWindow_Buffer windowBuffer;
    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();
    rgbaFrame = av_frame_alloc();
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA,vWidth,vHeight,1);
    uint8_t  *out_buffer = (uint8_t*) av_malloc(buffer_size*sizeof(uint8_t));
    av_image_fill_arrays(rgbaFrame->data,rgbaFrame->linesize,out_buffer,AV_PIX_FMT_RGBA,vWidth,vHeight,1);

    struct SwsContext *swsContext = sws_getContext(
            vWidth,vHeight,pCodecCtx->pix_fmt,
            vWidth,vHeight,AV_PIX_FMT_RGBA,
            SWS_BICUBIC,NULL,NULL,NULL
    );
    int isLog = -1;
    int frames = 30;
    jclass seakbar = env->GetObjectClass(seekbar);
    jmethodID set_seekbar_progress = env->GetMethodID(seakbar, "setProgress", "(I)V");
    isPause = 0;

    while (av_read_frame(pFormatCtx, pPacket) >= 0) {

        if (pPacket->stream_index == video_index) {
            ret = avcodec_send_packet(pCodecCtx, pPacket);
            /// TOOD ：输出帧数
            if (isLog == -1) {
                frames = pFormatCtx->streams[video_index]->avg_frame_rate.num /
                         pFormatCtx->streams[video_index]->avg_frame_rate.den;
                av_log(NULL, AV_LOG_INFO, "frames : %d", frames);
                av_log(NULL, AV_LOG_INFO, "timebase : %f", av_q2d(pCodecCtx->time_base));
                isLog = 1;
            }
            ///
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                av_log(NULL, AV_LOG_ERROR, "PLAYER ERROR");
                break;
            }
            while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                ret = ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "play error 3");
                    return;
                }
                ret = sws_scale(swsContext,
                                (const uint8_t *const *) pFrame->data, pFrame->linesize,
                                0, vHeight, rgbaFrame->data, rgbaFrame->linesize);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "player error 4");
                    return;
                }
                uint8_t *bit = static_cast<uint8_t *>(windowBuffer.bits);
                for (int h = 0; h < vHeight; h++) {
                    memcpy(bit + h * windowBuffer.stride * 4,
                           rgbaFrame->data[0] + h * rgbaFrame->linesize[0],
                           (size_t) rgbaFrame->linesize[0]);
                }
                ANativeWindow_unlockAndPost(nativeWindow);
                //获取当前时间
                double cur_time = (pFrame->best_effort_timestamp *
                                   av_q2d(pFormatCtx->streams[video_index]->time_base) * 1.0) /
                                  (pFormatCtx->duration / 100000000.0);
                av_log(NULL, AV_LOG_INFO, "cur time %f", cur_time);
                env->CallVoidMethod(seekbar, set_seekbar_progress, (int) (cur_time));
                usleep((unsigned long) 1000000 / frames / 2);
            }
            /*if(ret == AVERROR(EAGAIN)){
                av_log(NULL,AV_LOG_ERROR,"player error 2");
                continue;
            }*/
            /*   ret = ANativeWindow_lock(nativeWindow,&windowBuffer,NULL);
               if(ret<0){
                   av_log(NULL,AV_LOG_ERROR,"play error 3");
                   return ;
               }*/

        }
        av_packet_unref(pPacket);
        /////
        if(isPause) {
            pauseIndex = pFrame->best_effort_timestamp;
            break;
        }
    }
    sws_freeContext(swsContext);
    av_free(out_buffer);
    av_frame_free(&pFrame);
    av_frame_free(&rgbaFrame);
    av_packet_free(&pPacket);
    ANativeWindow_release(nativeWindow);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    env->ReleaseStringUTFChars(path,filepath);

    return;
}extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_ffmpegr4_activity_SurfaceViewActivity_getVideoDurattion(JNIEnv *env, jobject thiz,
                                                                         jstring path) {
    AVFormatContext *pFormatCtx;
    pFormatCtx = avformat_alloc_context();
    const char *filepath = env->GetStringUTFChars(path,0);
    av_log(nullptr,AV_LOG_INFO,"filepath: %s",filepath);
    avformat_open_input(&pFormatCtx, filepath,NULL, NULL);
    duration = pFormatCtx->duration;
    return pFormatCtx->duration;
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegr4_util_MusicPlay_playSound(JNIEnv *env, jobject instance, jstring input) {
    av_log_set_callback(log_callback_test2);
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVPacket *pPacket;
    AVCodec *pCodec;
    AVFrame *pFrame,*yuvFrame;
    int result ,audioindex;
    av_log_set_level(AV_LOG_INFO);
    av_log(nullptr,AV_LOG_INFO,"LOG: begin!");
    pFormatCtx = avformat_alloc_context();
    const char *filepath = env->GetStringUTFChars(input,0);
    av_log(nullptr,AV_LOG_INFO,"filepath: %s",filepath);
    result = avformat_open_input(&pFormatCtx, filepath,NULL, NULL);
    if(result < 0){
        av_log(NULL,AV_LOG_ERROR,"can;t open input");
        return ;
    }
    //av_dump_format(pFormatCtx,0,filepath,0);
    av_log(NULL,AV_LOG_INFO,"video duration: %" PRId64,pFormatCtx->duration);
    result = avformat_find_stream_info(pFormatCtx,NULL);
    if(result < 0) {
        av_log(NULL, AV_LOG_ERROR, "can;t find stream info");
        return;
    }
    audioindex = -1;
    for(int i = 0;i<pFormatCtx->nb_streams; i++){
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            audioindex = i;
    }
    if(audioindex == -1){
        av_log(NULL,AV_LOG_ERROR,"can;t find video stream");
        return ;
    }
    pCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecCtx,pFormatCtx->streams[audioindex]->codecpar);
    pCodec = (AVCodec *)(avcodec_find_decoder(pCodecCtx->codec_id));
    if(pCodec == NULL){
        av_log(NULL,AV_LOG_ERROR,"can;t find decoder");
        return ;
    }
    result = avcodec_open2(pCodecCtx,pCodec,NULL);
    if(result <0){
        av_log(NULL,AV_LOG_ERROR,"can;t open video decoder");
        return ;
    }
    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();
    SwrContext *swrContext = swr_alloc();
    uint8_t *out_buffer = (uint8_t *) av_malloc(44100 * 2);
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    enum AVSampleFormat out_format = AV_SAMPLE_FMT_S16;
    int out_sample_rate = pCodecCtx->sample_rate;

    swr_alloc_set_opts(swrContext,out_ch_layout,out_format,out_sample_rate,
            pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate,0,NULL);
    swr_init(swrContext);

    int out_channel_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    jclass david_player = env->GetObjectClass(instance);
    jmethodID createAudio = env->GetMethodID(david_player,"createTrack","(II)V");
    env->CallVoidMethod(instance, createAudio,44100, out_channel_nb);
    jmethodID audio_write = env->GetMethodID(david_player,"playTrack","([BI)V");
    isPause = 0;
    while(isPause == 0) {
        LOGD("pause flag %d",isPause);
        while (av_read_frame(pFormatCtx, pPacket) >= 0) {
            LOGD("pause flag depper %d",isPause);
            if (pPacket->stream_index == audioindex) {
                result = avcodec_send_packet(pCodecCtx, pPacket);
                if (result < 0 && result != AVERROR(EAGAIN) && result != AVERROR_EOF) {
                    av_log(NULL, AV_LOG_ERROR, "PLAYER ERROR");
                    break;
                }
                result = avcodec_receive_frame(pCodecCtx, pFrame);
                if (result == AVERROR(EAGAIN)) {
                    av_log(NULL, AV_LOG_ERROR, "player error 2");
                    continue;
                }
                if (result < 0) {
                    av_log(NULL, AV_LOG_ERROR, "player error 4");
                    return;
                } else {
                    swr_convert(swrContext, &out_buffer, 44100 * 2,
                                (const uint8_t **) pFrame->data, pFrame->nb_samples);
                    int size = av_samples_get_buffer_size(NULL, out_channel_nb, pFrame->nb_samples,
                                                          AV_SAMPLE_FMT_S16, 1);
                    jbyteArray audio_sample_array = env->NewByteArray(size);
                    env->SetByteArrayRegion(audio_sample_array, 0, size,
                                            (const jbyte *) out_buffer);
                    env->CallVoidMethod(instance, audio_write, audio_sample_array, size);
                    env->DeleteLocalRef(audio_sample_array);
                }

            }
        }
    }
        swr_free(&swrContext);
        av_free(out_buffer);
        av_frame_free(&pFrame);
        av_frame_free(&yuvFrame);
        av_packet_free(&pPacket);
        avcodec_close(pCodecCtx);
        avformat_close_input(&pFormatCtx);

        env->ReleaseStringUTFChars(input,filepath);
    }

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegr4_activity_SurfaceViewActivity_pausePlay(JNIEnv *env, jobject thiz) {
    if(isPause == 0)
        isPause = 1;
    LOGD("pause flag %d",isPause);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegr4_activity_SurfaceViewActivity_resumePlay(JNIEnv *env, jobject thiz,
                                                                  jint num, jstring file_path,
                                                                  jobject surface,
                                                                  jobject seek_bar) {


    av_log_set_callback(log_callback_test2);
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVPacket *pPacket;
    AVCodec *pCodec;
    AVFrame *pFrame,*rgbaFrame;
    int ret ,video_index;
    av_log_set_level(AV_LOG_INFO);
    av_log(nullptr,AV_LOG_INFO,"LOG: begin!");
    pFormatCtx = avformat_alloc_context();
    const char *filepath = env->GetStringUTFChars(file_path,0);
    av_log(nullptr,AV_LOG_INFO,"filepath: %s",filepath);
    ret = avformat_open_input(&pFormatCtx, filepath,NULL, NULL);
    if(ret < 0){
        av_log(NULL,AV_LOG_ERROR,"can;t open input");
        return ;
    }
    //av_dump_format(pFormatCtx,0,filepath,0);
    av_log(NULL,AV_LOG_INFO,"video duration: %" PRId64,pFormatCtx->duration);
    ret = avformat_find_stream_info(pFormatCtx,NULL);
    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "can;t find stream info");
        return;
    }
    video_index = -1;
    for(int i = 0;i<pFormatCtx->nb_streams; i++){
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            video_index = i;
    }
    if(video_index == -1){
        av_log(NULL,AV_LOG_ERROR,"can;t find video stream");
        return ;
    }
    pCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecCtx,pFormatCtx->streams[video_index]->codecpar);
    pCodec = (AVCodec *)(avcodec_find_decoder(pCodecCtx->codec_id));
    if(pCodec == NULL){
        av_log(NULL,AV_LOG_ERROR,"can;t find decoder");
        return ;
    }
    ret = avcodec_open2(pCodecCtx,pCodec,NULL);
    if(ret <0){
        av_log(NULL,AV_LOG_ERROR,"can;t open video decoder");
        return ;
    }
    int vWidth = pCodecCtx->width;
    int vHeight = pCodecCtx->height;
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env,surface);
    if(nativeWindow == NULL){
        av_log(NULL,AV_LOG_ERROR,"can;t create native window");
        return ;
    }
    ret = ANativeWindow_setBuffersGeometry(nativeWindow,vWidth,vHeight,WINDOW_FORMAT_RGBA_8888);
    if(ret <0){
        av_log(NULL,AV_LOG_ERROR,"can;t set native window buffer");
        return ;
    }
    ANativeWindow_Buffer windowBuffer;
    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();
    rgbaFrame = av_frame_alloc();
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA,vWidth,vHeight,1);
    uint8_t  *out_buffer = (uint8_t*) av_malloc(buffer_size*sizeof(uint8_t));
    av_image_fill_arrays(rgbaFrame->data,rgbaFrame->linesize,out_buffer,AV_PIX_FMT_RGBA,vWidth,vHeight,1);

    struct SwsContext *swsContext = sws_getContext(
            vWidth,vHeight,pCodecCtx->pix_fmt,
            vWidth,vHeight,AV_PIX_FMT_RGBA,
            SWS_BICUBIC,NULL,NULL,NULL
    );
    int isLog = -1;
    int frames = 30;
    jclass seakbar = env->GetObjectClass(seek_bar);
    jmethodID set_seekbar_progress = env->GetMethodID(seakbar, "setProgress", "(I)V");
    /// -----设置在暂停时间 记录timestamp
    isPause = 0;
    int64_t cur_timestamp = num / ((av_q2d(pFormatCtx->streams[video_index]->time_base) * 1.0) /
    (pFormatCtx->duration / 100000000.0));
    LOGD("cur_timestamp: %f",av_q2d(pFormatCtx->streams[video_index]->time_base));
    int isIFrame = 0;
    av_seek_frame(pFormatCtx,video_index,10.0/av_q2d(pFormatCtx->streams[video_index]->time_base),AVSEEK_FLAG_BACKWARD);
    /// -----
    while (av_read_frame(pFormatCtx, pPacket) >= 0) {

        if (pPacket->stream_index == video_index) {
            ret = avcodec_send_packet(pCodecCtx, pPacket);
            /// TOOD ：输出帧数
            if (isLog == -1) {
                frames = pFormatCtx->streams[video_index]->avg_frame_rate.num /
                         pFormatCtx->streams[video_index]->avg_frame_rate.den;
                av_log(NULL, AV_LOG_INFO, "frames : %d", frames);
                av_log(NULL, AV_LOG_INFO, "timebase : %f", av_q2d(pCodecCtx->time_base));
                isLog = 1;
            }
            ///
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                av_log(NULL, AV_LOG_ERROR, "PLAYER ERROR");
                break;
            }
            while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                /// ------避免跳转后的画面花屏？未测试
                if(isIFrame == 0 && !pFrame->key_frame){
                    continue;
                }
                isIFrame = 1;
                /// -------
                ret = ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "play error 3");
                    return;
                }
                ret = sws_scale(swsContext,
                                (const uint8_t *const *) pFrame->data, pFrame->linesize,
                                0, vHeight, rgbaFrame->data, rgbaFrame->linesize);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "player error 4");
                    return;
                }
                uint8_t *bit = static_cast<uint8_t *>(windowBuffer.bits);
                for (int h = 0; h < vHeight; h++) {
                    memcpy(bit + h * windowBuffer.stride * 4,
                           rgbaFrame->data[0] + h * rgbaFrame->linesize[0],
                           (size_t) rgbaFrame->linesize[0]);
                }

                ANativeWindow_unlockAndPost(nativeWindow);
                //获取当前时间
                double cur_time = (pFrame->best_effort_timestamp *
                                   av_q2d(pFormatCtx->streams[video_index]->time_base) * 1.0) /
                                  (pFormatCtx->duration / 100000000.0);
                av_log(NULL, AV_LOG_INFO, "cur time %f", cur_time);
                env->CallVoidMethod(seek_bar, set_seekbar_progress, (int) (cur_time));
                usleep((unsigned long) 1000000 / frames / 2);
            }
            /*if(ret == AVERROR(EAGAIN)){
                av_log(NULL,AV_LOG_ERROR,"player error 2");
                continue;
            }*/
            /*   ret = ANativeWindow_lock(nativeWindow,&windowBuffer,NULL);
               if(ret<0){
                   av_log(NULL,AV_LOG_ERROR,"play error 3");
                   return ;
               }*/

        }
        av_packet_unref(pPacket);
        /////
        if(isPause) {
            pauseIndex = pFrame->best_effort_timestamp;
            break;
        }
    }
    sws_freeContext(swsContext);
    av_free(out_buffer);
    av_frame_free(&pFrame);
    av_frame_free(&rgbaFrame);
    av_packet_free(&pPacket);
    ANativeWindow_release(nativeWindow);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    env->ReleaseStringUTFChars(file_path,filepath);

    return;
};extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegr4_activity_SurfaceViewActivity_playWithAss(JNIEnv *env, jobject thiz,
                                                                   jstring path, jobject surface,
                                                                   jobject seek_bar,
                                                                   jstring ass_path) {
    av_log_set_callback(log_callback_test2);
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVPacket *pPacket;
    AVCodec *pCodec;
    AVFrame *pFrame,*rgbaFrame;
    int ret ,video_index;
    av_log_set_level(AV_LOG_INFO);
    av_log(nullptr,AV_LOG_INFO,"LOG: begin!");
    pFormatCtx = avformat_alloc_context();
    const char *filepath = env->GetStringUTFChars(path,0);
    av_log(nullptr,AV_LOG_INFO,"filepath: %s",filepath);
    ret = avformat_open_input(&pFormatCtx, filepath,NULL, NULL);
    if(ret < 0){
        av_log(NULL,AV_LOG_ERROR,"can;t open input");
        return ;
    }
    //av_dump_format(pFormatCtx,0,filepath,0);
    av_log(NULL,AV_LOG_INFO,"video duration: %" PRId64,pFormatCtx->duration);
    ret = avformat_find_stream_info(pFormatCtx,NULL);
    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "can;t find stream info");
        return;
    }
    video_index = -1;
    for(int i = 0;i<pFormatCtx->nb_streams; i++){
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            video_index = i;
    }
    if(video_index == -1){
        av_log(NULL,AV_LOG_ERROR,"can;t find video stream");
        return ;
    }
    pCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecCtx,pFormatCtx->streams[video_index]->codecpar);
    pCodec = (AVCodec *)(avcodec_find_decoder(pCodecCtx->codec_id));
    if(pCodec == NULL){
        av_log(NULL,AV_LOG_ERROR,"can;t find decoder");
        return ;
    }
    ret = avcodec_open2(pCodecCtx,pCodec,NULL);
    if(ret <0){
        av_log(NULL,AV_LOG_ERROR,"can;t open video decoder");
        return ;
    }
    int vWidth = pCodecCtx->width;
    int vHeight = pCodecCtx->height;
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env,surface);
    if(nativeWindow == NULL){
        av_log(NULL,AV_LOG_ERROR,"can;t create native window");
        return ;
    }
    ret = ANativeWindow_setBuffersGeometry(nativeWindow,vWidth,vHeight,WINDOW_FORMAT_RGBA_8888);
    if(ret <0){
        av_log(NULL,AV_LOG_ERROR,"can;t set native window buffer");
        return ;
    }
    /// find ass
    AVFilterContext  *bufferContext = nullptr;
    AVFilterContext  *bufferSinkContext = nullptr;
    int isOpenSubtitle = -1;
    char *args = nullptr;

    const char *assPath = env->GetStringUTFChars(ass_path,0);

    sprintf(args,"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d,",pCodecCtx->width,pCodecCtx->height,
                pCodecCtx->pix_fmt,pFormatCtx->streams[video_index]->time_base.num,pFormatCtx->streams[video_index]->time_base.den,
                pCodecCtx->sample_aspect_ratio.num,pCodecCtx->sample_aspect_ratio.den);
        init_subtitle_filter(bufferContext,bufferSinkContext,args,assPath);

    /// find ass


    ANativeWindow_Buffer windowBuffer;
    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();
    AVFrame  *filterFrame = av_frame_alloc();
    rgbaFrame = av_frame_alloc();
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA,vWidth,vHeight,1);
    uint8_t  *out_buffer = (uint8_t*) av_malloc(buffer_size*sizeof(uint8_t));
    av_image_fill_arrays(rgbaFrame->data,rgbaFrame->linesize,out_buffer,AV_PIX_FMT_RGBA,vWidth,vHeight,1);

    struct SwsContext *swsContext = sws_getContext(
            vWidth,vHeight,pCodecCtx->pix_fmt,
            vWidth,vHeight,AV_PIX_FMT_RGBA,
            SWS_BICUBIC,NULL,NULL,NULL
    );
    int isLog = -1;
    int frames = 30;
    jclass seakbar = env->GetObjectClass(seek_bar);
    jmethodID set_seekbar_progress = env->GetMethodID(seakbar, "setProgress", "(I)V");
    isPause = 0;

    while (av_read_frame(pFormatCtx, pPacket) >= 0) {

        if (pPacket->stream_index == video_index) {
            ret = avcodec_send_packet(pCodecCtx, pPacket);
            /// TOOD ：输出帧数
            if (isLog == -1) {
                frames = pFormatCtx->streams[video_index]->avg_frame_rate.num /
                         pFormatCtx->streams[video_index]->avg_frame_rate.den;
                av_log(NULL, AV_LOG_INFO, "frames : %d", frames);
                av_log(NULL, AV_LOG_INFO, "timebase : %f", av_q2d(pCodecCtx->time_base));
                isLog = 1;
            }
            ///
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                av_log(NULL, AV_LOG_ERROR, "PLAYER ERROR");
                break;
            }
            while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                ret = ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "play error 3");
                    return;
                }
                if (isOpenSubtitle) {
                    if (av_buffersrc_add_frame_flags(bufferSinkContext, pFrame,
                                                     AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                        break;
                    }
                    while (true) {
                        int dst_linsize[4];
                        uint8_t *dst_data[4];
                        av_image_alloc(dst_data, dst_linsize, pCodecCtx->width, pCodecCtx->height,
                                       AV_PIX_FMT_RGBA, 1);
                        ret = av_buffersink_get_frame(bufferSinkContext, pFrame);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                        else if (ret < 0) break;
                        SwsContext *swsContext1 = sws_getContext(filterFrame->width,
                                                                 filterFrame->height,
                                                                 AVPixelFormat(filterFrame->format),
                                                                 pCodecCtx->width,
                                                                 pCodecCtx->height,
                                                                 AV_PIX_FMT_RGBA, SWS_BICUBIC,
                                                                 nullptr, nullptr, nullptr);
                        sws_scale(swsContext1,
                                  (const uint8_t *const *) filterFrame->data, filterFrame->linesize,
                                  0, vHeight, dst_data, dst_linsize);
                        uint8_t *bit = static_cast<uint8_t *>(windowBuffer.bits);
                        for (int h = 0; h < vHeight; h++) {
                            memcpy(bit + h * windowBuffer.stride * 4,
                                   filterFrame->data[0] + h * filterFrame->linesize[0],
                                   (size_t) filterFrame->linesize[0]);
                        }
                        ANativeWindow_unlockAndPost(nativeWindow);
                        usleep((unsigned long) 1000000 / frames / 2);
                    }
                } else{
                    ret = sws_scale(swsContext,
                                    (const uint8_t *const *) pFrame->data, pFrame->linesize,
                                    0, vHeight, rgbaFrame->data, rgbaFrame->linesize);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "player error 4");
                    return;
                }
                uint8_t *bit = static_cast<uint8_t *>(windowBuffer.bits);
                for (int h = 0; h < vHeight; h++) {
                    memcpy(bit + h * windowBuffer.stride * 4,
                           rgbaFrame->data[0] + h * rgbaFrame->linesize[0],
                           (size_t) rgbaFrame->linesize[0]);
                }
                ANativeWindow_unlockAndPost(nativeWindow);
                //获取当前时间
                double cur_time = (pFrame->best_effort_timestamp *
                                   av_q2d(pFormatCtx->streams[video_index]->time_base) * 1.0) /
                                  (pFormatCtx->duration / 100000000.0);
                av_log(NULL, AV_LOG_INFO, "cur time %f", cur_time);
                env->CallVoidMethod(seek_bar, set_seekbar_progress, (int) (cur_time));
                usleep((unsigned long) 1000000 / frames / 2);
            }
        }
            /*if(ret == AVERROR(EAGAIN)){
                av_log(NULL,AV_LOG_ERROR,"player error 2");
                continue;
            }*/
            /*   ret = ANativeWindow_lock(nativeWindow,&windowBuffer,NULL);
               if(ret<0){
                   av_log(NULL,AV_LOG_ERROR,"play error 3");
                   return ;
               }*/

        }
        av_packet_unref(pPacket);
        /////
        if(isPause) {
            pauseIndex = pFrame->best_effort_timestamp;
            break;
        }
    }
    sws_freeContext(swsContext);
    av_free(out_buffer);
    av_frame_free(&pFrame);
    av_frame_free(&rgbaFrame);
    av_packet_free(&pPacket);
    ANativeWindow_release(nativeWindow);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    env->ReleaseStringUTFChars(path,filepath);

    return;
}
bool init_subtitle_filter(AVFilterContext * &buffersrcContext, AVFilterContext * &buffersinkContext,
                                           const char* args, const char* filterDesc)
{
    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *output = avfilter_inout_alloc();
    AVFilterInOut *input = avfilter_inout_alloc();
    AVFilterGraph *filterGraph = avfilter_graph_alloc();

    auto release = [&output, &input] {
        avfilter_inout_free(&output);
        avfilter_inout_free(&input);
    };

    if (!output || !input || !filterGraph) {
        release();
        return false;
    }

    //创建输入过滤器，需要arg
    if (avfilter_graph_create_filter(&buffersrcContext, buffersrc, "in",
                                     args, nullptr, filterGraph) < 0) {
        release();
        return false;
    }

    if (avfilter_graph_create_filter(&buffersinkContext, buffersink, "out",
                                     nullptr, nullptr, filterGraph) < 0) {
        release();
        return false;
    }

    output->name = av_strdup("in");
    output->next = nullptr;
    output->pad_idx = 0;
    output->filter_ctx = buffersrcContext;

    input->name = av_strdup("out");
    input->next = nullptr;
    input->pad_idx = 0;
    input->filter_ctx = buffersinkContext;

    if (avfilter_graph_parse_ptr(filterGraph, filterDesc,
                                 &input, &output, nullptr) < 0) {
        release();
        return false;
    }

    if (avfilter_graph_config(filterGraph, nullptr) < 0) {
        release();
        return false;
    }

    release();
    return true;
}
/// 这里设计问题，应该直接调AudioPlay的方法的
/// 根据实际效果还是开两个线程好一点 不然音频会有卡顿
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegr4_activity_SurfaceViewActivity_playVideoWithMusic(JNIEnv *env, jobject thiz,
                                                                          jstring path,
                                                                          jobject surface,
                                                                          jobject seek_bar,
                                                                          jobject mediaplay) {
    av_log_set_callback(log_callback_test2);
    AVFormatContext *pFormatVctx;
    AVCodecContext *pCodecVctx,*pCodecActx;
    AVPacket *pVPacket;
    AVCodec *pVCodec,*pACodec;
    AVFrame *pVFrame,*rgbaFrame;
    int ret ,video_index, audio_index;
    av_log_set_level(AV_LOG_INFO);
    av_log(nullptr,AV_LOG_INFO,"LOG: begin!");
    pFormatVctx = avformat_alloc_context();
    const char *filepath = env->GetStringUTFChars(path,0);
    av_log(nullptr,AV_LOG_INFO,"filepath: %s",filepath);
    ret = avformat_open_input(&pFormatVctx, filepath, NULL, NULL);
    if(ret < 0){
        av_log(NULL,AV_LOG_ERROR,"can;t open input");
        return ;
    }
    //av_dump_format(pFormatVctx,0,filepath,0);
    av_log(NULL, AV_LOG_INFO,"video duration: %" PRId64, pFormatVctx->duration);
    ret = avformat_find_stream_info(pFormatVctx, NULL);
    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "can;t find stream info");
        return;
    }
    video_index = -1;

    for(int i = 0; i < pFormatVctx->nb_streams; i++){
        if(pFormatVctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            video_index = i;
        if(pFormatVctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            audio_index = i;
    }
    if(video_index == -1 || audio_index == -1){
        av_log(NULL,AV_LOG_ERROR,"can;t find video stream");
        return ;
    }
    pCodecVctx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecVctx, pFormatVctx->streams[video_index]->codecpar);
    pVCodec = (AVCodec *)(avcodec_find_decoder(pCodecVctx->codec_id));

    pCodecActx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecActx, pFormatVctx->streams[audio_index]->codecpar);
    pACodec = (AVCodec *)(avcodec_find_decoder(pCodecActx->codec_id));
    if(pVCodec == NULL || pACodec == NULL) {
        av_log(NULL,AV_LOG_ERROR,"can;t find decoder");
        return ;
    }
    ret = avcodec_open2(pCodecVctx, pVCodec, NULL);
    avcodec_open2(pCodecActx, pACodec, NULL);
    if(ret <0){
        av_log(NULL,AV_LOG_ERROR,"can;t open video decoder");
        return ;
    }
    int vWidth = pCodecVctx->width;
    int vHeight = pCodecVctx->height;
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env,surface);
    if(nativeWindow == NULL){
        av_log(NULL,AV_LOG_ERROR,"can;t create native window");
        return ;
    }
    ret = ANativeWindow_setBuffersGeometry(nativeWindow,vWidth,vHeight,WINDOW_FORMAT_RGBA_8888);
    if(ret <0){
        av_log(NULL,AV_LOG_ERROR,"can;t set native window buffer");
        return ;
    }
    ANativeWindow_Buffer windowBuffer;
    pVPacket = av_packet_alloc();

    pVFrame = av_frame_alloc();
    rgbaFrame = av_frame_alloc();
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA,vWidth,vHeight,1);
    uint8_t  *out_buffer = (uint8_t*) av_malloc(buffer_size*sizeof(uint8_t));
    av_image_fill_arrays(rgbaFrame->data,rgbaFrame->linesize,out_buffer,AV_PIX_FMT_RGBA,vWidth,vHeight,1);

    struct SwsContext *swsContext = sws_getContext(
            vWidth, vHeight, pCodecVctx->pix_fmt,
            vWidth, vHeight, AV_PIX_FMT_RGBA,
            SWS_BICUBIC, NULL, NULL, NULL
    );
    int isLog = -1;
    int frames = 30;
    jclass seakbar = env->GetObjectClass(seek_bar);
    jmethodID set_seekbar_progress = env->GetMethodID(seakbar, "setProgress", "(I)V");
    isPause = 0;
    /// alloc audio
    SwrContext *swrContext = swr_alloc();
    uint8_t *out_v_buffer = (uint8_t *) av_malloc(44100 * 2);
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    enum AVSampleFormat out_format = AV_SAMPLE_FMT_S16;
    int out_sample_rate = pCodecActx->sample_rate;

    swr_alloc_set_opts(swrContext,out_ch_layout,out_format,out_sample_rate,
                       pCodecActx->channel_layout, pCodecActx->sample_fmt, pCodecActx->sample_rate,0,NULL);
    swr_init(swrContext);

    int out_channel_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    jclass david_player = env->GetObjectClass(mediaplay);
    jmethodID createAudio = env->GetMethodID(david_player,"createTrack","(II)V");
    env->CallVoidMethod(mediaplay, createAudio,44100, out_channel_nb);
    jmethodID audio_write = env->GetMethodID(david_player,"playTrack","([BI)V");
    av_log(NULL, AV_LOG_INFO, "audioindex : %d  videoindex %d", audio_index,video_index);
    while (av_read_frame(pFormatVctx, pVPacket) >= 0) {
        if (pVPacket->stream_index == video_index) {
            ret = avcodec_send_packet(pCodecVctx, pVPacket);
            /// TOOD ：输出帧数
            if (isLog == -1) {
                frames = pFormatVctx->streams[video_index]->avg_frame_rate.num /
                         pFormatVctx->streams[video_index]->avg_frame_rate.den;
                av_log(NULL, AV_LOG_INFO, "frames : %d", frames);
                av_log(NULL, AV_LOG_INFO, "timebase : %f", av_q2d(pCodecVctx->time_base));
                isLog = 1;
            }
            ///
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                av_log(NULL, AV_LOG_ERROR, "PLAYER ERROR");
                break;
            }
            while (avcodec_receive_frame(pCodecVctx, pVFrame) == 0) {
                ret = ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "play error 3");
                    return;
                }
                ret = sws_scale(swsContext,
                                (const uint8_t *const *) pVFrame->data, pVFrame->linesize,
                                0, vHeight, rgbaFrame->data, rgbaFrame->linesize);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "player error 4");
                    return;
                }
                uint8_t *bit = static_cast<uint8_t *>(windowBuffer.bits);
                for (int h = 0; h < vHeight; h++) {
                    memcpy(bit + h * windowBuffer.stride * 4,
                           rgbaFrame->data[0] + h * rgbaFrame->linesize[0],
                           (size_t) rgbaFrame->linesize[0]);
                }
                ANativeWindow_unlockAndPost(nativeWindow);
                //获取当前时间
                double cur_time = (pVFrame->best_effort_timestamp *
                                   av_q2d(pFormatVctx->streams[video_index]->time_base) * 1.0) /
                                  (pFormatVctx->duration / 100000000.0);
                av_log(NULL, AV_LOG_INFO, "cur time %f", cur_time);
                env->CallVoidMethod(seek_bar, set_seekbar_progress, (int) (cur_time));
                //usleep((unsigned long) 1000000 / frames / 2);
            }
            /*if(ret == AVERROR(EAGAIN)){
                av_log(NULL,AV_LOG_ERROR,"player error 2");
                continue;
            }*/
            /*   ret = ANativeWindow_lock(nativeWindow,&windowBuffer,NULL);
               if(ret<0){
                   av_log(NULL,AV_LOG_ERROR,"play error 3");
                   return ;
               }*/

        }

        if (pVPacket->stream_index == audio_index) {
            ret = avcodec_send_packet(pCodecActx, pVPacket);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                av_log(NULL, AV_LOG_ERROR, "PLAYER ERROR BY audio");
                break;
            }
           ret=avcodec_receive_frame(pCodecActx, pVFrame);
            // receive frame 这里一般只有一次
            if (ret == AVERROR(EAGAIN)) {
                av_log(NULL, AV_LOG_ERROR, "player error 2");
                continue;
            }
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "player error 4");
                return;
            }else{
                swr_convert(swrContext, &out_v_buffer, 44100 * 2,
                            (const uint8_t **) pVFrame->data, pVFrame->nb_samples);
                int size = av_samples_get_buffer_size(NULL, out_channel_nb, pVFrame->nb_samples,
                                                      AV_SAMPLE_FMT_S16, 1);
                jbyteArray audio_sample_array = env->NewByteArray(size);
                env->SetByteArrayRegion(audio_sample_array, 0, size,
                                        (const jbyte *) out_v_buffer);
                env->CallVoidMethod(mediaplay, audio_write, audio_sample_array, size);
                env->DeleteLocalRef(audio_sample_array);

            }


        }
        av_packet_unref(pVPacket);
        /////
        if(isPause) {
            pauseIndex = pVFrame->best_effort_timestamp;
            break;
        }
    }
    swr_free(&swrContext);
    sws_freeContext(swsContext);
    av_free(out_buffer);
    av_free(out_v_buffer);
    av_frame_free(&pVFrame);
    av_frame_free(&rgbaFrame);
    av_packet_free(&pVPacket);
    ANativeWindow_release(nativeWindow);
    avcodec_close(pCodecVctx);
    avcodec_close(pCodecActx);
    avformat_close_input(&pFormatVctx);
    env->ReleaseStringUTFChars(path,filepath);

    return;
}