package com.example.ffmpegr4.activity;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.app.Dialog;
import android.content.Intent;
import android.media.AudioTrack;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;
import android.view.Surface;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import com.example.ffmpegr4.R;
import com.example.ffmpegr4.helper.Logger;
import com.example.ffmpegr4.util.MusicPlay;
import com.example.ffmpegr4.util.PathConvertor;
import com.example.ffmpegr4.view.MySurfaceView;

import java.util.concurrent.Callable;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.FutureTask;
import java.util.concurrent.ThreadPoolExecutor;

public class SurfaceViewActivity extends AppCompatActivity {
    private static final int ON_CHOOSE_VIDEOS = 10;
    MySurfaceView surfaceView;
    TextView title;
    TextView text_duration;
    TextView text_video_progress;
    SeekBar seekBar;
    Button button;
    Button btnPause;
    Button btnChoose;
    String filePath;
    Thread video_thread;
    MusicPlay musicPlay;
    FutureTask videoTask;
    long time_duration = 0;

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avutil");
        System.loadLibrary("avcodec");
        System.loadLibrary("swresample");
        System.loadLibrary("avformat");
        System.loadLibrary("swscale");
        System.loadLibrary("avfilter");
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_surface_view);
        filePath = "/storage/emulated/0/douyin2.mp4";
        musicPlay = new MusicPlay();
        setUI();
        setListeners();


    }

    private void setListeners() {
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                videoTask = new FutureTask(new Runnable() {
                    @Override
                    public void run() {
                        /// TODO 这里有坑，因为没有线程时MAIN线程在跑C语言代码，在C语言代码中更新UI无效，所以这时候需要新开线程跑C语言代码
                        video_thread = new Thread(){
                            @Override
                            public void run() {
                                playVideoWithMusic(filePath,surfaceView.getMyHolder().getSurface(),seekBar,musicPlay);
                                //playVideo(filePath,surfaceView.getMyHolder().getSurface(),seekBar);
                              //  playWithAss(filepath,surfaceView.getMyHolder().getSurface(),seekBar,"/sdcard/zimu.ass");
                            }
                        };
                        video_thread.start();
                    }
                },null);
                Thread thread = new Thread(videoTask);
                thread.start();
            }
        });
        btnPause.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        pausePlay();
                    }
                }).start();
            }
        });
        btnChoose.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                Intent intent = new Intent(Intent.ACTION_PICK, MediaStore.Video.Media.EXTERNAL_CONTENT_URI);
                intent.setDataAndType(MediaStore.Video.Media.EXTERNAL_CONTENT_URI,"video/*");
                startActivityForResult(intent,ON_CHOOSE_VIDEOS);
            }
        });
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                text_video_progress.setText(""+progress+" %");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                pausePlay();
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                Callable<Integer> runnable = new Callable() {
                    @Override
                    public Object call() throws Exception {
                        resumePlay(seekBar.getProgress(),filePath,surfaceView.getMyHolder().getSurface(),seekBar);
                        return null;
                    }
                };
                videoTask = new FutureTask<>(runnable);
                Thread thread = new Thread(videoTask);
                thread.start();
            }
        });
    }

    @Override
    protected void onPause() {
        super.onPause();
        // TODO 关闭线程
    }

    private void setUI() {
        btnChoose = findViewById(R.id.btn_choose);
        surfaceView = findViewById(R.id.surface_view);
        btnPause = findViewById(R.id.btn_Pause);
        title = findViewById(R.id.text_title);
        seekBar = findViewById(R.id.seekbar);
        text_video_progress = findViewById(R.id.text_video_progress);
        text_duration = findViewById(R.id.text_duration);
        button = findViewById(R.id.btn_play);
        time_duration = getVideoDurattion(filePath);
        text_duration.setText(" "+time_duration/10e5+"s");
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == ON_CHOOSE_VIDEOS) {
            if (data != null) {
                Logger.D(data.getData().toString());
                filePath = PathConvertor.utilhandler(this, data);
                time_duration = getVideoDurattion(filePath);
                text_duration.setText(" "+time_duration/10e5+"s");
            }
        }
    }
    /// TODO extract seekbar to anther method
    private native void playVideo(String path, Surface surface,SeekBar seekBar);
    private native long getVideoDurattion(String path);
    private native void pausePlay();
    private native void resumePlay(int num,String filePath, Surface surface,SeekBar seekBar);
    private native void playWithAss(String path, Surface surface, SeekBar seekBar, String assPath);
    private native void playVideoWithMusic(String path, Surface surface, SeekBar seekBar, MusicPlay mediaplay);
}