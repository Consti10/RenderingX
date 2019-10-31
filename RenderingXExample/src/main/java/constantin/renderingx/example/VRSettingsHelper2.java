package constantin.renderingx.example;

import android.content.Context;
import android.content.SharedPreferences;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.GvrViewerParams;

public class VRSettingsHelper2 {
    public static final String NAME="pref_undistortion";

    public static final String HEADSET="TEST";

    public static void calculateUndistortionIfNeeded(final Context context){
        final SharedPreferences pref_undistortion=context.getSharedPreferences(NAME,Context.MODE_PRIVATE);
        final String params=pref_undistortion.getString(HEADSET,"");
        if(params.equals("")){
            //calculate undistortion for this headset
        }
    }

    public static String getCurrentSelectedHeadsetName(final Context c){
        GvrView view=new GvrView(c);
        final GvrViewerParams params=view.getGvrViewerParams();
        return params.getModel();
    }
}
