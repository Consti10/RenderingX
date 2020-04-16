package constantin.renderingx.core.video;

import android.graphics.SurfaceTexture;
import android.view.Surface;

public interface ISurfaceAvailable {
    // Always called on the UI thread and while activity is in state == resumed
    void start(final SurfaceTexture surfaceTexture, final Surface surface);
    void stop();
}
