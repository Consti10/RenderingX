package constantin.renderingx.example.stereo.video360degree;

import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Surface;

import com.google.vr.ndk.base.GvrApi;

import constantin.renderingx.core.VrActivity;
import constantin.renderingx.core.views.MyGLSurfaceView;
import constantin.renderingx.core.views.MyVRLayout;
import constantin.renderingx.example.R;
import constantin.video.core.gl.ISurfaceAvailable;
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
    private static final boolean USE_ANDROID_MEDIA_PLAYER=false;
    private VideoPlayer videoPlayer;
    private MediaPlayer mediaPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final GvrApi gvrApi;
        final MyVRLayout myVRLayout = new MyVRLayout(this);
        gvrApi= myVRLayout.getGvrApi();

        final Bundle bundle=getIntent().getExtras();
        final int SPHERE_MODE=bundle.getInt(KEY_SPHERE_MODE,0);
        final String VIDEO_FILENAME=bundle.getString(KEY_VIDEO_FILENAME);
        //start initialization
        final MyGLSurfaceView gLView = new MyGLSurfaceView(this);
        gLView.setEGLContextClientVersion(2);
        // Use one of both ! Default to the player from VideoCore
        if(USE_ANDROID_MEDIA_PLAYER){
            //TODO doesnt work on mp4 assets because of compression
            mediaPlayer=MediaPlayer.create(this, R.raw.test_room1_1920mono);
        }else{
            VideoSettings.setVS_SOURCE(this, VideoSettings.VS_SOURCE.ASSETS);
            VideoSettings.setVS_ASSETS_FILENAME_TEST_ONLY(this,VIDEO_FILENAME);
            VideoSettings.setVS_FILE_ONLY_LIMIT_FPS(this,40);
            videoPlayer=new VideoPlayer(this);
        }
        // VideoCore provides a convenient callback to use with VideoSurfaceHolder
        final ISurfaceAvailable iSurfaceAvailable=USE_ANDROID_MEDIA_PLAYER ?
                new ISurfaceAvailable() {
                    @Override
                    public void XSurfaceCreated(SurfaceTexture surfaceTexture, Surface surface) {
                        mediaPlayer.setSurface(surface);
                        mediaPlayer.start();
                    }
                    @Override
                    public void XSurfaceDestroyed() {
                        mediaPlayer.pause();
                    }
                } :
                videoPlayer.configure2();
        final Renderer360Video renderer =new Renderer360Video(this,iSurfaceAvailable, gvrApi,false,
                true,SPHERE_MODE);
        gLView.setRenderer(renderer);
        gLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        gLView.setPreserveEGLContextOnPause(true);
        setContentView(myVRLayout);
        myVRLayout.setPresentationView(gLView);
    }

}