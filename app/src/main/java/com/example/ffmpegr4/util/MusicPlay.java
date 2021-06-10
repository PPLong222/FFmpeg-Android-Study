package com.example.ffmpegr4.util;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class MusicPlay {
    public native void playSound(String input);
    private AudioTrack audioTrack;
    public void createTrack(int samplerRateInHz, int nb_channels){
        int channelCOnfig;
        if(nb_channels ==1){
            channelCOnfig = AudioFormat.CHANNEL_OUT_MONO;
        }else if (nb_channels == 2){
            channelCOnfig = AudioFormat.CHANNEL_OUT_STEREO;
        }else{
            channelCOnfig = AudioFormat.CHANNEL_OUT_MONO;
        }
        int buffersize = AudioTrack.getMinBufferSize(samplerRateInHz,
                channelCOnfig,AudioFormat.ENCODING_PCM_16BIT);
        audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,samplerRateInHz,
                channelCOnfig,AudioFormat.ENCODING_PCM_16BIT,buffersize,AudioTrack.MODE_STREAM);
        audioTrack.play();
    }
    public void playTrack(byte[] buffer, int length){
        if(audioTrack!=null && audioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING){
            audioTrack.write(buffer,0,length);
        }
    }

    public AudioTrack getAudioTrack() {
        return audioTrack;
    }
}