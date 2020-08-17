package constantin.renderingx.core;

import android.os.Bundle;
import android.view.KeyEvent;
import android.view.WindowManager;

import androidx.appcompat.app.AppCompatActivity;

import constantin.renderingx.core.vrsettings.ASettingsVR;

// Specific implementation of AppCompatActivity that handles full screen and key events
public class VrActivity  extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Make dialoque for permission
        /*if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            final boolean donNotAsk=PreferenceManager.getDefaultSharedPreferences(this).
                    getBoolean(getString(R.string.RX_VR_PERMISSION_ACTION_MANAGE_WRITE_SETTINGS_DO_NOT_ASK),false);
            if(!Settings.System.canWrite(this) && !donNotAsk){
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setCancelable(false);
                builder.setMessage("Permission to automatically adjust your screen brightness in VR mode");
                builder.setPositiveButton("Okay", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        Intent intent = new Intent(Settings.ACTION_MANAGE_WRITE_SETTINGS);
                        intent.setData(Uri.parse("package:" + getApplication().getPackageName()));
                        startActivity(intent);
                    }
                });
                builder.setNegativeButton("Never", new DialogInterface.OnClickListener() {
                    @SuppressLint("ApplySharedPref")
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        PreferenceManager.getDefaultSharedPreferences(VrActivity.this).edit().
                                putBoolean(getString(R.string.RX_VR_PERMISSION_ACTION_MANAGE_WRITE_SETTINGS_DO_NOT_ASK),true).commit();
                    }
                });
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        }*/
    }

    @Override
    protected void onResume(){
        super.onResume();
        WindowManager.LayoutParams layout = getWindow().getAttributes();
        layout.screenBrightness = ASettingsVR.getScreenBrightnessInRangeZeroTo1(this);
        getWindow().setAttributes(layout);
        //if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.M || Settings.System.canWrite(this)) {
        //}
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
