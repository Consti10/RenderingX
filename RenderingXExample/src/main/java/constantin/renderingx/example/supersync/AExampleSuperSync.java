package constantin.renderingx.example.supersync;

import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.view.Surface;

import androidx.appcompat.app.AppCompatActivity;

import constantin.renderingx.core.views.VrViewSuperSync;
import constantin.renderingx.core.xglview.SurfaceTextureHolder;
import constantin.video.core.video_player.VideoPlayer;
import constantin.video.core.video_player.VideoSettings;

public class AExampleSuperSync extends AppCompatActivity {
    private VrViewSuperSync mViewSuperSync;
    private GLRExampleSuperSync mGLRStereoSuperSync;
    private VideoPlayer videoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        VideoSettings.setVS_SOURCE(this, VideoSettings.VS_SOURCE.ASSETS);
        VideoSettings.setVS_ASSETS_FILENAME_TEST_ONLY(this,"360DegreeVideos/testRoom1_1920Mono.mp4");
        VideoSettings.setVS_FILE_ONLY_LIMIT_FPS(this,40);
        videoPlayer=new VideoPlayer(this);

        final SurfaceTextureHolder.ISurfaceTextureAvailable iSurfaceTextureAvailable=new SurfaceTextureHolder.ISurfaceTextureAvailable() {
            @Override
            public void surfaceTextureCreated(SurfaceTexture surfaceTexture, Surface surface) {
                videoPlayer.addAndStartDecoderReceiver(surface);
            }
            @Override
            public void surfaceTextureDestroyed() {
                videoPlayer.stopAndRemoveReceiverDecoder();
            }
        };
        mViewSuperSync=new VrViewSuperSync(this);
        mGLRStereoSuperSync = new GLRExampleSuperSync(this,mViewSuperSync.getGvrApi());
        mViewSuperSync.setRenderer(mGLRStereoSuperSync,iSurfaceTextureAvailable);
        setContentView(mViewSuperSync);
    }

    @Override
    protected void onResume() {
        super.onResume();
        System.out.println("YYY onResumeX()");
    }

    @Override
    protected void onPause(){
        super.onPause();
        System.out.println("YYY onPauseX()");
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        mViewSuperSync=null;
        mGLRStereoSuperSync=null;
    }
}
