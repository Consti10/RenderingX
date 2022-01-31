package constantin.renderingx.core.xglview;

// I do not need anything other than RGB_888 or RGB_8888 for the GL Surface
public class XSurfaceParams {
    // R,G,B channels, default to 8 bits per channel
    final int mR=8,mG=8,mB=8;
    // Alpha is optionally, use either RGBA_8888 or RGB_888 without alpha
    final int mA;
    // If not 0, enables MSAA on the surface. Note that not all devices support all levels of MSAA,
    // query availability first
    int mWantedMSAALevel;
    // add the "EGL_KHR_mutable_render_buffer" flag to switch on/off direct screen rendering
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
