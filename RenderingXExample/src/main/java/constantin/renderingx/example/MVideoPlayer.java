package constantin.renderingx.example;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.Surface;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.LifecycleOwner;
import androidx.lifecycle.OnLifecycleEvent;

import constantin.renderingx.core.ISurfaceTextureAvailable;
import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoNative.VideoNative;
import constantin.video.core.VideoPlayer;

//Uses the LiveVideo10ms VideoCore lib which is intended for live streaming, not file playback.
//I recommend using android MediaPlayer if only playback from file is needed

public class MVideoPlayer implements LifecycleObserver, ISurfaceTextureAvailable {

    private final VideoPlayer videoPlayer;
    private Surface mVideoSurface;
    private final AppCompatActivity parent;

    public MVideoPlayer(final AppCompatActivity parent,final Context context, final String assetsFilename, final IVideoParamsChanged vpc){
        this.parent=parent;
        VideoNative.setVS_SOURCE(context, VideoNative.VS_SOURCE.ASSETS);
        VideoNative.setVS_ASSETS_FILENAME_TEST_ONLY(context,assetsFilename);
        VideoNative.setVS_FILE_ONLY_LIMIT_FPS(context,40);
        videoPlayer=new VideoPlayer(context,vpc);
        parent.getLifecycle().addObserver(this);
    }

    //This one is called by the OpenGL Thread !
    @Override
    public void onSurfaceTextureAvailable(final SurfaceTexture surfaceTexture) {
        final MVideoPlayer reference=this;
        //To avoid race conditions always start and stop the Video player on the UI thread
        parent.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                //If the callback gets called after the application was paused / destroyed
                //(which is possible because the callback was originally not invoked on the UI thread )
                //only create the Surface for later use. The next onResume() event will re-start the video
                mVideoSurface=new Surface(surfaceTexture);
                if(reference.parent.getLifecycle().getCurrentState().isAtLeast(Lifecycle.State.RESUMED)){
                    videoPlayer.prepare(mVideoSurface);
                    videoPlayer.addAndStartReceiver();
                }
            }
        });
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void resume(){
        if(mVideoSurface!=null){
            videoPlayer.prepare(mVideoSurface);
            videoPlayer.addAndStartReceiver();
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void pause(){
        if(mVideoSurface!=null){
            videoPlayer.stopAndRemovePlayerReceiver();
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    private void destroy(){
        if(mVideoSurface!=null){
            mVideoSurface.release();
        }
    }


}
