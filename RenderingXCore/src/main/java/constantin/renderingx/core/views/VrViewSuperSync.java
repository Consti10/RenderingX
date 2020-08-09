package constantin.renderingx.core.views;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.OnLifecycleEvent;

import java.util.Objects;

import constantin.renderingx.core.xglview.SurfaceTextureHolder;
import constantin.renderingx.core.xglview.XGLSurfaceView;
import constantin.renderingx.core.xglview.XSurfaceParams;


@SuppressLint("ViewConstructor")
public class VrViewSuperSync extends VrView{

    public VrViewSuperSync(AppCompatActivity context) {
        super(context);
        xglSurfaceView.setEGLConfigPrams(new XSurfaceParams(0,0,true));
        xglSurfaceView.DO_SUPERSYNC_MODS=true;
    }


}