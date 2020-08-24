package constantin.renderingx.core.xglview;

import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.view.Surface;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;

import constantin.video.core.gl.ISurfaceTextureAvailable;

//TODO figure out if this should react to the lifecycle
public class SurfaceTextureHolder implements LifecycleObserver {
    private final AppCompatActivity parent;
    private final ISurfaceTextureAvailable iSurfaceTextureAvailable;
    private int mGLTextureVideo;
    private SurfaceTexture surfaceTexture;
    private Surface surface=null;

    public SurfaceTextureHolder(final AppCompatActivity parent,final ISurfaceTextureAvailable iSurfaceTextureAvailable){
        this.parent=parent;
        this.iSurfaceTextureAvailable=iSurfaceTextureAvailable;
        parent.getLifecycle().addObserver(this);
    }

    public int getTextureId(){
        return mGLTextureVideo;
    }
    public SurfaceTexture getSurfaceTexture(){
        return surfaceTexture;
    }

    public void createOnOpenGLThread(){
        int[] videoTexture=new int[1];
        GLES20.glGenTextures(1, videoTexture, 0);

        mGLTextureVideo = videoTexture[0];
        // Unfortunately the SurfaceTexture constructor that does not take a OpenGL texture id is only available
        // on api 26 and above
        surfaceTexture=new SurfaceTexture(mGLTextureVideo,false);

        parent.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                surface=new Surface(surfaceTexture);
                if(SurfaceTextureHolder.this.parent.getLifecycle().getCurrentState().isAtLeast(Lifecycle.State.STARTED)){
                    iSurfaceTextureAvailable.surfaceTextureCreated(surfaceTexture,surface);
                }
            }
        });
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_START)
    private void start(){
        if(surface!=null){
            iSurfaceTextureAvailable.surfaceTextureCreated(surfaceTexture,surface);
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_STOP)
    private void stop(){
        if(surface!=null){
            iSurfaceTextureAvailable.surfaceTextureDestroyed();
        }
    }

    public void destroyOnUiThread(){
        surfaceTexture.release();
        surface.release();
        //iSurfaceTextureAvailable.surfaceTextureDestroyed();
    }
}
