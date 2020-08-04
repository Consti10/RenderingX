package constantin.renderingx.example.supersync;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

import constantin.renderingx.core.views.ViewSuperSync;
import constantin.video.core.video_player.VideoPlayer;
import constantin.video.core.video_player.VideoSettings;

public class AExampleSuperSync extends AppCompatActivity {
    private ViewSuperSync mViewSuperSync;
    private GLRExampleSuperSync mGLRStereoSuperSync;
    private VideoPlayer videoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        VideoSettings.setVS_SOURCE(this, VideoSettings.VS_SOURCE.ASSETS);
        VideoSettings.setVS_ASSETS_FILENAME_TEST_ONLY(this,"360DegreeVideos/testRoom1_1920Mono.mp4");
        VideoSettings.setVS_FILE_ONLY_LIMIT_FPS(this,40);
        videoPlayer=new VideoPlayer(this);

        mViewSuperSync=new ViewSuperSync(this);
        mGLRStereoSuperSync = new GLRExampleSuperSync(this,videoPlayer.configure2(),mViewSuperSync.getGvrApi());
        mViewSuperSync.setRenderer(mGLRStereoSuperSync);
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
