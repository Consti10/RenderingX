package constantin.renderingx.example.supersync;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

import com.google.vr.sdk.base.GvrActivity;

import constantin.renderingx.core.VrActivity;
import constantin.renderingx.core.views.VrView;
import constantin.video.core.video_player.VideoPlayer;
import constantin.video.core.video_player.VideoSettings;

public class AExampleSuperSync extends VrActivity {
    private VrView mViewSuperSync;
    private GLRExampleSuperSync mGLRStereoSuperSync;
    private VideoPlayer videoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        VideoSettings.setVS_SOURCE(this, VideoSettings.VS_SOURCE.ASSETS);
        VideoSettings.setVS_ASSETS_FILENAME_TEST_ONLY(this,"360DegreeVideos/testRoom1_1920Mono.mp4");
        VideoSettings.setVS_FILE_ONLY_LIMIT_FPS(this,40);
        videoPlayer=new VideoPlayer(this);

        mViewSuperSync=new VrView(this);
        mGLRStereoSuperSync = new GLRExampleSuperSync(this,mViewSuperSync.getGvrApi());
        mViewSuperSync.enableSuperSync();
        mViewSuperSync.getPresentationView().setRenderer(mGLRStereoSuperSync,videoPlayer.configure2());
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
