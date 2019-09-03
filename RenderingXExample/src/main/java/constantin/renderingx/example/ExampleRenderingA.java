package constantin.renderingx.example;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.SeekBar;
import android.widget.Spinner;

import constantin.renderingX.PerformanceHelper;

public class ExampleRenderingA extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {
    private GLSurfaceView gLView;
    private GLRExample renderer;
    private SeekBar seekBar1,seekBar2,seekBar3;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_example_rendering);
        seekBar1=findViewById(R.id.seekBar1);
        seekBar2=findViewById(R.id.seekBar2);
        seekBar3=findViewById(R.id.seekBar3);
        gLView = findViewById(R.id.GLSurfaceView);
        gLView.setEGLContextClientVersion(2);
        renderer=new GLRExample(this);
        gLView.setRenderer(renderer);
        gLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        gLView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                renderer.touch(event);
                return true;
            }
        });
        gLView.setZOrderOnTop(false);
        final Spinner spinner=findViewById(R.id.spinner);
        spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                renderer.selectedMode(position);
                if(position==0 || position==1){
                    enableDisableSeekBars(true);
                }else{
                    enableDisableSeekBars(false);
                }
            }
            @Override
            public void onNothingSelected(AdapterView<?> parent) { }
        });
        seekBar1.setOnSeekBarChangeListener(this);
        seekBar2.setOnSeekBarChangeListener(this);
        seekBar3.setOnSeekBarChangeListener(this);
        enableDisableSeekBars(true);
    }

    @Override
    protected void onResume() {
        super.onResume();
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    private void enableDisableSeekBars(boolean enable){
        if(enable){
            seekBar1.setVisibility(View.VISIBLE);
            seekBar2.setVisibility(View.VISIBLE);
            seekBar3.setVisibility(View.VISIBLE);
        }else{
            seekBar1.setVisibility(View.GONE);
            seekBar2.setVisibility(View.GONE);
            seekBar3.setVisibility(View.GONE);
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        System.out.println("SeekBar values:"+seekBar1.getProgress()+" "+seekBar2.getProgress());
        renderer.setSeekBarValues(seekBar1.getProgress(),seekBar2.getProgress(),seekBar3.getProgress());
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) { }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) { }
}
