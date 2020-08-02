package constantin.renderingx.example.stereo.distortion;

import android.annotation.SuppressLint;
import android.content.Context;

import com.google.vr.ndk.base.GvrApi;

import constantin.renderingx.core.MVrHeadsetParams;
import constantin.renderingx.core.xglview.XGLSurfaceView;


//See native code for documentation

public class RendererDistortion implements XGLSurfaceView.FullscreenRenderer {
    static {
        System.loadLibrary("example-1");
    }
    private native long nativeConstruct(Context context,long nativeGvrContext);
    private native void nativeDelete(long p);
    private native void nativeOnSurfaceCreated(long p,final Context context);
    private native void nativeOnDrawFrame(long p);

    private final Context mContext;
    private final long nativeRenderer;

    public RendererDistortion(final Context context, final GvrApi gvrApi){
        mContext=context;
        nativeRenderer=nativeConstruct(context, gvrApi.getNativeGvrContext());
    }

    @Override
    public void onContextCreated(int width,int height) {
        nativeOnSurfaceCreated(nativeRenderer,mContext);
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
}
