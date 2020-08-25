package constantin.renderingx.core.vrsettings;

import android.content.SharedPreferences;
import android.os.Bundle;

import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;

import constantin.renderingx.core.R;

import static android.content.Context.MODE_PRIVATE;

public class FSettingsVR1 extends PreferenceFragmentCompat implements SharedPreferences.OnSharedPreferenceChangeListener{

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        PreferenceManager prefMgr = getPreferenceManager();
        prefMgr.setSharedPreferencesName("pref_vr_rendering");
        prefMgr.setSharedPreferencesMode(MODE_PRIVATE);
        addPreferencesFromResource(R.xml.pref_vr_rendering1);
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
        requireActivity().setResult(ASettingsVR.RESULT_CODE_SETTINGS_CHANGED_RESTART_REQUIRED,null);
    }
}