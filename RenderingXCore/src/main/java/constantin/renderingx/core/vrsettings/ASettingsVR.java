package constantin.renderingx.core.vrsettings;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceScreen;

import constantin.renderingx.core.R;

public class ASettingsVR extends AppCompatActivity implements PreferenceFragmentCompat.OnPreferenceStartScreenCallback{
    private static final String TAG=ASettingsVR.class.getSimpleName();
    // This one is written to  getActivity().setResult() if the vr settings have changed
    // such that the base VR application needs to be restarted
    public static final int RESULT_CODE_SETTINGS_CHANGED_RESTART_REQUIRED =321;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Display the fragment as the main content.
        FSettingsVR fSettingsVR = new FSettingsVR();
        getSupportFragmentManager().beginTransaction()
                .replace(android.R.id.content, fSettingsVR)
                .commit();
    }

    public static float getScreenBrightnessInRangeZeroTo1(final Context context){
        // normalize the value
        return context.getSharedPreferences("pref_vr_rendering",MODE_PRIVATE).getFloat(context.getString(R.string.VR_SCREEN_BRIGHTNESS_PERCENTAGE),80)/100.0f;
    }

    @Override
    public boolean onPreferenceStartScreen(PreferenceFragmentCompat caller, PreferenceScreen preferenceScreen) {
        Log.d(TAG,"name is"+caller.getPreferenceManager().getSharedPreferencesName());
        final FSettingsVR newSettingsFragment=new FSettingsVR(preferenceScreen);
        //Bundle args = new Bundle();
        //args.putString(PreferenceFragmentCompat.ARG_PREFERENCE_ROOT,preferenceScreen.getKey());
        //newSettingsFragment.setArguments(args);

        getSupportFragmentManager().beginTransaction()
                .replace(android.R.id.content,newSettingsFragment)
                .addToBackStack(null)
                .commit();

        //f.setPreferenceScreen(pref);
        Log.d(TAG,"onPreferenceStartScreen "+preferenceScreen.getKey());
        return false;
    }
}
