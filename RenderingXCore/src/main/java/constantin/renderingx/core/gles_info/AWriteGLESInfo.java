package constantin.renderingx.core.gles_info;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import java.util.ArrayList;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.opengles.GL10;

import constantin.renderingx.core.BuildConfig;
import constantin.renderingx.core.views.MyEGLConfigChooser;

import static constantin.renderingx.core.views.MyEGLConfigChooser.findConfigAttrib;


/*
 * Started from main when
 * a) the app has been installed for the first time
 * b) an update made the check for HW settings necessary again
 * Writes hardware-specific settings in GLESInfo
 */

@SuppressLint("ApplySharedPref")
public class AWriteGLESInfo extends AppCompatActivity {
    private static final String TAG="AWriteGLESInfo";
    private Activity context;
    private GLSurfaceView mTestView;
    //
    public static final String PREFERENCES_TAG="pref_gl_info";
    private static final String SAVED_VERSION_CODE="SAVED_VERSION_CODE";
    private static final String SAVED_BUILD_VERSION="SAVED_BUILD_VERSION";
    //
    private static final int F_BuildConfig_VERSION_CODE=3;//BuildConfig.VERSION_CODE

    //write values when either a) the library was updated or b) the os (android) was updated
    static boolean shouldWriteValues(final Context c){
        //return true;
        final SharedPreferences pref = c.getSharedPreferences(PREFERENCES_TAG, MODE_PRIVATE);
        return pref.getInt(SAVED_VERSION_CODE, 0) != F_BuildConfig_VERSION_CODE ||
                pref.getInt(SAVED_BUILD_VERSION, 0) != android.os.Build.VERSION.SDK_INT;
    }
    static void wroteValues(final Context c){
        final SharedPreferences pref = c.getSharedPreferences(PREFERENCES_TAG, MODE_PRIVATE);
        pref.edit().putInt(SAVED_VERSION_CODE, F_BuildConfig_VERSION_CODE).putInt(SAVED_BUILD_VERSION,android.os.Build.VERSION.SDK_INT).commit();
    }

    public static void writeGLESInfoIfNeeded(final Context c){
        if(shouldWriteValues(c)){
            Log.d(TAG,"onUpdate()");
            //Refresh the HW information
            final String text="FPV-VR needs to detect the CPU and GPU abilities of your phone to adjust power and performance settings.\n"+
                    "Please do not interrupt during testing. This will not reset your settings.";
            final String title="First start or update";
            AlertDialog.Builder builder = new AlertDialog.Builder(c);
            builder.setCancelable(false);
            builder.setMessage(text)
                    .setTitle(title);
            builder.setPositiveButton("Okay", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    Intent i=new Intent();
                    i.setClass(c, AWriteGLESInfo.class);
                    c.startActivity(i);
                }
            });
            AlertDialog dialog = builder.create();
            dialog.show();
        }
    }


    private void exitAfterSuccessfulTesting(){
        wroteValues(context);
        final String text="CPU/GPU abilities successfully tested";
        context.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(context);
                builder.setCancelable(false);
                builder.setMessage(text);
                builder.setPositiveButton("Okay", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        context.finish();
                    }
                });
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        context = this;
        mTestView = new GLSurfaceView(context);

        mTestView.setEGLContextClientVersion(2);
        mTestView.setEGLConfigChooser(new MyTestConfigChooser2());
        //mTestView.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory(false));

        mTestView.setRenderer(new MyTestRenderer());
        setContentView(mTestView);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mTestView.onResume();
    }

    @Override
    protected void onPause() {
        mTestView.onPause();
        super.onPause();
    }

    private final class MyTestConfigChooser2 implements  GLSurfaceView.EGLConfigChooser {
        private static final int EGL_OPENGL_ES3_BIT_KHR = 0x40;

        private void findAllMSAALevels(EGL10 egl, javax.microedition.khronos.egl.EGLDisplay display) {
            int[] configSpec = new int[] {
                    EGL10.EGL_RED_SIZE, 8,
                    EGL10.EGL_GREEN_SIZE, 8,
                    EGL10.EGL_BLUE_SIZE, 8,
                    EGL10.EGL_ALPHA_SIZE, 8,
                    EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
                    EGL10.EGL_SURFACE_TYPE,EGL10.EGL_WINDOW_BIT,
                    //
                    EGL10.EGL_SAMPLE_BUFFERS,1,
                    EGL10.EGL_NONE
            };
            int[] num_config = new int[1];
            if (!egl.eglChooseConfig(display, configSpec, null, 0, num_config)) {
                //No msaa available
                Log.d(TAG,"Cannot find configuration with MSAA enabled 1");
                return;
            }
            int numConfigs = num_config[0];
            if (numConfigs <= 0) {
                Log.d(TAG,"Cannot find configuration with MSAA enabled 2");
                return;
            }
            javax.microedition.khronos.egl.EGLConfig[] configs = new javax.microedition.khronos.egl.EGLConfig[numConfigs];
            if (!egl.eglChooseConfig(display, configSpec, configs, numConfigs,
                    num_config)) {
                Log.d(TAG,"Cannot find configuration with MSAA enabled 3");
                return;
            }

            ArrayList<Integer> list=new ArrayList<>();
            for (javax.microedition.khronos.egl.EGLConfig config : configs) {
                int msaaLevel=findConfigAttrib(egl,display,config,EGL10.EGL_SAMPLES,0);
                if(msaaLevel>0){
                    if(!list.contains(msaaLevel)){
                        list.add(msaaLevel);
                    }
                }
            }
            //This string will hold all MSAA levels. Integers, seperated by a '#'
            StringBuilder allMSAALevels= new StringBuilder("0#");
            //At the end, the chooser will choose the highest MSAA level config, or a config without MSAA if available
            int maxMSAALevel=0;
            for(Integer msaaLevel:list){
                allMSAALevels.append(msaaLevel).append("#");
                if(msaaLevel>maxMSAALevel){
                    maxMSAALevel=msaaLevel;
                }
            }
            //QCOMM GPUs support 2xMSAA and 4xMSAA while
            //MALI GPUs support 4xMSAA and 16xMSAA
            Log.d(TAG,"Max MSAA Level:"+maxMSAALevel);
            Log.d(TAG,"All MSAA Levels:"+allMSAALevels);
            OpenGLESValues.writeResultsMSAA(context,allMSAALevels.toString());
        }

        @Override
        public javax.microedition.khronos.egl.EGLConfig chooseConfig(EGL10 egl, javax.microedition.khronos.egl.EGLDisplay display) {

            //First, do the MSAA levels for RGBA8888
            findAllMSAALevels(egl,display);

            //then use our normal config chooser
            final MyEGLConfigChooser myEGLConfigChooser=new MyEGLConfigChooser(false,0,false);
            return myEGLConfigChooser.chooseConfig(egl,display);
        }
    }


    private final class MyTestRenderer implements GLSurfaceView.Renderer {
        private int nFrames;
        private final float[] rgb = new float[3];
        private double lastTS;
        boolean exitHasBeenCalled;
        boolean pulseGreenColor;
        @Override
        public void onSurfaceCreated(GL10 gl, javax.microedition.khronos.egl.EGLConfig config) {
            nFrames = 0;
            rgb[0] = 0.0f;
            rgb[1] = 0.0f;
            rgb[2] = 0.0f;
            lastTS=System.currentTimeMillis();
            exitHasBeenCalled=false;
            pulseGreenColor=true;
        }
        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {}

        @Override
        public void onDrawFrame(GL10 gl) {
            nFrames++;
            if(pulseGreenColor){
                rgb[1] += 0.02f;
                if (rgb[1] > 1) {
                    rgb[1] = 0;
                }
            }
            if (nFrames == 2) {
                Extensions.storeInSharedPreference(context);
                OpenGLESValues.storeInSharedPreference(context);
            }
            double MAX_TIME_MS = 0;
            if (nFrames > 2 && (System.currentTimeMillis()-lastTS> MAX_TIME_MS && !exitHasBeenCalled)) {
                exitHasBeenCalled=true;
                exitAfterSuccessfulTesting();
                pulseGreenColor=false;
                rgb[1]=1.0f;
            }
            GLES20.glClearColor(rgb[0], rgb[1], rgb[2], 1.0f);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_STENCIL_BUFFER_BIT);
        }

    }

}
