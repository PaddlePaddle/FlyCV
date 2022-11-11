package com.org.flycv.samples.imagetransform;

public class ImageTramsform {

    static {
        System.loadLibrary("FlyCV_1.0");
    }

    public static void cvColor() {
        nativeCvColor();
    }

    public static void cvColorCL() {
        nativeCvColorCL();
    }

    public static void cvResize() {
        nativeResize();
    }

    public static void cvResizeCL() {
        nativeResizeCL();
    }

    private static native void nativeCvColor();

    private static native void nativeCvColorCL();

    private static native void nativeResize();

    private static native void nativeResizeCL();

}
