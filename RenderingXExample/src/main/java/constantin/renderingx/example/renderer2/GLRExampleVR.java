package constantin.renderingx.example.renderer2;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.GvrViewerParams;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.renderingx.core.MyVrHeadsetParams;
import constantin.renderingx.example.TestVideoPlayer;

// !
// At the time, pausing / resuming the video is not implemented and will result in an app crash !
// !
//See native code for documentation

public class GLRExampleVR implements GLSurfaceView.Renderer{
    public static final int SPHERE_MODE_NONE=0;
    public static final int SPHERE_MODE_GVR_EQUIRECTANGULAR=1;
    public static final int SPHERE_MODE_INSTA360_TEST=2;
    public static final int SPHERE_MODE_INSTA360_TEST2=3;
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
    //initialized when Surface ready
    private SurfaceTexture displayTexture=null;
    private Surface videoSurface;
    //Disable video playback completely
    private final boolean PLAY_VIDEO;
    private final TestVideoPlayer testVideoPlayer;

    @SuppressLint("ApplySharedPref")
    public GLRExampleVR(final Context context, final GvrApi gvrApi, boolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
                        boolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT, boolean MESH,int SPHERE_MODE){
        mContext=context;
        //Only create video surface/ start video Player if rendering one of both spheres
        PLAY_VIDEO=SPHERE_MODE!=SPHERE_MODE_NONE;
        if(PLAY_VIDEO){
            //Switch between Android Media player and LiveVideo10ms Core Video player
            if(SPHERE_MODE==SPHERE_MODE_GVR_EQUIRECTANGULAR){
                //testVideoPlayer=new TestVideoPlayer(context,true,"360DegreeVideos/testRoom1_1920Mono.mp4");
                //testVideoPlayer=new TestVideoPlayer(context,true,"360DegreeVideos/paris_by_diego.mp4");
                testVideoPlayer=new TestVideoPlayer(context,true,"360DegreeVideos/testRoom1_1920Mono.mp4");
            }else{
                //testVideoPlayer=new TestVideoPlayer(context,false,"360DegreeVideos/360_test.h264");
                testVideoPlayer=new TestVideoPlayer(context,false,"360DegreeVideos/video360.h264");
            }
        }else{
            testVideoPlayer=null;
        }

        nativeRenderer=nativeConstruct(context,
                gvrApi.getNativeGvrContext(),RENDER_SCENE_USING_GVR_RENDERBUFFER,RENDER_SCENE_USING_VERTEX_DISPLACEMENT,MESH,SPHERE_MODE);

        final MyVrHeadsetParams params=new MyVrHeadsetParams(context);
        nativeUpdateHeadsetParams(nativeRenderer,params.ScreenWidthMeters,params.ScreenHeightMeters,
                params.ScreenToLensDistance,params.InterLensDistance,params.VerticalAlignment,params.VerticalDistanceToLensCenter,
                params.fov,params.kN,params.ScreenWidthPixels,params.ScreenHeightPixels);
    }


    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        int mGLTextureVideo=0;
        if(PLAY_VIDEO){
            int[] videoTexture=new int[1];
            GLES20.glGenTextures(1, videoTexture, 0);
            mGLTextureVideo = videoTexture[0];
            displayTexture=new SurfaceTexture(mGLTextureVideo,false);
            videoSurface=new Surface(displayTexture);
            testVideoPlayer.setSurfaceAndStart(videoSurface);
        }
        nativeOnSurfaceCreated(nativeRenderer,mContext,mGLTextureVideo);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeOnSurfaceChanged(nativeRenderer,width,height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        if(PLAY_VIDEO)displayTexture.updateTexImage();
        nativeOnDrawFrame(nativeRenderer);
    }

    public void onPause(){

    }

    public void onResume(){

    }

    public void end(){
        if(PLAY_VIDEO)testVideoPlayer.end();
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
