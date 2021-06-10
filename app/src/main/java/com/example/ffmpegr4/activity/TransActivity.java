package com.example.ffmpegr4.activity;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.example.ffmpegr4.R;

public class TransActivity extends AppCompatActivity {
    Button btn_start_trans;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_trans);
        btn_start_trans = findViewById(R.id.btn_start_trans);
        btn_start_trans.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

            }
        });
    }
    private native void transVideo(String path,String outout);
}