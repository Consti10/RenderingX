package constantin.renderingx.core;

import android.app.Activity;
import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.Toast;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrUiLayout;

//The GvrLayout does not allow users to create a 'normal' context when selected headset==Daydream
//Simple workaround, you can use this as a drop-in replacement of GvrLayout when using your own presentationView anyway
//TODO do we need DisplaySynchronizer ? Its implementation seems to be broken

public class MyVRLayout extends FrameLayout {

    private GvrApi gvrApi;
    //private DisplaySynchronizer displaySynchronizer;

    public MyVRLayout(final Context context) {
        super(context);
        init();
    }

    public MyVRLayout(final Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    private void init(){
        LayoutInflater.from(getContext()).inflate(R.layout.my_vr_layout, this, true);
        //displaySynchronizer=new DisplaySynchronizer(getContext(),getDisplay());
        gvrApi=new GvrApi(getContext(),null);
        final Activity activity=(Activity)getContext();
        findViewById(R.id.vr_overlay_settings_button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GvrUiLayout.launchOrInstallGvrApp(activity);
                activity.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(activity,"VR Headset changes require an activity restart",Toast.LENGTH_LONG).show();
                    }
                });
            }
        });
        findViewById(R.id.vr_overlay_back_button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                activity.finish();
            }
        });
        //In VR, always enable sustained performance if possible
        PerformanceHelper.enableSustainedPerformanceIfPossible(activity);
        //Dim the screen to n percent TODO
        //Enable Immersive mode
        PerformanceHelper.enableImmersiveSticky(activity);
    }

    public GvrApi getGvrApi(){
        return gvrApi;
    }

    public void setPresentationView(View presentationView){
        addView(presentationView,0);
    }

    public void onResumeX(){
        gvrApi.resumeTracking();
    }

    public void onPauseX(){
        gvrApi.pauseTracking();
    }

    public void shutdown(){
        gvrApi.shutdown();
    }

}
