package constantin.renderingx.example.renderer2;

import androidx.appcompat.app.AppCompatActivity;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;

import com.google.vr.ndk.base.GvrLayout;
import com.google.vr.sdk.base.Distortion;

import constantin.renderingX.PerformanceHelper;
import constantin.renderingx.example.R;
import constantin.renderingx.example.renderer1.GLRExample;

public class TestActivity2 extends AppCompatActivity {
    private GvrLayout gvrLayout;
    private GLSurfaceView gLView;
    private GLRTest renderer;

    public static final String KEY_MODE="KEY_MODE";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        gvrLayout=new GvrLayout(this);

        final Bundle bundle=getIntent().getExtras();
        final int MODE=bundle.getInt(KEY_MODE,0);

        gLView = new GLSurfaceView(this);
        gLView.setEGLContextClientVersion(2);
        if(MODE==0){
            renderer=new GLRTest(this,gvrLayout.getGvrApi(),true,
                    true,true,false,false);
        }else{
            renderer=new GLRTest(this,gvrLayout.getGvrApi(),false,
                    true,false,true,false);
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
