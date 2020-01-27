package constantin.renderingx.core;

import android.app.Activity;
import android.os.Build;
import android.os.PowerManager;
import android.util.Log;
import android.view.View;

import com.google.vr.sdk.base.AndroidCompat;

import static android.content.Context.POWER_SERVICE;

public final class PerformanceHelper {
    private static final String TAG="PerformanceHelper";

    public static void enableSustainedPerformanceIfPossible(Activity c){
        if (Build.VERSION.SDK_INT >= 24) {
            final PowerManager powerManager = (PowerManager)c.getSystemService(POWER_SERVICE);
            if(powerManager!=null){
                if (powerManager.isSustainedPerformanceModeSupported()) {
                    //slightly lower, but sustainable clock speeds
                    //I also enable this mode (if the device supports it) when not doing front buffer rendering,
                    //because when the user decides to render at 120fps or more (disable vsync/60fpsCap)
                    //the App benefits from sustained performance, too
                    AndroidCompat.setSustainedPerformanceMode(c,true);
                    Log.d(TAG,"Sustained performance set true");
                }else{
                    Log.d(TAG,"Sustained performance not available");
                }
            }
        }
    }

    public static void disableSustainedPerformanceIfEnabled(Activity c){
        if (Build.VERSION.SDK_INT >= 24) {
            final PowerManager powerManager = (PowerManager)c.getSystemService(POWER_SERVICE);
            if(powerManager!=null){
                if (powerManager.isSustainedPerformanceModeSupported()) {
                    AndroidCompat.setSustainedPerformanceMode(c, false);
                }
            }
        }
    }

    public static void enableAndroidVRModeIfPossible(Activity c){
        if(Build.VERSION.SDK_INT>=24){
            boolean succ= AndroidCompat.setVrModeEnabled(c,true);
            if(!succ){
                Log.d(TAG,"Cannot enable vr mode");
            }
        }
    }

    public static void disableAndroidVRModeIfEnabled(Activity c){
        if (Build.VERSION.SDK_INT >= 24) {
            AndroidCompat.setVrModeEnabled(c,false);
        }
    }


    public static void setImmersiveSticky(final Activity c){
        final View decorView = c.getWindow().getDecorView();
        decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        decorView.setOnSystemUiVisibilityChangeListener(
                new View.OnSystemUiVisibilityChangeListener() {
                    @Override
                    public void onSystemUiVisibilityChange(int visibility) {
                        if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                            c.getWindow().getDecorView().setSystemUiVisibility(
                                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                            | View.SYSTEM_UI_FLAG_FULLSCREEN
                                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
                        }
                        //System.out.println("visibility changed again");
                    }
                });
    }


}
