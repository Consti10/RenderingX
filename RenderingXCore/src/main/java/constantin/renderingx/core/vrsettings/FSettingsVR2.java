package constantin.renderingx.core.vrsettings;

import android.content.SharedPreferences;
import android.os.Bundle;

import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;
import androidx.preference.SwitchPreference;

import constantin.renderingx.core.R;

import static android.content.Context.MODE_PRIVATE;

public class FSettingsVR2 extends PreferenceFragmentCompat implements SharedPreferences.OnSharedPreferenceChangeListener{

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        PreferenceManager prefMgr = getPreferenceManager();
        prefMgr.setSharedPreferencesName("pref_vr_rendering");
        prefMgr.setSharedPreferencesMode(MODE_PRIVATE);
        addPreferencesFromResource(R.xml.pref_vr_rendering2);
        setupHeadTrackingCategory(getPreferenceScreen().getSharedPreferences());
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
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        if(key.contentEquals(getString(R.string.GroundHeadTrackingMode)) || key.contentEquals(getString(R.string.AirHeadTrackingMode))){
            setupHeadTrackingCategory(sharedPreferences);
        }
        requireActivity().setResult(ASettingsVR.RESULT_CODE_SETTINGS_CHANGED_RESTART_REQUIRED,null);
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
}