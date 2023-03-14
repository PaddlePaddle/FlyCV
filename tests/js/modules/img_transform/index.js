function cvtColorTest() {
    var src = Module.imread("source_img");
    var dst = new Module.Mat();
    Module.cvtColor(src, dst, Module.ColorConvertType.CVT_PA_RGBA2PA_BGR);
    var temp = new Module.Mat();
    Module.cvtColor(dst, temp, Module.ColorConvertType.CVT_PA_BGR2PA_RGBA);
    Module.imshow("target_img", temp);

    src.delete();
    dst.delete();
    temp.delete();
}

function addWeightedTest() {
    var src1_rgba = Module.imread("source_img1");
    var src2_rgba = Module.imread("source_img2");
    var src1_bgr = new Module.Mat();
    var src2_bgr = new Module.Mat();
    var dst_bgr = new Module.Mat();
    var dst_rgba = new Module.Mat();

    Module.cvtColor(src1_rgba, src1_bgr, Module.ColorConvertType.CVT_PA_RGBA2PA_BGR);
    Module.cvtColor(src2_rgba, src2_bgr, Module.ColorConvertType.CVT_PA_RGBA2PA_BGR);

    Module.addWeighted(src1_bgr, 0.5, src2_bgr, 0.6, 0.8, dst_bgr);
    Module.cvtColor(dst_bgr, dst_rgba, Module.ColorConvertType.CVT_PA_BGR2PA_RGBA);

    Module.imshow("target_img", dst_rgba);
}