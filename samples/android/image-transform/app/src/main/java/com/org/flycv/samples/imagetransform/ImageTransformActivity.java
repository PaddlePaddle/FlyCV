package com.org.flycv.samples.imagetransform;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

/**
 * push test data
 * 1) adb shell
 * 2) cd sdcard
 * 3) mkdir flycv
 * 4) cd flycv
 * 5) mkdir dst
 * 6) exit
 * 7) adb push /flycv/tests/data /sdcard/flycv
 *
 */
public class ImageTransformActivity extends BaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void onClick(View view) {
        TextView tv = (TextView) view;
        if (tv.getText().equals("cvColor")) {
            ImageTramsform.cvColor();
        } else if (tv.getText().equals("cvResize")) {
            ImageTramsform.cvResize();
        }
    }
}

