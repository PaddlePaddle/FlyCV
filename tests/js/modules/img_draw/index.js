function lineTest() {
    let src_rgba = Module.imread("source_img");
    let src_rgb = new Module.Mat();
    let pts1 = new Module.Point(50, 50);
    let pts2 = new Module.Point(200, 200);
    let scalar = [255, 0, 255, 255];

    Module.cvtColor(src_rgba, src_rgb, Module.ColorConvertType.CVT_PA_RGBA2PA_RGB);

    Module.line(src_rgb, pts1, pts2, scalar, 1, Module.LineType.LINE_8, 0);
    Module.imshow("target_img", src_rgb);

    pts1.delete();
    pts2.delete();
    src_rgba.delete();
    src_rgb.delete();
}

function circleTest() {
    let src = Module.imread("source_img");
    let center = new Module.Point(200, 200);
    let scalar = [255, 0, 255];

    Module.circle(src, center, 2, scalar, 1, Module.LineType.LINE_8, 0);
    Module.imshow("target_img", src);

    src.delete();
}