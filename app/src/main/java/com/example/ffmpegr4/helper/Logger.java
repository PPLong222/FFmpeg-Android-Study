package com.example.ffmpegr4.helper;

import android.util.Log;

public class Logger {
    public static final String LOGGER_INFO = "MY_LOGGER";
    public static void D(String string){
        Log.d(LOGGER_INFO, "-----------"+string+"-----------");
    }
}
