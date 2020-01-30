package constantin.renderingx.example.renderer2;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.GvrViewerParams;

import java.io.IOException;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


public class GLRExampleVR implements GLSurfaceView.Renderer{
    static {
        System.loadLibrary("example-renderer2");
    }
    private native long nativeConstruct(Context context,long nativeGvrContext,boolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
                                        boolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT,boolean MESH,boolean SPHERE,boolean SPHERE_2);
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
    private final MediaPlayer mediaPlayer;

    public GLRExampleVR(final Context context, final GvrApi gvrApi, boolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
                        boolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT, boolean MESH, boolean SPHERE, boolean SPHERE2){
        mContext=context;
        mediaPlayer=new MediaPlayer();
        try {
            AssetFileDescriptor afd = context.getAssets().openFd("testvideo.mp4");
            mediaPlayer.setDataSource(afd.getFileDescriptor(),afd.getStartOffset(), afd.getLength());
        } catch (IOException e) {
            e.printStackTrace();
        }

        GvrView view=new GvrView(context);
        final GvrViewerParams params=view.getGvrViewerParams();

        nativeRenderer=nativeConstruct(context,
                gvrApi.getNativeGvrContext(),RENDER_SCENE_USING_GVR_RENDERBUFFER,RENDER_SCENE_USING_VERTEX_DISPLACEMENT,MESH,SPHERE,SPHERE2);

        float[] fov=new float[4];
        fov[0]=params.getLeftEyeMaxFov().getLeft();
        fov[1]=params.getLeftEyeMaxFov().getRight();
        fov[2]=params.getLeftEyeMaxFov().getBottom();
        fov[3]=params.getLeftEyeMaxFov().getTop();
        float[] kN=params.getDistortion().getCoefficients();

        nativeUpdateHeadsetParams(nativeRenderer,view.getScreenParams().getWidthMeters(),view.getScreenParams().getHeightMeters(),
                params.getScreenToLensDistance(),params.getInterLensDistance(),params.getVerticalAlignment().ordinal(),params.getVerticalDistanceToLensCenter(),
                fov,kN,view.getScreenParams().getWidth(),view.getScreenParams().getHeight());
        view.shutdown();
    }


    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        int[] videoTexture=new int[1];
        GLES20.glGenTextures(1, videoTexture, 0);
        final int mGLTextureVideo = videoTexture[0];
        displayTexture=new SurfaceTexture(mGLTextureVideo,false);
        videoSurface=new Surface(displayTexture);
        mediaPlayer.setSurface(videoSurface);
        mediaPlayer.setLooping(true);
        mediaPlayer.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
            @Override
            public void onPrepared(MediaPlayer mediaPlayer) {
                mediaPlayer.start();
            }
        });
        mediaPlayer.prepareAsync();
        nativeOnSurfaceCreated(nativeRenderer,mContext,mGLTextureVideo);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeOnSurfaceChanged(nativeRenderer,width,height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        displayTexture.updateTexImage();
        nativeOnDrawFrame(nativeRenderer);
    }

    public void end(){
        mediaPlayer.stop();
        mediaPlayer.release();
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
