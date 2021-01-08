package constantin.renderingx.core.views;

import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.WindowManager;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.MVrHeadsetParams;
import constantin.renderingx.core.vrsettings.ASettingsVR;


// Specific implementation of AppCompatActivity that handles full screen and key events
public class VrActivity  extends AppCompatActivity {
    private static final String TAG="VrActivity";
    public static final int REQUEST_CODE_NOTIFY_IF_VR_SETTINGS_CHANGED =99;

    private String currentlySelectedGvrViewerModel=null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if(ASettingsVR.getVR_INVERT_LEFT_AND_RIGHT(this)){
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
        }else{
            // Landscape is the default
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        // To find out if the model has changed
        currentlySelectedGvrViewerModel= MVrHeadsetParams.getCurrentGvrViewerModel(this);

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
        if(!MVrHeadsetParams.getCurrentGvrViewerModel(this).contentEquals(currentlySelectedGvrViewerModel)){
            createAlertDialogConfigChanged();
        }
    }

    @Override
    protected void onPause(){
        super.onPause();
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

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        //Log.d(TAG,"Got onActivityResult "+requestCode+" "+resultCode);
        // We get this message when the settings hold by the vr layout have changed
        if (requestCode == REQUEST_CODE_NOTIFY_IF_VR_SETTINGS_CHANGED) {
            if(resultCode==ASettingsVR.RESULT_CODE_SETTINGS_CHANGED_RESTART_REQUIRED){
               createAlertDialogConfigChanged();
            }
        }
    }

    private void createAlertDialogConfigChanged(){
        Log.d(TAG,"Got result that we need to restart the activity");
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setCancelable(false);
        builder.setTitle("Vr config changed").setMessage("Restart required");
        builder.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                recreate();
            }
        });
        builder.setNegativeButton("No",null);
        AlertDialog dialog = builder.create();
        dialog.show();
    }
}
