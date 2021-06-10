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
import android.widget.TextView;

import com.example.ffmpegr4.R;
import com.example.ffmpegr4.helper.Logger;
import com.example.ffmpegr4.view.MySurfaceView;

public class SurfaceViewActivity extends AppCompatActivity {
    private static final int ON_CHOOSE_VIDEOS = 10;
    MySurfaceView surfaceView;
    TextView title;
    TextView text_duration;
    Button button;
    Button btnChoose;
    String filePath;

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
        String filepath = "/sdcard/douyin2.mp4";
        surfaceView = findViewById(R.id.surface_view);
        title = findViewById(R.id.text_title);
        text_duration = findViewById(R.id.text_duration);
        button = findViewById(R.id.btn_play);
        btnChoose = findViewById(R.id.btn_choose);
       /* Logger.D(Environment.getExternalStorageDirectory().toString());
        Logger.D(Environment.getStorageDirectory().getAbsolutePath());*/
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                playVideo(filepath,surfaceView.getMyHolder().getSurface());
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
        long time_duration = getVideoDurattion(filepath);
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

    private native void playVideo(String path, Surface surface);
    private native long getVideoDurattion(String path);


}