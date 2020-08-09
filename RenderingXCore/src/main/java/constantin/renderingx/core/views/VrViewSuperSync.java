package constantin.renderingx.core.views;

import android.annotation.SuppressLint;

import androidx.appcompat.app.AppCompatActivity;

import constantin.renderingx.core.xglview.XSurfaceParams;


@SuppressLint("ViewConstructor")
public class VrViewSuperSync extends VrView{

    public VrViewSuperSync(AppCompatActivity context) {
        super(context);
        xglSurfaceView.setEGLConfigPrams(new XSurfaceParams(0,0,true));
        xglSurfaceView.DO_SUPERSYNC_MODS=true;
    }


}