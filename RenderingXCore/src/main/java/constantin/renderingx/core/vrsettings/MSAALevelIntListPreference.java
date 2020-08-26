package constantin.renderingx.core.vrsettings;

import android.content.Context;
import android.util.AttributeSet;

import com.mapzen.prefsplusx.IntListPreference;

import java.util.ArrayList;

import constantin.renderingx.core.gles_info.OpenGLESValues;

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
        CharSequence[] msaaEntries =new CharSequence[allMSAALevels.size()];
        CharSequence[] msaaEntryValues =new CharSequence[allMSAALevels.size()];
        for(int i=0;i<allMSAALevels.size();i++){
            msaaEntries[allMSAALevels.size()-1-i]=""+allMSAALevels.get(i)+"xMSAA";
            msaaEntryValues[allMSAALevels.size()-1-i]=""+allMSAALevels.get(i);
        }
        setEntries(msaaEntries);
        setEntryValues(msaaEntryValues);
    }

}
