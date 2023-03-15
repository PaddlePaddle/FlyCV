function cvtColorTest() {
    let src = Module.imread("source_img");
    let dst = new Module.Mat();
    Module.cvtColor(src, dst, Module.ColorConvertType.CVT_PA_RGBA2PA_BGR);
    let temp = new Module.Mat();
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

    src1_rgba.delete();
    src2_rgba.delete();
    src1_bgr.delete();
    src2_bgr.delete();
    dst_bgr.delete();
    dst_rgba.delete();
}

function extractChannelTest() {
    let src_rgba = Module.imread("source_img");
    let src_bgr = new Module.Mat();
    let dst_gray1 = new Module.Mat();
    let dst_rgba1 = new Module.Mat();
    let dst_gray2 = new Module.Mat();
    let dst_rgba2 = new Module.Mat();
    let dst_gray3 = new Module.Mat();
    let dst_rgba3 = new Module.Mat();

    Module.cvtColor(src_rgba, src_bgr, Module.ColorConvertType.CVT_PA_RGBA2PA_BGR);
    Module.extractChannel(src_bgr, dst_gray1, 0);
    Module.extractChannel(src_bgr, dst_gray2, 1);
    Module.extractChannel(src_bgr, dst_gray3, 2);
    Module.cvtColor(dst_gray1, dst_rgba1, Module.ColorConvertType.CVT_GRAY2PA_RGBA);
    Module.cvtColor(dst_gray2, dst_rgba2, Module.ColorConvertType.CVT_GRAY2PA_RGBA);
    Module.cvtColor(dst_gray3, dst_rgba3, Module.ColorConvertType.CVT_GRAY2PA_RGBA);

    Module.imshow("target_img1", dst_rgba1);
    Module.imshow("target_img2", dst_rgba2);
    Module.imshow("target_img3", dst_rgba3);

    src_rgba.delete();
    src_bgr.delete();
    dst_gray1.delete();
    dst_rgba1.delete();
    dst_gray2.delete();
    dst_rgba2.delete();
    dst_gray3.delete();
    dst_rgba3.delete();
}

function flipTest() {
    let src = Module.imread("source_img");
    let dst_x = new Module.Mat();
    let dst_y = new Module.Mat();
    Module.flip(src, dst_x, Module.FlipType.X);
    Module.flip(src, dst_y, Module.FlipType.Y);

    Module.imshow("target_img1", dst_x);
    Module.imshow("target_img2", dst_y);

    src.delete();
    dst_x.delete();
    dst_y.delete();
}

function transposeTest() {
    let src = Module.imread("source_img");
    let dst = new Module.Mat();

    Module.transpose(src, dst);
    Module.imshow("target_img", dst);

    src.delete();
    dst.delete();
}

function copyMakeBorderTest() {
    let src = Module.imread("source_img");
    let dst = new Module.Mat();
    let colorArr = [255, 0, 255, 255];

    Module.copyMakeBorder(src, dst, 10, 20, 30, 40, Module.BorderType.BORDER_CONSTANT, colorArr);
    Module.imshow("target_img", dst);

    src.delete();
    dst.delete();
}

function cropTest() {
    let src = Module.imread("source_img");
    let dst = new Module.Mat();
    let rect = new Module.Rect(50, 50, 200, 300);

    Module.crop(src, dst, rect);
    Module.imshow("target_img", dst);

    rect.delete();
    src.delete();
    dst.delete();
}

function resizeTest() {
    let src = Module.imread("source_img");
    let dst = new Module.Mat();
    let size = new Module.Size(180, 320);

    Module.resize(src, dst, size, 0, 0, Module.InterpolationType.INTER_LINEAR);
    Module.imshow("target_img", dst);

    size.delete();
    src.delete();
    dst.delete();
}

function subtractTest() {
    let src_u8 = Module.imread("source_img");
    let src_f32 = new Module.Mat();
    let dst = new Module.Mat();
    let scalar = [5, 55, 0, 55];

    src_u8.convertTo(src_f32, Module.FCVImageType.PKG_RGBA_F32, 1.0, 0);
    Module.subtract(src_f32, scalar, dst);
    console.log(src_f32.data())

    src_u8.delete();
    src_f32.delete();
    dst.delete();
}