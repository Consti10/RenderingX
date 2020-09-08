package constantin.renderingx.core.xglview;

// I do not need anything other than RGB_888 or RGB_8888 for the GL Surface
public class XSurfaceParams {
    // R,G,B channels, default to 8 bits per channel
    final int mR=8,mG=8,mB=8;
    // Alpha is optionally, use either RGBA_8888 or RGB_888 without alpha
    final int mA;
    int mWantedMSAALevel;
    boolean mUseMutableFlag;
    public XSurfaceParams(final int a, final int msaaLevel, final boolean useMutable){
        mA=a;
        mWantedMSAALevel=msaaLevel;
        mUseMutableFlag=useMutable;
    }
    public XSurfaceParams(final int alpha, final int msaaLevel){
        mA=alpha;
        mWantedMSAALevel=msaaLevel;
        mUseMutableFlag=false;
    }
    public static XSurfaceParams RGB(final int wantedMSAALevel, final boolean useMutable){
        return new XSurfaceParams(0,wantedMSAALevel,useMutable);
    }
    public static XSurfaceParams RGBA(final int wantedMSAALevel, final boolean useMutable){
        return new XSurfaceParams(8,wantedMSAALevel,useMutable);
    }
}
