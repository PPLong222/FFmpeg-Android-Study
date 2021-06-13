package com.example.ffmpegr4.activity;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.example.ffmpegr4.R;
import com.example.ffmpegr4.helper.Logger;
import com.example.ffmpegr4.util.MusicPlay;

public class AudioActivity extends AppCompatActivity {
    Button btnAudio;
    Thread audioThread;
    volatile boolean isExit = false;
    MusicPlay musicPlay;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio);
        btnAudio = findViewById(R.id.btn_playSound);
        musicPlay = new MusicPlay();
        btnAudio.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                audioThread= new Thread(){
                    @Override
                    public void run() {
                        Logger.D("RUN");
                        musicPlay.playSound("/storage/emulated/0/douyin2.mp4");
                        Logger.D("END");
                    }
                };
                audioThread.start();
            }
        });
    }

    @Override
    protected void onPause() {
        super.onPause();
        musicPlay.getAudioTrack().stop();
    }
}