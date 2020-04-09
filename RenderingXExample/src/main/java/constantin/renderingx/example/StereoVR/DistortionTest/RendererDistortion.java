package constantin.renderingx.example.StereoVR.DistortionTest;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import androidx.annotation.Nullable;

import com.google.vr.ndk.base.GvrApi;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.renderingx.core.MyVrHeadsetParams;
import constantin.video.core.ISurfaceTextureAvailable;


//See native code for documentation

public class RendererDistortion implements GLSurfaceView.Renderer {
    static {
        System.loadLibrary("example-1");
    }
    private native long nativeConstruct(Context context,long nativeGvrContext);
    private native void nativeDelete(long p);
    private native void nativeOnSurfaceCreated(long p,final Context context);
    private native void nativeOnDrawFrame(long p);
    private native void nativeUpdateHeadsetParams(long nativePointer,final MyVrHeadsetParams p);

    private final Context mContext;
    private final long nativeRenderer;

    @SuppressLint("ApplySharedPref")
    public RendererDistortion(final Context context, final GvrApi gvrApi){
        mContext=context;
        nativeRenderer=nativeConstruct(context,
                gvrApi.getNativeGvrContext());
        final MyVrHeadsetParams params=new MyVrHeadsetParams(context);
        nativeUpdateHeadsetParams(nativeRenderer,params);
    }


    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeOnSurfaceCreated(nativeRenderer,mContext);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        // In VR, the GLSurface is always as big as the screen
    }

    @Override
    public void onDrawFrame(GL10 gl) {
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
