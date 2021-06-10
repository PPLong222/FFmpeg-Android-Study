package com.example.ffmpegr4.activity;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.VideoView;

import com.example.ffmpegr4.R;

public class VideoActivity extends AppCompatActivity {
    private VideoView videoView;
    Button button;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);
        videoView = findViewById(R.id.videovidew);
        videoView.setVideoPath("content://media/external/video/media/291607");
        button = findViewById(R.id.btn_vplay);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                videoView.start();
            }
        });
    }
}