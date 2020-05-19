package constantin.renderingx.core;

// Surface Texture Helper

import android.graphics.SurfaceTexture;

// 18.05.2020
// It looks like we can safely use the timestamp to measure delay
// Also, if the timestamp changes we know that the update of SurfaceTexture was successfully
public class STHelper {
    // returns true if SurfaceTexture was actually updated
    // Tested here:
    // https://console.firebase.google.com/u/1/project/fpv-vr-for-wifibroadcast/testlab/histories/bh.88d8060ed73917a4/matrices/9009976508266637057
    public static boolean updateAndCheck(final SurfaceTexture surfaceTexture){
        final long oldTimestamp=surfaceTexture.getTimestamp();
        surfaceTexture.updateTexImage();
        if(surfaceTexture.getTimestamp()!=oldTimestamp){
            return true;
        }
        return false;
    }

}
