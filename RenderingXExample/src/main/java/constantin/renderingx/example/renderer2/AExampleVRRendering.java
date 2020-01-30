package constantin.renderingx.example.renderer2;

import androidx.appcompat.app.AppCompatActivity;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.widget.FrameLayout;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;

import constantin.renderingx.core.MyVRLayout;
import constantin.renderingx.core.PerformanceHelper;
import constantin.renderingx.example.R;

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
                    true,true,false,false);
        }else if(MODE==1){
            renderer=new GLRExampleVR(this, gvrApi,false,
                    true,false,true,false);
        }else{
            renderer=new GLRExampleVR(this, gvrApi,false,
                    true,false,false,true);
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
    }

    @Override
    protected void onPause(){
        super.onPause();
        if(gvrLayout!=null)gvrLayout.onPause();
        if(myVRLayout!=null)myVRLayout.onPauseX();
        renderer.end();
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
            PerformanceHelper.setImmersiveSticky(this);
        }
    }

}
