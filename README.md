# FFmpeg - Android 学习

##一些想法
1. 本来做添加外挂字幕的时候准备的时 TextView 加 文件流解析成对象的方式在C代码持续回调Java方法实现显示，后来发现可以ffmpeg avfliter好像可以做
## 问题
1. 对于av_seek_frame出现的花屏问题，能否用判断keyframe来解决

2. 对于AVFrame内的pic_type 为啥没有 I B P 但keyframe却有值
   结果显示全是 AV_PICTURE_TYPE_NONE