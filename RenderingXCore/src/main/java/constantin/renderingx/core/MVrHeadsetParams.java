package constantin.renderingx.core;

import android.content.Context;

import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.GvrViewerParams;

// Unfortunately, to obtain all the distortion params, we have to create and delete a GvrView object
// links to the .cpp class with same name

public class MVrHeadsetParams {
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

    public MVrHeadsetParams(final Context context){

        GvrView view=new GvrView(context);
        //GvrViewerParams.createFromUri()

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

    public static String getCurrentGvrViewerModel(final Context context){
        GvrView view=new GvrView(context);
        String ret=view.getGvrViewerParams().getModel();
        view.shutdown();
        return ret;
    }
}
