package constantin.renderingx.core;

import android.app.Activity;
import android.view.View;


//Use to enable 'true' android immersive sticky == FULLSCREEN
//For VR
//(True) Fullscreen is such a pain on android !
 
public final class FullscreenHelper {
    //Call this in onCreate
    public static void enableImmersiveSticky(final Activity c){
        setImmersiveSticky(c);
        c.getWindow()
                .getDecorView()
                .setOnSystemUiVisibilityChangeListener(
                        new View.OnSystemUiVisibilityChangeListener() {
                            @Override
                            public void onSystemUiVisibilityChange(int visibility) {
                                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                                    setImmersiveSticky(c);
                                }
                            }
                        });
    }
    public static void setImmersiveSticky(final Activity c){
        c.getWindow()
                .getDecorView()
                .setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
    }
}
