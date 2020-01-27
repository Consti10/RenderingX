package constantin.renderingx.example.supersync;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.view.Surface;

import com.google.vr.ndk.base.BufferViewport;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.sdk.base.Eye;

import constantin.renderingX.GLESInfo.GLESInfo;
import constantin.renderingX.ViewSuperSync;

public class GLRExampleSuperSync implements ViewSuperSync.IRendererSuperSync {
    static {
        System.loadLibrary("example-renderer");
    }
    private native long nativeConstruct(Context context,boolean qcomTiledRenderingAvailable, boolean reusableSyncAvailable);
    private native void nativeDelete(long glRendererStereoP);
    private native void nativeOnSurfaceCreated(long glRendererStereoP,Context androidContext);
    private native void nativeOnSurfaceChanged(long glRendererStereoP,int width,int height);
    private native void nativeEnterSuperSyncLoop(long glRendererStereoP, int exclusiveVRCore);
    private native void nativeExitSuperSyncLoop(long glRendererMonoP);
    private native void nativeDoFrame(long glRendererStereoP,long lastVsync);

    private final Context mContext;
    // Opaque native pointer to the native GLRStereoSuperSync instance.
    private final long nativeGLRSuperSync;

    public GLRExampleSuperSync(final Context context, GvrApi gvrApi){
        mContext=context;
        final boolean qcomTiledRenderingAvailable= GLESInfo.isExtensionAvailable(context, GLESInfo.GL_QCOM_tiled_rendering);
        final boolean reusableSyncAvailable=GLESInfo.isExtensionAvailable(context,GLESInfo.EGL_KHR_reusable_sync);
        nativeGLRSuperSync=nativeConstruct(context,qcomTiledRenderingAvailable,reusableSyncAvailable);
    }

    @Override
    public void onSurfaceCreated() {
        nativeOnSurfaceCreated(nativeGLRSuperSync,mContext);
    }

    @Override
    public void onSurfaceChanged(int width, int height) {
        nativeOnSurfaceChanged(nativeGLRSuperSync,width,height);
    }

    @Override
    public void enterSuperSyncLoop(final int exclusiveVRCore) {
        nativeEnterSuperSyncLoop(nativeGLRSuperSync,exclusiveVRCore);
    }


    @Override
    public void requestExitSuperSyncLoop() {
        nativeExitSuperSyncLoop(nativeGLRSuperSync);
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


