package constantin.renderingx.core.vrsettings;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;
import androidx.preference.SwitchPreference;


import com.mapzen.prefsplusx.IntListPreference;

import java.util.ArrayList;

import constantin.renderingx.core.R;
import constantin.renderingx.core.gles_info.OpenGLESValues;


public class ASettingsVR extends AppCompatActivity {
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

    @Override
    protected void onPause(){
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    public static float getScreenBrightnessInRangeZeroTo1(final Context context){
        // normalize the value
        return context.getSharedPreferences("pref_vr_rendering",MODE_PRIVATE).getFloat(context.getString(R.string.VR_SCREEN_BRIGHTNESS_PERCENTAGE),80)/100.0f;
    }

    public static class FSettingsVR extends PreferenceFragmentCompat implements SharedPreferences.OnSharedPreferenceChangeListener{


        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
            PreferenceManager prefMgr = getPreferenceManager();
            prefMgr.setSharedPreferencesName("pref_vr_rendering");
            prefMgr.setSharedPreferencesMode(MODE_PRIVATE);
            addPreferencesFromResource(R.xml.pref_vr_rendering);
        }

        @Override
        public void onActivityCreated(Bundle savedInstanceState){
            super.onActivityCreated(savedInstanceState);
            setupHeadTrackingCategory(getPreferenceScreen().getSharedPreferences());
            setupMSAALevelsPreference(getActivity());
        }

        @Override
        public void onResume(){
            super.onResume();
            getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onPause(){
            super.onPause();
            getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onSharedPreferenceChanged(SharedPreferences preferences, String key) {
            if(key.contentEquals(getString(R.string.GroundHeadTrackingMode)) || key.contentEquals(getString(R.string.AirHeadTrackingMode))){
                setupHeadTrackingCategory(preferences);
            }
            getActivity().setResult(ASettingsVR.RESULT_CODE_SETTINGS_CHANGED_RESTART_REQUIRED,null);
        }

        private void setupHeadTrackingCategory(SharedPreferences sharedPreferences){
            //head tracking. we cannot set dependencies depending on list preferences in xml, so we have to do it in java
            boolean enabledGHT=sharedPreferences.getInt(getString(R.string.GroundHeadTrackingMode),0)!=0;
            SwitchPreference sp1=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingX));
            SwitchPreference sp2=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingY));
            SwitchPreference sp3=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingZ));
            sp1.setEnabled(enabledGHT);
            sp2.setEnabled(enabledGHT);
            sp3.setEnabled(enabledGHT);
            /*boolean enabledAHT=sharedPreferences.getInt(getString(R.string.AirHeadTrackingMode),0)!=0;
            SwitchPreference sp1x=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingYaw));
            SwitchPreference sp2x=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingRoll));
            SwitchPreference sp3x=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingPitch));
            sp1x.setEnabled(enabledAHT);
            sp2x.setEnabled(enabledAHT);
            sp3x.setEnabled(enabledAHT);
            ListPreference lp1=(ListPreference)findPreference(getString(R.string.AHTRefreshRateMs));
            lp1.setEnabled(enabledAHT);*/
        }

        private void setupMSAALevelsPreference(final Context c){
            ArrayList<Integer> allMSAALevels= OpenGLESValues.availableMSAALevels(c);
            CharSequence[] msaaEntries =new CharSequence[allMSAALevels.size()];
            CharSequence[] msaaEntryValues =new CharSequence[allMSAALevels.size()];
            for(int i=0;i<allMSAALevels.size();i++){
                msaaEntries[allMSAALevels.size()-1-i]=""+allMSAALevels.get(i)+"xMSAA";
                msaaEntryValues[allMSAALevels.size()-1-i]=""+allMSAALevels.get(i);
            }
            IntListPreference msaaPreference = (IntListPreference) findPreference(this.getString(R.string.MultiSampleAntiAliasing));
            msaaPreference.setEntries(msaaEntries);
            msaaPreference.setEntryValues(msaaEntryValues);
        }

    }
}
