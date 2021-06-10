#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <stdio.h>
#define LOG_TAG "ffmpegLog"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
extern "C"{
#include "android/native_window.h"
#include "include/libavcodec/avcodec.h"
#include "include/libavcodec/jni.h"
#include "include/libavformat/avformat.h"
#include "include/libavcodec/codec.h"
#include "include/libavutil/log.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
JavaVM *javaVm;
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
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegr4_SurfaceViewActivity_playVideo(JNIEnv *env, jobject thiz, jstring path, jobject surface) {
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
    while(av_read_frame(pFormatCtx,pPacket) >= 0){
        if(pPacket->stream_index == video_index){
            ret = avcodec_send_packet(pCodecCtx,pPacket);
            if(ret<0 && ret != AVERROR(EAGAIN) &&ret != AVERROR_EOF){
                av_log(NULL,AV_LOG_ERROR,"PLAYER ERROR");
                return ;
            }
            ret = avcodec_receive_frame(pCodecCtx,pFrame);
            if(ret<0 && ret != AVERROR_EOF){
                av_log(NULL,AV_LOG_ERROR,"player error 2");
                continue;
            }
            ret = sws_scale(swsContext,
                            (const uint8_t* const*)pFrame->data,pFrame->linesize,
                            0,vHeight,rgbaFrame->data,rgbaFrame->linesize);
            if(ret<0){
                av_log(NULL,AV_LOG_ERROR,"play error 3");
                return ;
            }
            ret = ANativeWindow_lock(nativeWindow,&windowBuffer,NULL);
            if(ret <0){
                av_log(NULL,AV_LOG_ERROR,"player error 4");
                return ;
            }else{
                uint8_t  *bit = (uint8_t *)windowBuffer.bits;
                for(int h =0;h<vHeight;h++){
                    memcpy(bit+h*windowBuffer.stride*4,
                            out_buffer+h*rgbaFrame->linesize[0],
                            rgbaFrame->linesize[0]);
                }
                ANativeWindow_unlockAndPost(nativeWindow);
            }
        }
        av_packet_unref(pPacket);

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