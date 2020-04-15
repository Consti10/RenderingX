package constantin.renderingx.example;

import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.view.Surface;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;

import constantin.video.core.ISurfaceTextureAvailable;
import constantin.video.core.IVideoParamsChanged;

/**
 * This class uses the android MediaPlayer lib instead of VideoCore from https://github.com/Consti10/LiveVideo10ms
 */
public class XVideoPlayerSurfaceTexture implements LifecycleObserver, ISurfaceTextureAvailable {
    private final MediaPlayer mediaPlayer;
    //Used for Android lifecycle and executing callback on the UI thread
    private final AppCompatActivity parent;
    //null in the beginning, becomes valid in the future via onSurfaceTextureAvailable
    //(Constructor of Surface takes SurfaceTexture)
    private Surface mVideoSurface;

    //TODO doesnt work on mp4 assets because of compression
    public XVideoPlayerSurfaceTexture(final AppCompatActivity parent,final String MediaPlayerDataSource){
        this.parent=parent;
        mediaPlayer=MediaPlayer.create(parent,R.raw.test_room1_1920mono);
        //already prepared !
        parent.getLifecycle().addObserver(this);
    }

    public void setIVideoParamsChanged(final IVideoParamsChanged vpc){
        //Not supported
    }

    //This one is called by the OpenGL Thread !
    @Override
    public void onSurfaceTextureAvailable(final SurfaceTexture surfaceTexture) {
        //To avoid race conditions always start and stop the Video player on the UI thread
        parent.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                //If the callback gets called after the application was paused / destroyed
                //(which is possible because the callback was originally not invoked on the UI thread )
                //only create the Surface for later use. The next onResume() event will re-start the video
                mVideoSurface=new Surface(surfaceTexture);
                if(XVideoPlayerSurfaceTexture.this.parent.getLifecycle().getCurrentState().isAtLeast(Lifecycle.State.RESUMED)){
                    mediaPlayer.setSurface(mVideoSurface);
                    mediaPlayer.start();
                }
            }
        });
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void resume(){
        if(mVideoSurface!=null){
            mediaPlayer.setSurface(mVideoSurface);
            mediaPlayer.start();
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void pause(){
        if(mediaPlayer.isLooping()){
           mediaPlayer.pause();
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    private void destroy(){
        if(mVideoSurface!=null){
            mVideoSurface.release();
        }
    }


}
