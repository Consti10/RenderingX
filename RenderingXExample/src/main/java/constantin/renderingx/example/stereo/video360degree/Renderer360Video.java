package constantin.renderingx.example.stereo.video360degree;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.SurfaceTexture;

import androidx.appcompat.app.AppCompatActivity;

import com.google.vr.ndk.base.GvrApi;

import constantin.renderingx.core.xglview.GLContextSurfaceLess;
import constantin.renderingx.core.xglview.SurfaceTextureHolder;
import constantin.renderingx.core.xglview.XGLSurfaceView;

//See native code for documentation

public class Renderer360Video implements XGLSurfaceView.FullscreenRendererWithSurfaceTexture, GLContextSurfaceLess.SecondarySharedContext {
    static {
        System.loadLibrary("example-2");
    }
    private native long nativeConstruct(Context context,long nativeGvrContext,int SPHERE_MODE);
    private native void nativeDelete(long p);
    private native void nativeOnSurfaceCreated(long p, final Context context,SurfaceTextureHolder surfaceTextureHolder);
    private native void nativeOnDrawFrame(long p);
    private native void nativeOnSecondaryContextCreated(long nativePointer,final Context context);
    private native void nativeOnSecondaryContextDoWork(long nativePointer);

    private final Context mContext;
    private final long nativeRenderer;

    @SuppressLint("ApplySharedPref")
    public Renderer360Video(final AppCompatActivity context,final GvrApi gvrApi, int SPHERE_MODE){
        mContext=context;
        nativeRenderer=nativeConstruct(context, gvrApi.getNativeGvrContext(),SPHERE_MODE);
    }

    @Override
    public void onContextCreated(int width,int height,final SurfaceTextureHolder surfaceTextureHolder) {
        nativeOnSurfaceCreated(nativeRenderer,mContext,surfaceTextureHolder);
    }

    @Override
    public void onDrawFrame() {
        nativeOnDrawFrame(nativeRenderer);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            nativeDelete(nativeRenderer);
        } finally {
            super.finalize();
        }
    }

    @Override
    public void onSecondaryContextCreated() {
        nativeOnSecondaryContextCreated(nativeRenderer,mContext);
    }

    @Override
    public void onSecondaryContextDoWork() {
        nativeOnSecondaryContextDoWork(nativeRenderer);
    }
}
