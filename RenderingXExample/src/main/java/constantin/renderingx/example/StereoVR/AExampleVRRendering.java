package constantin.renderingx.example.StereoVR;

import androidx.appcompat.app.AppCompatActivity;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.view.KeyEvent;
import android.view.MotionEvent;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;
import com.google.vr.sdk.base.GvrView;

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
    public static final String KEY_SPHERE_MODE ="KEY_SPHERE_MODE";
    public static final String KEY_VIDEO_FILENAME="KEY_VIDEO_FILENAME";
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
        final int SPHERE_MODE=bundle==null ? 0 : bundle.getInt(KEY_SPHERE_MODE,0);
        gLView = new GLSurfaceView(this);
        gLView.setEGLContextClientVersion(2);
        final GLRExampleVR renderer;
        if(SPHERE_MODE==SPHERE_MODE_NONE){
            //When SPHERE_MODE==SPHERE_MODE_NONE it means we do the VDDC test mesh
            renderer =new GLRExampleVR(this, null,gvrApi,true,
                    true,true,SPHERE_MODE_NONE);
        }else{
            final String VIDEO_FILENAME=bundle.getString(KEY_VIDEO_FILENAME);
            //Only create video surface/ start video Player if rendering one of both spheres
            mVideoPlayer=new VideoPlayerSurfaceTexture(this,null,VIDEO_FILENAME);
            renderer =new GLRExampleVR(this,mVideoPlayer, gvrApi,false,
                    true,false,SPHERE_MODE);
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

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        //Some VR headsets use a clamp to hold the phone in place. This clamp may press against the volume up/down buttons.
        if(event.getKeyCode()==KeyEvent.KEYCODE_VOLUME_DOWN || event.getKeyCode()==KeyEvent.KEYCODE_VOLUME_UP){
            return true;
        }
        if(event.getKeyCode()==KeyEvent.KEYCODE_BACK){
            return true;
        }
        return super.dispatchKeyEvent(event);
    }

}