package constantin.renderingx.core;

import android.view.KeyEvent;

import androidx.appcompat.app.AppCompatActivity;

// Specific implementation of AppCompatActivity that handles full screen and key events
public class VrActivity  extends AppCompatActivity {

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
