package constantin.renderingx.example.renderer3;

import androidx.appcompat.app.AppCompatActivity;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;

import com.google.vr.ndk.base.GvrLayout;

import constantin.renderingX.PerformanceHelper;

public class TestActivity3 extends AppCompatActivity {
    private GvrLayout gvrLayout;
    private GLSurfaceView gLView;
    private GLRTest renderer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        gvrLayout=new GvrLayout(this);
        gLView = new GLSurfaceView(this);
        gLView.setEGLContextClientVersion(2);
        renderer=new GLRTest(this,gvrLayout.getGvrApi());
        gLView.setRenderer(renderer);
        gLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        gvrLayout.setPresentationView(gLView);
        setContentView(gvrLayout);
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
}
