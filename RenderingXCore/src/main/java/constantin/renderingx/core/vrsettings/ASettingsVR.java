package constantin.renderingx.core.vrsettings;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceScreen;
import androidx.preference.PreferenceManager;
import android.content.SharedPreferences;
import constantin.renderingx.core.R;

// Simple Settings Activity that handles VR settings
public class ASettingsVR extends AppCompatActivity implements PreferenceFragmentCompat.OnPreferenceStartScreenCallback{
    private static final String TAG=ASettingsVR.class.getSimpleName();
    // This one is written to  getActivity().setResult() if the vr settings have changed
    // such that the base VR application needs to be restarted
    public static final int RESULT_CODE_SETTINGS_CHANGED_RESTART_REQUIRED =321;
    public static final String PREFERENCES_NAME="pref_vr_rendering";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Display the fragment as the main content.
        FSettingsVR fSettingsVR = new FSettingsVR();
        getSupportFragmentManager().beginTransaction()
                .replace(android.R.id.content, fSettingsVR)
                .commit();
    }

    @Override
    public boolean onPreferenceStartScreen(PreferenceFragmentCompat caller, PreferenceScreen preferenceScreen) {
        //Log.d(TAG,"name is"+caller.getPreferenceManager().getSharedPreferencesName());
        final FSettingsVR newSettingsFragment=new FSettingsVR();
        Bundle args = new Bundle();
        args.putString(PreferenceFragmentCompat.ARG_PREFERENCE_ROOT,preferenceScreen.getKey());
        newSettingsFragment.setArguments(args);

        getSupportFragmentManager().beginTransaction()
                .replace(android.R.id.content,newSettingsFragment)
                .addToBackStack(null)
                .commit();

        Log.d(TAG,"onPreferenceStartScreen "+preferenceScreen.getKey());
        return false;
    }
    public static SharedPreferences getVRSharedPreferences(final Context context){
        return context.getSharedPreferences(PREFERENCES_NAME,MODE_PRIVATE);
    }
    public static float getScreenBrightnessInRangeZeroTo1(final Context context){
        // normalize the value
        return getVRSharedPreferences(context).getFloat(context.getString(R.string.VR_SCREEN_BRIGHTNESS_PERCENTAGE),80)/100.0f;
    }
    public static int getVR_RENDERING_MODE(final Context context){
        return getVRSharedPreferences(context).getInt(context.getString(R.string.VR_RENDERING_MODE),0);
    }

    //@SuppressLint("ApplySharedPref")
    public static void initializePreferences(final Context context,final boolean readAgain){
        PreferenceManager.setDefaultValues(context,PREFERENCES_NAME,MODE_PRIVATE, R.xml.pref_vr_rendering,readAgain);
    }
}
