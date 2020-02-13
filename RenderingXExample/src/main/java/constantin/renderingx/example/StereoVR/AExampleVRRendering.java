package constantin.renderingx.example.StereoVR;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.SurfaceTexture;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.view.Surface;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;
import com.google.vr.sdk.base.GvrView;

import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.ISurfaceTextureAvailable;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.renderingx.core.MyVRLayout;
import constantin.renderingx.example.MVideoPlayer;

public class AExampleVRRendering extends AppCompatActivity implements ISurfaceTextureAvailable {
    private GLSurfaceView gLView;
    private GLRExampleVR renderer;
    //Use one of both
    private static final boolean USE_GVR_LAYOUT=false;
    private GvrLayout gvrLayout;
    private MyVRLayout myVRLayout;

    public static final String KEY_MODE="KEY_MODE";
    //Default mode is 0 (test VDDC)

    //Disable video playback completely by setting videoFilename to null
    private String videoFilename=null;
    //private final TestVideoPlayer testVideoPlayer;
    private MVideoPlayer mVideoPlayer;
    public static final int SPHERE_MODE_NONE=0;
    public static final int SPHERE_MODE_GVR_EQUIRECTANGULAR=1;
    public static final int SPHERE_MODE_INSTA360_TEST=2;
    public static final int SPHERE_MODE_INSTA360_TEST2=3;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        GvrApi gvrApi;
        if(USE_GVR_LAYOUT){
            gvrLayout=new GvrLayout(this);
            gvrApi =gvrLayout.getGvrApi();
        }else{
            //displaySynchronizer=new DisplaySynchronizer(this,getWindowManager().getDefaultDisplay());
            //gvrApi=new GvrApi(this,null);
            myVRLayout=new MyVRLayout(this);
            gvrApi=myVRLayout.getGvrApi();
        }

        final Bundle bundle=getIntent().getExtras();
        final int MODE=bundle==null ? 0 : bundle.getInt(KEY_MODE,0);

        gLView = new GLSurfaceView(this);
        gLView.setEGLContextClientVersion(2);
        if(MODE==0){
            renderer=new GLRExampleVR(this, this,gvrApi,true,
                    true,true,SPHERE_MODE_NONE);
            videoFilename=null;
        }else{
            renderer=new GLRExampleVR(this,this, gvrApi,false,
                    true,false,MODE);
            //Only create video surface/ start video Player if rendering one of both spheres
            if(MODE==SPHERE_MODE_GVR_EQUIRECTANGULAR){
                videoFilename="360DegreeVideos/testRoom1_1920Mono.mp4";
            }else{
                videoFilename="360DegreeVideos/video360.h264";
            }
        }
        gLView.setRenderer(renderer);
        gLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        if(USE_GVR_LAYOUT){
            setContentView(gvrLayout);
            gvrLayout.setPresentationView(gLView);
        }else{
            setContentView(myVRLayout);
            myVRLayout.setPresentationView(gLView);
        }
        System.out.println("Path is:"+ Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS));
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(gvrLayout!=null)gvrLayout.onResume();
        if(myVRLayout!=null)myVRLayout.onResumeX();
        gLView.onResume();
    }

    @Override
    protected void onPause(){
        super.onPause();
        gLView.onPause();
        if(gvrLayout!=null)gvrLayout.onPause();
        if(myVRLayout!=null)myVRLayout.onPauseX();
        if(mVideoPlayer!=null){
            mVideoPlayer.stop();
            mVideoPlayer=null;
        }
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        if(gvrLayout!=null)gvrLayout.shutdown();
        if(myVRLayout!=null)myVRLayout.shutdown();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            FullscreenHelper.setImmersiveSticky(this);
        }
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture) {
        if(videoFilename!=null && mVideoPlayer==null){
            Surface mVideoSurface=new Surface(surfaceTexture);
            mVideoPlayer=new MVideoPlayer(this,videoFilename,mVideoSurface,null);
            mVideoPlayer.start();
        }
    }
}