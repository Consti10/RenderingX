package constantin.renderingx.core.vrsettings;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;

import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;

import constantin.renderingx.core.R;
import constantin.renderingx.core.deviceinfo.Extensions;

public class FSettingsVR extends PreferenceFragmentCompat implements SharedPreferences.OnSharedPreferenceChangeListener{
    private static final String TAG=FSettingsVR.class.getSimpleName();

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        //Log.d("LOL","name is"+getPreferenceManager().getSharedPreferencesName());
        PreferenceManager prefMgr = getPreferenceManager();
        prefMgr.setSharedPreferencesName(ASettingsVR.PREFERENCES_NAME);
        setPreferencesFromResource(R.xml.pref_vr_rendering,rootKey);
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
        if(key.contentEquals(getString(R.string.VR_RENDERING_MODE))){
            validateUserInputRenderingModes(preferences,key);
        }
    }

    @SuppressLint("ApplySharedPref")
    private void validateUserInputRenderingModes(final SharedPreferences pref_vr_rendering, final String rendereringModesKey){
        final int wantedRenderingMode=pref_vr_rendering.getInt(rendereringModesKey,0);
        Log.d(TAG,"Validating rendering mode "+wantedRenderingMode);
        final Context context=requireActivity();
        if(wantedRenderingMode==2 ){
            // Unlimited
            if(!isUnlimitedSupported(context)){
                String warn="Cannot enable unlimited.";
                warn+="\n-EGL_KHR_mutable_render_bufferAvailable: "+Extensions.available(context,Extensions.EGL_KHR_mutable_render_buffer);
                warn+="\n-EGL_ANDROID_front_buffer_auto_refreshAvailable: "+Extensions.available(context,Extensions.EGL_ANDROID_front_buffer_auto_refresh );
                Log.d(TAG,warn);
                pref_vr_rendering.edit().putInt(rendereringModesKey,0).commit();
            }
        }
        if(wantedRenderingMode==3){
            if(!isSuperSyncSupported(context)){
                String warn="Cannot enable SuperSync.";
                warn+="\n-EGL_KHR_mutable_render_bufferAvailable: "+Extensions.available(context,Extensions.EGL_KHR_mutable_render_buffer);
                warn+="\n-EGL_ANDROID_front_buffer_auto_refreshAvailable: "+Extensions.available(context,Extensions.EGL_ANDROID_front_buffer_auto_refresh );
                warn+="\n-EGL_KHR_reusable_syncAvailable: "+Extensions.available(context,Extensions.EGL_KHR_reusable_sync );
                Log.d(TAG,warn);
                pref_vr_rendering.edit().putInt(rendereringModesKey,0).commit();
            }
        }
    }

    public static boolean isUnlimitedSupported(final Context context){
        return Extensions.available(context, Extensions.EGL_KHR_mutable_render_buffer)
                && Extensions.available(context,Extensions.EGL_ANDROID_front_buffer_auto_refresh);
    }

    public static boolean isSuperSyncSupported(final Context context){
        return (Extensions.available(context, Extensions.EGL_KHR_mutable_render_buffer)
                && Extensions.available(context,Extensions.EGL_ANDROID_front_buffer_auto_refresh)
                && Extensions.available(context,Extensions.EGL_KHR_reusable_sync));
    }

}
