package constantin.renderingx.core.views;

import android.annotation.SuppressLint;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import constantin.renderingx.core.xglview.GLContextSurfaceLess;
import constantin.renderingx.core.xglview.SurfaceTextureHolder;
import constantin.renderingx.core.xglview.XGLSurfaceView;
import constantin.renderingx.core.xglview.XSurfaceParams;

@SuppressLint("ViewConstructor")
public class VrView extends VRLayout {
    final XGLSurfaceView xglSurfaceView;

    public VrView(AppCompatActivity context) {
        super(context);
        xglSurfaceView=new XGLSurfaceView(context);
        super.setPresentationView(xglSurfaceView);
    }

    public void setRenderer(final XGLSurfaceView.FullscreenRenderer renderer2){
        xglSurfaceView.setRenderer(renderer2);
    }
    public void setRenderer(final XGLSurfaceView.FullscreenRendererWithSurfaceTexture renderer3, final  SurfaceTextureHolder.ISurfaceTextureAvailable iSurfaceTextureAvailable){
        xglSurfaceView.setRenderer(renderer3,iSurfaceTextureAvailable);
    }

    public void setmISecondaryContext(final GLContextSurfaceLess.SecondarySharedContext i){
        xglSurfaceView.setmISecondaryContext(i);
    }

    public void setEGLConfigPrams(final XSurfaceParams wantedSurfaceParams){
        xglSurfaceView.setEGLConfigPrams(wantedSurfaceParams);
    }

    @Override
    public void setPresentationView(View presentationView){
        throw new IllegalStateException("Illegal access to setPresentationView");
    }
}
