package constantin.renderingx.example.renderer2;

import androidx.appcompat.app.AppCompatActivity;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;

import com.google.vr.ndk.base.GvrLayout;

import constantin.renderingX.core.PerformanceHelper;

public class AExampleVRRendering extends AppCompatActivity {
    private GvrLayout gvrLayout;
    private GLSurfaceView gLView;
    private GLRExampleVR renderer;

    public static final String KEY_MODE="KEY_MODE";
    //Default mode is 0 (test VDDC)

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        gvrLayout=new GvrLayout(this);

        final Bundle bundle=getIntent().getExtras();
        final int MODE=bundle==null ? 0 : bundle.getInt(KEY_MODE,0);

        gLView = new GLSurfaceView(this);
        gLView.setEGLContextClientVersion(2);
        if(MODE==0){
            renderer=new GLRExampleVR(this,gvrLayout.getGvrApi(),true,
                    true,true,false,false);
        }else if(MODE==1){
            renderer=new GLRExampleVR(this,gvrLayout.getGvrApi(),false,
                    true,false,true,false);
        }else{
            renderer=new GLRExampleVR(this,gvrLayout.getGvrApi(),false,
                    true,false,false,true);
        }
        gLView.setRenderer(renderer);
        gLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        gvrLayout.setPresentationView(gLView);
        setContentView(gvrLayout);

        System.out.println("Path is:"+ Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS));

    }

    @Override
    protected void onResume() {
        super.onResume();
        gvrLayout.onResume();
        PerformanceHelper.setImmersiveSticky(this);
    }

    @Override
    protected void onPause(){
        super.onPause();
        gvrLayout.onPause();
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        gvrLayout.shutdown();
    }
}
