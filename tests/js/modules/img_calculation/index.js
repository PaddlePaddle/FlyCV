function meanTest() {
    let src = Module.imread("source_img");

    let mean_scalar = Module.mean(src);
    console.log(mean_scalar);

    let rect = new Module.Rect(0, 0, 100, 100);
    let mean_scalar2 = Module.mean(src, rect);
    console.log(mean_scalar2);

    src.delete();
    rect.delete();
}