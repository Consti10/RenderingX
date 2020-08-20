package constantin.renderingx.core.views;

import android.annotation.SuppressLint;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import constantin.renderingx.core.xglview.GLContextSurfaceLess;
import constantin.renderingx.core.xglview.SurfaceTextureHolder;
import constantin.renderingx.core.xglview.XGLSurfaceView;
import constantin.renderingx.core.xglview.XSurfaceParams;
import constantin.video.core.gl.ISurfaceTextureAvailable;

// VrView is almost the same as VrLayout but it also holds and creates the OpenGL Surface view
// that is used to render content
@SuppressLint("ViewConstructor")
public class VrView extends VRLayout {
    final XGLSurfaceView xglSurfaceView;

    public VrView(AppCompatActivity context) {
        super(context);
        xglSurfaceView=new XGLSurfaceView(context);
        super.setPresentationView(xglSurfaceView);
    }

    public XGLSurfaceView getPresentationView(){
        return xglSurfaceView;
    }

    public void enableSuperSync(){
        xglSurfaceView.setEGLConfigPrams(new XSurfaceParams(0,0,true));
        xglSurfaceView.DO_SUPERSYNC_MODS=true;
        xglSurfaceView.ENABLE_HIGH_PRIORITY_CONTEXT=true;
    }

    @Override
    public void setPresentationView(View presentationView){
        throw new IllegalStateException("Illegal access to setPresentationView");
    }
}
