package constantin.renderingx.core.xglview;

import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.view.Surface;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;

public class SurfaceTextureHolder {
    private final AppCompatActivity parent;
    private final ISurfaceTextureAvailable iSurfaceTextureAvailable;
    private int mGLTextureVideo;
    private SurfaceTexture surfaceTexture;
    private Surface surface;

    public interface ISurfaceTextureAvailable{
        // Called when the SurfaceTexture (and Surface) was created on the OpenGL Thread
        // However, the callback is run on the UI Thread to ease synchronization
        void surfaceTextureCreated(final SurfaceTexture surfaceTexture, final Surface surface);
        // Called when the SurfaceTexture (and Surface) are about to be deleted. After this call
        // returns, both SurfaceTexture and Surface are invalid
        void surfaceTextureDestroyed();
    }

    public SurfaceTextureHolder(final AppCompatActivity parent,final ISurfaceTextureAvailable iSurfaceTextureAvailable){
        this.parent=parent;
        this.iSurfaceTextureAvailable=iSurfaceTextureAvailable;
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

        surface=new Surface(surfaceTexture);

        parent.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if(SurfaceTextureHolder.this.parent.getLifecycle().getCurrentState().isAtLeast(Lifecycle.State.RESUMED)){
                    iSurfaceTextureAvailable.surfaceTextureCreated(surfaceTexture,surface);
                }
            }
        });
    }

    public void destroyOnUiThread(){
        surfaceTexture.release();
        surface.release();
        iSurfaceTextureAvailable.surfaceTextureDestroyed();
    }
}
