package com.example.ffmpegr4.activity;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.media.AudioTrack;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.view.Surface;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import com.example.ffmpegr4.R;
import com.example.ffmpegr4.helper.Logger;
import com.example.ffmpegr4.view.MySurfaceView;

public class SurfaceViewActivity extends AppCompatActivity {
    private static final int ON_CHOOSE_VIDEOS = 10;
    MySurfaceView surfaceView;
    TextView title;
    TextView text_duration;
    TextView text_video_progress;
    SeekBar seekBar;
    Button button;
    Button btnChoose;
    String filePath;
    Thread video_thread;

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

        setUI();
        setListeners(filePath);

    }

    private void setListeners(String filepath) {
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                /// TODO 这里有坑，因为没有线程时MAIN线程在跑C语言代码，在C语言代码中更新UI无效，所以这时候需要新开线程跑C语言代码
                video_thread = new Thread(){
                    @Override
                    public void run() {
                        playVideo(filepath,surfaceView.getMyHolder().getSurface(),seekBar);
                    }
                };
                video_thread.start();

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

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

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
        title = findViewById(R.id.text_title);
        seekBar = findViewById(R.id.seekbar);
        text_video_progress = findViewById(R.id.text_video_progress);
        text_duration = findViewById(R.id.text_duration);
        button = findViewById(R.id.btn_play);
        long time_duration = getVideoDurattion(filePath);
        text_duration.setText(" "+time_duration/10e5+"s");
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == ON_CHOOSE_VIDEOS){
            Logger.D(data.getData().toString());
            filePath = data.getData().toString();
        }
    }
    /// TODO extract seekbar to anther method
    private native void playVideo(String path, Surface surface,SeekBar seekBar);
    private native long getVideoDurattion(String path);



}