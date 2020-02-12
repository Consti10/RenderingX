package constantin.renderingx.example;

import android.content.Context;
import android.view.Surface;

import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoNative.VideoNative;
import constantin.video.core.VideoPlayer;

//Uses the LiveVideo10ms VideoCore lib which is intended for live streaming, not file playback.
//I recommend using android MediaPlayer if only playback from file is needed

public class MVideoPlayer implements IVideoParamsChanged {

    private final VideoPlayer videoPlayer;
    private final IVideoParamsChanged mVideoParamsChangedI;
    private final Surface mSurface;


    public MVideoPlayer(final Context context,final String assetsFilename,final Surface surface, final IVideoParamsChanged vpc){
        mVideoParamsChangedI=vpc;
        mSurface=surface;
        VideoNative.setVS_SOURCE(context, VideoNative.VS_SOURCE.ASSETS);
        VideoNative.setVS_ASSETS_FILENAME_TEST_ONLY(context,assetsFilename);
        VideoNative.setVS_FILE_ONLY_LIMIT_FPS(context,40);
        videoPlayer=new VideoPlayer(context,this);
    }

    public synchronized void start(){
        videoPlayer.prepare(mSurface);
        videoPlayer.addAndStartReceiver();
    }

    public synchronized void stop(){
        videoPlayer.stopAndRemovePlayerReceiver();
    }

    //called by CPP code
    @Override
    public void onVideoRatioChanged(int videoW, int videoH) {
        if(mVideoParamsChangedI!=null){
            mVideoParamsChangedI.onVideoRatioChanged(videoW,videoH);
        }
    }

    @Override
    public void onDecodingInfoChanged(DecodingInfo decodingInfo) {
        if(mVideoParamsChangedI!=null){
            mVideoParamsChangedI.onDecodingInfoChanged(decodingInfo);
        }
    }

}
