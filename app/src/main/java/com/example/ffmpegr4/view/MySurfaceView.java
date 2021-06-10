package com.example.ffmpegr4.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.os.Build;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import com.example.ffmpegr4.helper.Logger;

public class MySurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    private SurfaceHolder surfaceHolder;
    private Paint paint;
    private Canvas canvas;

    public MySurfaceView(Context context) {
        super(context);
    }

    public MySurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public MySurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);

    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public MySurfaceView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    public SurfaceHolder getMyHolder(){
        return surfaceHolder;
    }
    private void init() {
        Logger.D("init holder");
        surfaceHolder = getHolder();
        surfaceHolder.addCallback(this);
        setFocusable(true);

    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        Logger.D("surface create");
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        Logger.D("surface Changed");
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        Logger.D("surface Destroyed");
    }
}
