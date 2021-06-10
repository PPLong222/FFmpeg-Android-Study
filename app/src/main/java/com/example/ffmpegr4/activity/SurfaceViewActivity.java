package com.example.ffmpegr4.activity;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
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
    TextView textView;
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

        surfaceView = findViewById(R.id.surface_view);
        textView = findViewById(R.id.text_title);
        button = findViewById(R.id.btn_play);
        btnChoose = findViewById(R.id.btn_choose);
        Uri uri = Uri.parse("android:resource://"+getResources().getResourcePackageName(R.raw.ig)+"/"+R.raw.ig);
        Logger.D(Environment.getExternalStorageDirectory().toString());
        Logger.D(Environment.getStorageDirectory().getAbsolutePath());
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                playVideo("/sdcard/DCIM/Camera/VID_20190706_084618.mp4",surfaceView.getMyHolder().getSurface());
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


}