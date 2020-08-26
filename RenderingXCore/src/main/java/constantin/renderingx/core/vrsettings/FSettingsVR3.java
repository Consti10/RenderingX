package constantin.renderingx.core.vrsettings;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;

import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;

import com.mapzen.prefsplusx.IntListPreference;

import java.util.ArrayList;

import constantin.renderingx.core.R;
import constantin.renderingx.core.gles_info.OpenGLESValues;

import static android.content.Context.MODE_PRIVATE;

public class FSettingsVR3 extends PreferenceFragmentCompat implements SharedPreferences.OnSharedPreferenceChangeListener{

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        PreferenceManager prefMgr = getPreferenceManager();
        prefMgr.setSharedPreferencesName("pref_vr_rendering");
        prefMgr.setSharedPreferencesMode(MODE_PRIVATE);
        addPreferencesFromResource(R.xml.pref_vr_rendering3);
        setupMSAALevelsPreference(requireActivity());
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