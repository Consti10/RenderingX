package constantin.renderingx.example.renderer3;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import com.google.vr.ndk.base.GvrApi;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.renderingx.example.VRSettingsHelper;


public class GLRTest implements GLSurfaceView.Renderer{
    static {
        System.loadLibrary("example-renderer3");
    }
    private native long nativeConstruct(Context context,float[] undistortionData,long nativeGvrContext);
    private native void nativeDelete(long p);
    private native void nativeOnSurfaceCreated(long p,final Context context);
    private native void nativeOnSurfaceChanged(long p,int width,int height);
    private native void nativeOnDrawFrame(long p);

    private final Context mContext;
    private final long nativeRenderer;

    public GLRTest(final Context context, final GvrApi gvrApi){
        mContext=context;
        nativeRenderer=nativeConstruct(context, VRSettingsHelper.getUndistortionCoeficients(context),gvrApi.getNativeGvrContext());
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeOnSurfaceCreated(nativeRenderer,mContext);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeOnSurfaceChanged(nativeRenderer,width,height);
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
