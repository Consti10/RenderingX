package constantin.renderingx.core.vrsettings;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;
import androidx.preference.PreferenceScreen;

import constantin.renderingx.core.R;
import static android.content.Context.MODE_PRIVATE;

public class FSettingsVR extends PreferenceFragmentCompat implements SharedPreferences.OnSharedPreferenceChangeListener{
    // If created by the custom constructor taking a PreferenceScreen the fragment will be inflated using this preferenceScreen instead
    // Workaround for nested preference screens inside .xml file
    /*private final PreferenceScreen preferenceScreen;
    FSettingsVR(PreferenceScreen preferenceScreen){
        this.preferenceScreen=preferenceScreen;
    }
    FSettingsVR(){
        this.preferenceScreen=null;
    }*/
    
    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        //Log.d("LOL","name is"+getPreferenceManager().getSharedPreferencesName());
        PreferenceManager prefMgr = getPreferenceManager();
        prefMgr.setSharedPreferencesName("pref_vr_rendering");
        prefMgr.setSharedPreferencesMode(MODE_PRIVATE);
        setPreferencesFromResource(R.xml.pref_vr_rendering,rootKey);
        //if(preferenceScreen==null){
            //addPreferencesFromResource(R.xml.pref_vr_rendering);
        //}else{
        //    setPreferenceScreen(preferenceScreen);
        //}
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState){
        super.onActivityCreated(savedInstanceState);
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
        requireActivity().setResult(ASettingsVR.RESULT_CODE_SETTINGS_CHANGED_RESTART_REQUIRED,null);
    }

}
