package constantin.renderingX;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLUtils;

import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.lang.annotation.Native;


/**
 * Holds only static functions that can be called by cpp
 * Including text rendering data without android assets (e.g. as raw data in .cpp files) would make this obsolete and the library
 * more independent from Android
 */

public class NDKHelper {
    /**
     * Upload a texture image stored in the app assets folder to GPU memory (e.g. the currently bound texture unit)
     * Usage:
     * 1) bind any Texture to the OpenGL context (from cpp)
     * 2) call 'uploadAssetImage' via ndk (from cpp)
     * --> pass it a valid Android Context and filename
     * 3) optional: unbind the texture -> the image is now in GPU memory
     * @param androidContext a valid Android context
     * @param imageFilename a valid name of an assets file
     * @param extractAlpha the signed distance fields only use alpha chanel
     */
    private static void uploadAssetImageToGPU(Context androidContext,String imageFilename,boolean extractAlpha){
        Bitmap bmp=getBitmapFromAssets(androidContext,imageFilename);
        if(extractAlpha){
            bmp=bmp.extractAlpha();
        }
        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0,bmp, 0);
        bmp.recycle();
    }

    private static void getFloatArrayFromAssets(Context androidContext,String fileName,float[] input){
        float[] tmp=getFloatArrayFromAssets(androidContext,fileName);
        if(tmp.length!=input.length)throw new RuntimeException();
        System.arraycopy(tmp, 0, input, 0, input.length);
    }

    private static float[] getFloatArrayFromAssets(Context androidContext,String fileName){
        AssetManager am=androidContext.getAssets();
       float[] obj;
        try{
            InputStream is;
            is=am.open(fileName);
            ObjectInputStream ois=new ObjectInputStream(is);
            obj=(float[])ois.readObject();
        }catch (Exception e){
            e.printStackTrace();
            obj=new float[0];
        }
        //for(float f:obj){
        //    System.out.println("Succ"+f);
        //}
        return obj;
    }

    private static Bitmap getBitmapFromAssets(Context context, String fileName){
        AssetManager am = context.getAssets();
        InputStream is = null;
        try{
            is = am.open(fileName);
        }catch(IOException e) {
            e.printStackTrace();
        }
        return BitmapFactory.decodeStream(is);
    }
}
