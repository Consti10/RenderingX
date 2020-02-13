package constantin.renderingx.example.StereoVR;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import androidx.annotation.Nullable;

import com.google.vr.ndk.base.GvrApi;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.renderingx.core.IActivityPauseResume;
import constantin.renderingx.core.ISurfaceTextureAvailable;
import constantin.renderingx.core.MyVrHeadsetParams;
import constantin.renderingx.example.MVideoPlayer;


//See native code for documentation

public class GLRExampleVR implements GLSurfaceView.Renderer {
    static {
        System.loadLibrary("example-renderer2");
    }
    private native long nativeConstruct(Context context,long nativeGvrContext,boolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
                                        boolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT,boolean MESH,int SPHERE_MODE);
    private native void nativeDelete(long p);
    private native void nativeOnSurfaceCreated(long p,final Context context,int videoTexture);
    private native void nativeOnSurfaceChanged(long p,int width,int height);
    private native void nativeOnDrawFrame(long p);
    private native void nativeUpdateHeadsetParams(long nativePointer,float screen_width_meters,
                                                  float screen_height_meters,
                                                  float screen_to_lens_distance,
                                                  float inter_lens_distance,
                                                  int vertical_alignment,
                                                  float tray_to_lens_distance,
                                                  float[] device_fov_left,
                                                  float[] radial_distortion_params,
                                                  int screenWidthP,int screenHeightP);

    private final Context mContext;
    private final long nativeRenderer;
    //When iSurfaceTextureAvailable!= null create a video surface texture and then call the interface
    //Else, no video surface texture is created and the callback is not called
    private final @Nullable ISurfaceTextureAvailable iSurfaceTextureAvailable;
    private @Nullable SurfaceTexture displayTexture=null;

    @SuppressLint("ApplySharedPref")
    public GLRExampleVR(final Context context,@Nullable final ISurfaceTextureAvailable iSurfaceTextureAvailable, final GvrApi gvrApi, boolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
                        boolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT, boolean MESH,int SPHERE_MODE){
        mContext=context;
        this.iSurfaceTextureAvailable=iSurfaceTextureAvailable;

        nativeRenderer=nativeConstruct(context,
                gvrApi.getNativeGvrContext(),RENDER_SCENE_USING_GVR_RENDERBUFFER,RENDER_SCENE_USING_VERTEX_DISPLACEMENT,MESH,SPHERE_MODE);

        final MyVrHeadsetParams params=new MyVrHeadsetParams(context);
        nativeUpdateHeadsetParams(nativeRenderer,params.ScreenWidthMeters,params.ScreenHeightMeters,
                params.ScreenToLensDistance,params.InterLensDistance,params.VerticalAlignment,params.VerticalDistanceToLensCenter,
                params.fov,params.kN,params.ScreenWidthPixels,params.ScreenHeightPixels);
    }


    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        final int mGLTextureVideo;
        if(iSurfaceTextureAvailable!=null){
            int[] videoTexture=new int[1];
            GLES20.glGenTextures(1, videoTexture, 0);
            mGLTextureVideo = videoTexture[0];
            displayTexture=new SurfaceTexture(mGLTextureVideo,false);
            iSurfaceTextureAvailable.onSurfaceTextureAvailable(displayTexture);
        }else{
            mGLTextureVideo=0;
        }
        nativeOnSurfaceCreated(nativeRenderer,mContext,mGLTextureVideo);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeOnSurfaceChanged(nativeRenderer,width,height);
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
