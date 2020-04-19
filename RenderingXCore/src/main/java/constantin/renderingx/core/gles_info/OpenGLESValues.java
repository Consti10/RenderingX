package constantin.renderingx.core.gles_info;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.opengl.GLES20;

import java.util.ArrayList;

import static android.content.Context.MODE_PRIVATE;

// Other than extensions, these are values that need to be queried and may be not boolean values
public class OpenGLESValues {
    private static final String PREF_NAME="pref_gles_values";
    // Int
    public static final String GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS="GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS";
    public static final String GL_MAX_VERTEX_UNIFORM_VECTORS="GL_MAX_VERTEX_UNIFORM_VECTORS";
    // Extra
    private static final String MAX_GL_LINE_WIDTH="MAX_GL_LINE_WIDTH";
    private static final String AllMSAALevels="AllMSAALevels";

    // Call this with OpenGL context bound
    @SuppressLint("ApplySharedPref")
    public static void storeInSharedPreference(final Context c){
        final SharedPreferences pref = c.getSharedPreferences(PREF_NAME, MODE_PRIVATE);
        SharedPreferences.Editor editor=pref.edit();
        writeValueGLES20Integer(editor,GLES20.GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
        writeValueGLES20Integer(editor,GLES20.GL_MAX_VERTEX_UNIFORM_VECTORS,GL_MAX_VERTEX_UNIFORM_VECTORS);
        //
        float[] lineWidthRange= {0.0f, 0.0f};
        GLES20.glGetFloatv(GLES20.GL_LINE_WIDTH, lineWidthRange,0);
        //System.out.println("non aliased: "+lineWidthRange[0]+lineWidthRange[1]);
        GLES20.glGetFloatv(GLES20.GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange,0);
        //System.out.println("Aliased: "+lineWidthRange[0]+lineWidthRange[1]);
        final int MAX_LINE_WIDTH=(int)lineWidthRange[1];
        editor.putInt(MAX_GL_LINE_WIDTH,MAX_LINE_WIDTH);
        //
        editor.commit();
    }

    // Returns the GLES value
    public static void getInt(final Context context,final String key){
        context.getSharedPreferences(PREF_NAME,MODE_PRIVATE).getInt(key,0);
    }

    private static void writeValueGLES20Integer(final SharedPreferences.Editor editor, final int glKey, final String preferencesKey){
        int[] value=new int[1];
        GLES20.glGetIntegerv(glKey,value,0);
        editor.putInt(preferencesKey,value[0]);
        System.out.println("Value for "+preferencesKey+" is:"+value[0]);
    }

    // For MSAA
    static void writeResultsMSAA(final Context c, final String allMSAALevels){
        SharedPreferences pref = c.getSharedPreferences(PREF_NAME, MODE_PRIVATE);
        SharedPreferences.Editor editor=pref.edit();
        editor.putString(AllMSAALevels,allMSAALevels);
        editor.apply();
    }
    public static ArrayList<Integer> availableMSAALevels(final Context context){
        final SharedPreferences pref = context.getSharedPreferences(PREF_NAME, MODE_PRIVATE);
        final String allMSAALevels=pref.getString(AllMSAALevels,"0#");
        final String[] asStringArray=allMSAALevels.split("#");
        final ArrayList<Integer> asIntList=new ArrayList<>();
        for(final String s:asStringArray){
            int msaaLevel=Integer.parseInt(s);
            asIntList.add(msaaLevel);
        }
        return asIntList;
    }

}
