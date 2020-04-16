package constantin.renderingx.core.video;

import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.view.Surface;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;

// This class simplifies starting / stopping a Video player in sync with the applications lifecycle
// (e.g. pause player in onPause , resume player in onResume() )
// But also handles the edge case when onResume() is called but the OpenGL thread has not
// created the SurfaceTexture yet

public class VideoSurfaceHolder implements LifecycleObserver {
    private final AppCompatActivity parent;
    private final ISurfaceAvailable iSurfaceAvailable;
    //These members are created on the OpenGL thread
    private int mGLTextureVideo;
    private SurfaceTexture surfaceTexture;
    //This surface is created/ destroyed on the UI thread
    private Surface surface;

    public VideoSurfaceHolder(final AppCompatActivity activity, final ISurfaceAvailable iSurfaceAvailable){
        this.parent =activity;
        this.iSurfaceAvailable=iSurfaceAvailable;
        activity.getLifecycle().addObserver(this);
    }

    public int getTextureId(){
        return mGLTextureVideo;
    }

    public SurfaceTexture getSurfaceTexture(){
        return surfaceTexture;
    }

    // Call this on the OpenGl thread, e.g. in onSurfaceCreated()
    public void createSurfaceTextureGL(){
        int[] videoTexture=new int[1];
        GLES20.glGenTextures(1, videoTexture, 0);
        mGLTextureVideo = videoTexture[0];
        surfaceTexture=new SurfaceTexture(mGLTextureVideo,false);
        parent.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                //If the callback gets called after the application was paused / destroyed
                //(which is possible because the callback was originally not invoked on the UI thread )
                //only create the Surface for later use. The next onResume() event will re-start the video
                surface=new Surface(surfaceTexture);
                if(VideoSurfaceHolder.this.parent.getLifecycle().getCurrentState().isAtLeast(Lifecycle.State.RESUMED)){
                    iSurfaceAvailable.start(surfaceTexture,surface);
                }
            }
        });
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void resume(){
        if(surface!=null){
            iSurfaceAvailable.start(surfaceTexture,surface);
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void pause(){
        if(surface!=null){
            iSurfaceAvailable.stop();
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    private void destroy(){
        if(surface!=null){
            surface.release();
        }
    }

}