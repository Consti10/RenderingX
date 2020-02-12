package constantin.renderingx.example.StereoVR;

import androidx.appcompat.app.AppCompatActivity;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;
import com.google.vr.sdk.base.GvrView;

import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.renderingx.core.MyVRLayout;

public class AExampleVRRendering extends AppCompatActivity {
    private GLSurfaceView gLView;
    private GLRExampleVR renderer;
    //Use one of both
    private static final boolean USE_GVR_LAYOUT=false;
    private GvrLayout gvrLayout;
    private MyVRLayout myVRLayout;

    public static final String KEY_MODE="KEY_MODE";
    //Default mode is 0 (test VDDC)

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
            renderer=new GLRExampleVR(this, gvrApi,true,
                    true,true,GLRExampleVR.SPHERE_MODE_NONE);
        }else{
            renderer=new GLRExampleVR(this, gvrApi,false,
                    true,false,MODE);
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
        renderer.onActivityResumed();
    }

    @Override
    protected void onPause(){
        super.onPause();
        gLView.onPause();
        if(gvrLayout!=null)gvrLayout.onPause();
        if(myVRLayout!=null)myVRLayout.onPauseX();
        renderer.onActivityPaused();
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

}