package constantin.renderingx.example;


import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.media.MediaPlayer;
import android.view.Surface;

import java.io.IOException;

import constantin.video.core.VideoNative.VideoNative;
import constantin.video.core.VideoPlayer;

//Enables switching between Android Media Player and LiveVideo10ms VideoPlayer
//First one plays mp4, second one plays .h264

public class TestVideoPlayer {
    private final MediaPlayer mediaPlayer;
    private final VideoPlayer videoPlayer;
    private final boolean USE_ANDROID_MEDIA_PLAYER;

    public TestVideoPlayer(final Context context, final boolean USE_ANDROID_MEDIA_PLAYER, final String assetsFilename){
        this.USE_ANDROID_MEDIA_PLAYER=USE_ANDROID_MEDIA_PLAYER;
        if(USE_ANDROID_MEDIA_PLAYER){
            videoPlayer=null;
            mediaPlayer=new MediaPlayer();
            try {
                AssetFileDescriptor afd = context.getAssets().openFd(assetsFilename);
                mediaPlayer.setDataSource(afd.getFileDescriptor(),afd.getStartOffset(), afd.getLength());
            } catch (IOException e) {
                e.printStackTrace();
            }
        }else{
            mediaPlayer=null;
            VideoNative.setVS_SOURCE(context, VideoNative.VS_SOURCE.ASSETS);
            VideoNative.setVS_ASSETS_FILENAME_TEST_ONLY(context,assetsFilename);
            videoPlayer=new VideoPlayer(context,null);
        }
    }

    public void setSurfaceAndStart(final Surface surface){
        if(USE_ANDROID_MEDIA_PLAYER){
            mediaPlayer.setSurface(surface);
            mediaPlayer.setLooping(true);
            mediaPlayer.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
                @Override
                public void onPrepared(MediaPlayer mediaPlayer) {
                    mediaPlayer.start();
                }
            });
            mediaPlayer.prepareAsync();
        }else{
            videoPlayer.prepare(surface);
            videoPlayer.addAndStartReceiver();
        }
    }

    public void end(){
        if(USE_ANDROID_MEDIA_PLAYER){
            if(mediaPlayer.isPlaying()){
                mediaPlayer.stop();
                mediaPlayer.release();
            }
        }else{
            videoPlayer.stopAndRemovePlayerReceiver();
        }
    }
}
