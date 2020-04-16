package constantin.renderingx.example.StereoVR.DistortionTest;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.KeyEvent;

import androidx.appcompat.app.AppCompatActivity;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;

import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.views.MyGLSurfaceView;
import constantin.renderingx.core.views.MyVRLayout;

//Uses the LiveVideo10ms VideoCore lib which is intended for live streaming, not file playback.
//I recommend using android MediaPlayer if only playback from file is needed

//See native code (renderer) for documentation
public class AExampleDistortion extends AppCompatActivity {
    private static final String TAG="AExampleDistortion";
    private MyGLSurfaceView gLView;
    //Use one of both, either GvrLayout or MyVRLayout
    private static final boolean USE_GVR_LAYOUT=false;
    private GvrLayout gvrLayout;
    private MyVRLayout myVRLayout;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final GvrApi gvrApi;
        if(USE_GVR_LAYOUT){
            gvrLayout=new GvrLayout(this);
            gvrApi =gvrLayout.getGvrApi();
        }else{
            myVRLayout=new MyVRLayout(this);
            gvrApi=myVRLayout.getGvrApi();
        }
        gLView = new MyGLSurfaceView(this);
        gLView.setEGLContextClientVersion(2);
        final RendererDistortion renderer=new RendererDistortion(this,gvrApi);
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
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(gvrLayout!=null)gvrLayout.onResume();
    }

    @Override
    protected void onPause(){
        super.onPause();
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
        //Here we effectively disable these 2 buttons
        if(event.getKeyCode()==KeyEvent.KEYCODE_VOLUME_DOWN || event.getKeyCode()==KeyEvent.KEYCODE_VOLUME_UP){
            return true;
        }
        //if(event.getKeyCode()==KeyEvent.KEYCODE_BACK){
        //    return true;
        //}
        return super.dispatchKeyEvent(event);
    }

}