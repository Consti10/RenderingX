package constantin.renderingx.example.supersync;

import android.content.Context;

import com.google.vr.ndk.base.GvrApi;

import constantin.renderingx.core.gles_info.Extensions;
import constantin.renderingx.core.views.ViewSuperSync;

//See native code for documentation
public class GLRExampleSuperSync implements ViewSuperSync.IRendererSuperSync {
    static {
        System.loadLibrary("example-supersync");
    }
    private native long nativeConstruct(Context context,boolean qcomTiledRenderingAvailable, boolean reusableSyncAvailable);
    private native void nativeDelete(long glRendererStereoP);
    private native void nativeOnSurfaceCreated(long glRendererStereoP,Context androidContext,int width,int height);
    private native void nativeEnterSuperSyncLoop(long glRendererStereoP, int exclusiveVRCore);
    private native void nativeExitSuperSyncLoop(long glRendererMonoP);
    private native void nativeDoFrame(long glRendererStereoP,long lastVsync);

    private final Context mContext;
    // Opaque native pointer to the native GLRStereoSuperSync instance.
    private final long nativeGLRSuperSync;

    public GLRExampleSuperSync(final Context context, GvrApi gvrApi){
        mContext=context;
        final boolean qcomTiledRenderingAvailable= Extensions.available(context,Extensions.GL_QCOM_tiled_rendering);
        final boolean reusableSyncAvailable= Extensions.available(context,Extensions.EGL_KHR_reusable_sync);
        nativeGLRSuperSync=nativeConstruct(context,qcomTiledRenderingAvailable,reusableSyncAvailable);
    }

    @Override
    public void onContextCreated(int width, int height) {
        nativeOnSurfaceCreated(nativeGLRSuperSync,mContext,width,height);
    }

    @Override
    public void onDrawFrame() {
        nativeEnterSuperSyncLoop(nativeGLRSuperSync,0);
    }

    @Override
    public void setLastVSYNC(long lastVSYNC) {
        nativeDoFrame(nativeGLRSuperSync,lastVSYNC);
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


