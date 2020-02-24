package constantin.renderingx.example.StereoVR;

import androidx.appcompat.app.AppCompatActivity;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;

import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.MyVRLayout;
import constantin.video.core.VideoPlayerSurfaceTexture;

//Uses the LiveVideo10ms VideoCore lib which is intended for live streaming, not file playback.
//I recommend using android MediaPlayer if only playback from file is needed

//See native code (renderer) for documentation
public class AExampleVRRendering extends AppCompatActivity {
    private static final String TAG="AExampleVRRendering";
    public static final int SPHERE_MODE_NONE=0;
    public static final int SPHERE_MODE_GVR_EQUIRECTANGULAR=1;
    public static final int SPHERE_MODE_INSTA360_TEST=2;
    public static final int SPHERE_MODE_INSTA360_TEST2=3;
    private GLSurfaceView gLView;
    //Use one of both, either GvrLayout or MyVRLayout
    private static final boolean USE_GVR_LAYOUT=false;
    private GvrLayout gvrLayout;
    private MyVRLayout myVRLayout;
    //Default mode is 0 (test VDDC)
    public static final String KEY_MODE="KEY_MODE";
    //Disable video playback completely by leaving VideoPlayerSurfaceTexture uninitialized and
    //passing null to the ISurfaceTextureCreated
    private VideoPlayerSurfaceTexture mVideoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final GvrApi gvrApi;
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
        final GLRExampleVR renderer;
        if(MODE==0){
            renderer =new GLRExampleVR(this, null,gvrApi,true,
                    true,true,SPHERE_MODE_NONE);
        }else{
            final String videoFilename;
            if(MODE==SPHERE_MODE_GVR_EQUIRECTANGULAR){
                videoFilename="360DegreeVideos/testRoom1_1920Mono.mp4";
            }else{
                videoFilename="360DegreeVideos/mjpeg_test.h264";
                //videoFilename="360DegreeVideos/insta_webbn_1_shortened.h264";
            }
            //Only create video surface/ start video Player if rendering one of both spheres
            mVideoPlayer=new VideoPlayerSurfaceTexture(this,null,videoFilename);
            renderer =new GLRExampleVR(this,mVideoPlayer, gvrApi,false,
                    true,false,MODE);
        }
        gLView.setRenderer(renderer);
        gLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        gLView.setPreserveEGLContextOnPause(true);

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
        gLView.onResume();
    }

    @Override
    protected void onPause(){
        super.onPause();
        gLView.onPause();
        if(gvrLayout!=null)gvrLayout.onPause();
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        if(gvrLayout!=null)gvrLayout.shutdown();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            FullscreenHelper.setImmersiveSticky(this);
        }
    }

}