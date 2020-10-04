package constantin.renderingx.core.vrsettings;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;

import com.mapzen.prefsplusx.IntListPreference;

import java.util.ArrayList;

import constantin.renderingx.core.deviceinfo.OpenGLESValues;

// Allow the user to only select MSAA levels that are supported by this device
public class MSAALevelIntListPreference extends IntListPreference {

    public MSAALevelIntListPreference(Context context) {
        super(context);
        setEntriesAndEntryValues(context);
    }

    public MSAALevelIntListPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEntriesAndEntryValues(context);
    }

    private void setEntriesAndEntryValues(Context context){
        ArrayList<Integer> allMSAALevels= OpenGLESValues.availableMSAALevels(context);
        Log.d(TAG,"Available msaa levels "+allMSAALevels.toString());
        CharSequence[] msaaEntries =new CharSequence[allMSAALevels.size()];
        CharSequence[] msaaEntryValues =new CharSequence[allMSAALevels.size()];
        for(int i=0;i<allMSAALevels.size();i++){
            final int msaaLevel=allMSAALevels.get(i);
            msaaEntries[i]=""+msaaLevel+"xMSAA";
            msaaEntryValues[i]=""+msaaLevel;
        }
        setEntries(msaaEntries);
        setEntryValues(msaaEntryValues);
    }

}
