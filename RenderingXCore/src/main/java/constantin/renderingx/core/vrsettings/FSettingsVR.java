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
        //validateUserInputRenderingModes(preferences,key);
    }

    /*private void validateUserInputRenderingModes(final SharedPreferences pref_default,final String key){

        if(key.contentEquals(getString(R.string.DisableVSYNC))&& pref_default.getBoolean(key,false)){

            if(Extensions.available(getActivity(),Extensions.EGL_KHR_mutable_render_buffer)){
                SwitchPreference sp=(SwitchPreference)findPreference(getString(R.string.Disable60FPSLock));
                sp.setChecked(false);
            }else{
                String warn="This smartphone does not support disabling VSYNC\n";
                warn+="EGL_KHR_mutable_render_bufferAvailable: false\n";
                makeDialog(getActivity(),warn);
                SwitchPreference sp=(SwitchPreference)findPreference(getString(R.string.DisableVSYNC));
                sp.setChecked(false);
            }
        }else if(key.contentEquals(getString(R.string.SuperSync)) && pref_default.getBoolean(key,false)){
            //The user enabled the "SuperSync" option
            if(!(Extensions.available(getActivity(),Extensions.EGL_KHR_mutable_render_buffer) &&
                    Extensions.available(getActivity(),Extensions.EGL_ANDROID_front_buffer_auto_refresh ))){
                String warn="This smartphone does not support enabling SuperSync.";
                warn+="\n-EGL_KHR_mutable_render_bufferAvailable: "+Extensions.available(getActivity(),Extensions.EGL_KHR_mutable_render_buffer);
                warn+="\n-EGL_ANDROID_front_buffer_auto_refreshAvailable: "+Extensions.available(getActivity(),Extensions.EGL_ANDROID_front_buffer_auto_refresh );
                warn+="\n-EGL_KHR_reusable_syncAvailable: "+Extensions.available(getActivity(),Extensions.EGL_KHR_reusable_sync );
                makeDialog(getActivity(),warn);
                SwitchPreference sp=(SwitchPreference)findPreference(getString(R.string.SuperSync));
                sp.setChecked(false);
            }
        }else if(key.contentEquals(getString(R.string.Disable60FPSLock))&& pref_default.getBoolean(key,false)){
            //The user enabled the "disable60fpslock" option. Cannot be used simultaneous with "DisableVSYNC". / SuperSync
            SwitchPreference p1=(SwitchPreference)findPreference(getString(R.string.DisableVSYNC));
            p1.setChecked(false);
            SwitchPreference p2=(SwitchPreference)findPreference(getString(R.string.SuperSync));
            p2.setChecked(false);
        }
    }*/

}
