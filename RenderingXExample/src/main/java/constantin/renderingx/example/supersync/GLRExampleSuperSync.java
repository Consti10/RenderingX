package constantin.renderingx.example.supersync;

import android.content.Context;
import android.graphics.SurfaceTexture;

import androidx.appcompat.app.AppCompatActivity;

import com.google.vr.ndk.base.GvrApi;

import constantin.renderingx.core.MVrHeadsetParams;
import constantin.renderingx.core.VSYNC;
import constantin.renderingx.core.gles_info.Extensions;
import constantin.renderingx.core.views.ViewSuperSync;
import constantin.renderingx.core.xglview.SurfaceTextureHolder;
import constantin.video.core.gl.ISurfaceAvailable;
import constantin.video.core.gl.VideoSurfaceHolder;

//See native code for documentation
public class GLRExampleSuperSync implements ViewSuperSync.IRendererSuperSync {
    static {
        System.loadLibrary("example-supersync");
    }
    private native long nativeConstruct(Context context,long nativeGvrContext,long vsync);
    private native void nativeDelete(long glRendererStereoP);
    private native void nativeOnSurfaceCreated(long glRendererStereoP, Context androidContext,SurfaceTextureHolder surfaceTextureHolder,int width, int height);
    private native void nativeEnterSuperSyncLoop(long glRendererStereoP, int exclusiveVRCore);

    private final Context mContext;
    // Opaque native pointer to the native GLRStereoSuperSync instance.
    private final long nativeGLRSuperSync;
    private final VSYNC vsync;

    public GLRExampleSuperSync(final Context context, GvrApi gvrApi){
        mContext=context;
        vsync=new VSYNC((AppCompatActivity)context);
        nativeGLRSuperSync=nativeConstruct(context,gvrApi.getNativeGvrContext(),vsync.getNativeInstance());
    }

    @Override
    public void onContextCreated(int width, int height, SurfaceTextureHolder surfaceTextureHolder) {
        nativeOnSurfaceCreated(nativeGLRSuperSync,mContext,surfaceTextureHolder,width,height);
    }

    @Override
    public void onDrawFrame() {
        nativeEnterSuperSyncLoop(nativeGLRSuperSync,0);
    }


    @Override
    protected void finalize() throws Throwable {
        try {
            nativeDelete(nativeGLRSuperSync);
        } finally {
            super.finalize();
        }
    }

}


