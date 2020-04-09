package constantin.renderingx.example.StereoVR.Example360Video;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import androidx.annotation.Nullable;

import com.google.vr.ndk.base.GvrApi;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.renderingx.core.MVrHeadsetParams;
import constantin.video.core.ISurfaceTextureAvailable;


//See native code for documentation

public class Renderer360Video implements GLSurfaceView.Renderer {
    static {
        System.loadLibrary("example-2");
    }
    private native long nativeConstruct(Context context,long nativeGvrContext,boolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
                                        boolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT,int SPHERE_MODE);
    private native void nativeDelete(long p);
    private native void nativeOnSurfaceCreated(long p,final Context context,int videoTexture);
    private native void nativeOnDrawFrame(long p);
    private native void nativeUpdateHeadsetParams(long nativePointer,final MVrHeadsetParams p);

    private final Context mContext;
    private final long nativeRenderer;
    private final ISurfaceTextureAvailable iSurfaceTextureAvailable;
    private SurfaceTexture displayTexture=null;

    @SuppressLint("ApplySharedPref")
    public Renderer360Video(final Context context, @Nullable final ISurfaceTextureAvailable iSurfaceTextureAvailable, final GvrApi gvrApi, boolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
                            boolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT,int SPHERE_MODE){
        mContext=context;
        this.iSurfaceTextureAvailable=iSurfaceTextureAvailable;
        nativeRenderer=nativeConstruct(context,
                gvrApi.getNativeGvrContext(),RENDER_SCENE_USING_GVR_RENDERBUFFER,RENDER_SCENE_USING_VERTEX_DISPLACEMENT,SPHERE_MODE);
        final MVrHeadsetParams params=new MVrHeadsetParams(context);
        nativeUpdateHeadsetParams(nativeRenderer,params);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        int[] videoTexture=new int[1];
        GLES20.glGenTextures(1, videoTexture, 0);
        final int mGLTextureVideo = videoTexture[0];
        displayTexture=new SurfaceTexture(mGLTextureVideo,false);
        iSurfaceTextureAvailable.onSurfaceTextureAvailable(displayTexture);
        nativeOnSurfaceCreated(nativeRenderer,mContext,mGLTextureVideo);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        //Nothing
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        if(displayTexture!=null)displayTexture.updateTexImage();
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
