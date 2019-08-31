package constantin.renderingx.example;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import constantin.renderingX.GLESInfo.AWriteGLESInfo;
import constantin.renderingX.GLESInfo.GLESInfo;

public class MainActivity extends AppCompatActivity {
    Context context;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        context=this;
        setContentView(R.layout.activity_main);
        //This retreives any HW info needed for the app
        AWriteGLESInfo.writeGLESInfoIfNeeded(this);

        findViewById(R.id.button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent().setClass(context, ExampleRenderingA.class));
            }
        });
        findViewById(R.id.button2).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(GLESInfo.isExtensionAvailable(context,GLESInfo.EGL_ANDROID_front_buffer_auto_refresh) && GLESInfo.isExtensionAvailable(context,GLESInfo.EGL_KHR_mutable_render_buffer)){
                    startActivity(new Intent().setClass(context, ExampleSuperSyncA.class));
                }else{
                    Toast.makeText(context,"SuperSync is not supported on this phone",Toast.LENGTH_LONG).show();
                }
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
    }


}
