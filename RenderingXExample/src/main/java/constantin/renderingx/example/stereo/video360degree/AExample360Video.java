package constantin.renderingx.example.stereo.video360degree;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.net.Uri;
import android.os.Bundle;
import android.view.Surface;

import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.ProgressiveMediaSource;
import com.google.android.exoplayer2.upstream.DataSource;
import com.google.android.exoplayer2.upstream.DefaultDataSourceFactory;
import com.google.android.exoplayer2.upstream.RawResourceDataSource;
import com.google.android.exoplayer2.util.Util;
import com.google.vr.ndk.base.GvrApi;

import constantin.renderingx.core.VrActivity;
import constantin.renderingx.core.views.VrView;
import constantin.renderingx.core.xglview.SurfaceTextureHolder;
import constantin.renderingx.core.xglview.XGLSurfaceView;
import constantin.renderingx.core.views.VRLayout;
import constantin.renderingx.example.R;
import constantin.video.core.gl.ISurfaceTextureAvailable;
import constantin.video.core.video_player.VideoPlayer;
import constantin.video.core.video_player.VideoSettings;

//Uses the LiveVideo10ms VideoCore lib which is intended for live streaming, not file playback.
//I recommend using android MediaPlayer if only playback from file is needed

//See native code (renderer) for documentation
public class AExample360Video extends VrActivity {
    private static final String TAG="AExampleVRRendering";
    public static final int SPHERE_MODE_GVR_EQUIRECTANGULAR=0;
    public static final int SPHERE_MODE_INSTA360_TEST=1;
    public static final int SPHERE_MODE_INSTA360_TEST2=2;
    //Default mode is 0 (test VDDC)
    public static final String KEY_SPHERE_MODE ="KEY_SPHERE_MODE";
    public static final String KEY_VIDEO_FILENAME="KEY_VIDEO_FILENAME";
    // Only one of these two is in use at the same time
    private static final boolean USE_GOOGLE_EXO_PLAYER_INSTEAD =true;
    // ExoPlayer is the better choice for file playback, but my VideoPlayer
    // Is the better choice for low latency h264 live video playback
    private VideoPlayer videoPlayer;
    private SimpleExoPlayer simpleExoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final Bundle bundle=getIntent().getExtras();
        final int SPHERE_MODE=bundle.getInt(KEY_SPHERE_MODE,0);
        final String VIDEO_FILENAME=bundle.getString(KEY_VIDEO_FILENAME);
        //start initialization
        //final VRLayout myVRLayout = new VRLayout(this);
        final VrView mVrView=new VrView(this);


        //final XGLSurfaceView glSurfaceView2=new XGLSurfaceView(this);
        //glSurfaceView2.setEGLConfigPrams(new XSurfaceParams(0,0,true));
        //glSurfaceView2.DO_SUPERSYNC_MODS=true;

        // Use one of both ! Default to the player from VideoCore
        if(USE_GOOGLE_EXO_PLAYER_INSTEAD){
           simpleExoPlayer=createAndConfigureExoPlayer(this);
        }else{
            VideoSettings.setVS_SOURCE(this, VideoSettings.VS_SOURCE.ASSETS);
            VideoSettings.setVS_ASSETS_FILENAME_TEST_ONLY(this,VIDEO_FILENAME);
            VideoSettings.setVS_FILE_ONLY_LIMIT_FPS(this,40);
            videoPlayer=new VideoPlayer(this);
        }
        final ISurfaceTextureAvailable iSurfaceTextureAvailable=new ISurfaceTextureAvailable() {
            @Override
            public void surfaceTextureCreated(SurfaceTexture surfaceTexture, Surface surface) {
                if(USE_GOOGLE_EXO_PLAYER_INSTEAD){
                    simpleExoPlayer.getVideoComponent().setVideoSurface(surface);
                }else{
                    videoPlayer.addAndStartDecoderReceiver(surface);
                }
            }
            @Override
            public void surfaceTextureDestroyed() {
                if(USE_GOOGLE_EXO_PLAYER_INSTEAD){
                    simpleExoPlayer.getVideoComponent().setVideoSurface(null);
                }else{
                    videoPlayer.stopAndRemoveReceiverDecoder();
                }
            }
        };
        Renderer360Video renderer = new Renderer360Video(this,mVrView.getGvrApi(), SPHERE_MODE);
        mVrView.setRenderer(renderer,iSurfaceTextureAvailable);
        mVrView.setmISecondaryContext(renderer);
        setContentView(mVrView);
    }

    @Override
    protected void onResume(){
        super.onResume();
        if(simpleExoPlayer!=null){
            simpleExoPlayer.setPlayWhenReady(true);
        }
    }
    @Override
    protected void onPause(){
        super.onPause();
        if(simpleExoPlayer!=null){
            simpleExoPlayer.setPlayWhenReady(false);
        }
    }


    // ExoPlayer is a better choice than the Android MediaPlayer
    private static SimpleExoPlayer createAndConfigureExoPlayer(final Context context){
        SimpleExoPlayer simpleExoPlayer = new SimpleExoPlayer.Builder(context).build();
        final Uri uri = RawResourceDataSource.buildRawResourceUri(R.raw.test_room1_1920mono);
        DataSource.Factory dataSourceFactory = new DefaultDataSourceFactory(context,
                Util.getUserAgent(context, "VideoExample"));
        MediaSource videoSource =
                new ProgressiveMediaSource.Factory(dataSourceFactory)
                        .createMediaSource(uri);
        simpleExoPlayer.setRepeatMode(Player.REPEAT_MODE_ALL);
        simpleExoPlayer.prepare(videoSource);
        return simpleExoPlayer;
    }

}