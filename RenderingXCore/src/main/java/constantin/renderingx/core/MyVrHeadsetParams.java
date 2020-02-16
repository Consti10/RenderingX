package constantin.renderingx.core;

import android.app.Activity;
import android.content.Context;

import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.GvrViewerParams;

//Unfortunately, to obtain all the distortion params, we have to create and delete a GvrView object

public class MyVrHeadsetParams {
    public final float ScreenWidthMeters;
    public final float ScreenHeightMeters;
    //
    public final float ScreenToLensDistance;
    public final float InterLensDistance;
    //
    public final int VerticalAlignment;
    //
    public final float VerticalDistanceToLensCenter;
    public final float[] fov=new float[4];
    public final float[] kN;
    //
    public final int ScreenWidthPixels;
    public final int ScreenHeightPixels;

    public MyVrHeadsetParams(final Context context){

        GvrView view=new GvrView(context);
        final GvrViewerParams gvrViewerParams=view.getGvrViewerParams();

        fov[0]=gvrViewerParams.getLeftEyeMaxFov().getLeft();
        fov[1]=gvrViewerParams.getLeftEyeMaxFov().getRight();
        fov[2]=gvrViewerParams.getLeftEyeMaxFov().getBottom();
        fov[3]=gvrViewerParams.getLeftEyeMaxFov().getTop();
        kN=gvrViewerParams.getDistortion().getCoefficients();

        ScreenWidthMeters=view.getScreenParams().getWidthMeters();
        ScreenHeightMeters=view.getScreenParams().getHeightMeters();
        ScreenToLensDistance=gvrViewerParams.getScreenToLensDistance();
        InterLensDistance=gvrViewerParams.getInterLensDistance();
        VerticalAlignment=gvrViewerParams.getVerticalAlignment().ordinal();
        VerticalDistanceToLensCenter=gvrViewerParams.getVerticalDistanceToLensCenter();
        ScreenWidthPixels=view.getScreenParams().getWidth();
        ScreenHeightPixels=view.getScreenParams().getHeight();

        view.shutdown();
    }
}
