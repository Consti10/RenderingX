package constantin.renderingx.core;

// Surface Texture Helper

import android.graphics.SurfaceTexture;

public class STHelper {
    // returns true if SurfaceTexture was actually updated
    // Tested on following APIs:

    public static boolean updateAndCheck(final SurfaceTexture surfaceTexture){
        final long oldTimestamp=surfaceTexture.getTimestamp();
        surfaceTexture.updateTexImage();
        if(surfaceTexture.getTimestamp()!=oldTimestamp){
            return true;
        }
        return false;
    }

}
