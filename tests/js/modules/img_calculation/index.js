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

function normTest() {
    let src = Module.imread("source_img");

    let norm_inf = Module.norm(src, Module.NormType.NORM_INF);
    console.log(norm_inf);
    let norm_l1 = Module.norm(src, Module.NormType.NORM_L1);
    console.log(norm_l1);
    let norm_l2 = Module.norm(src, Module.NormType.NORM_L2);
    console.log(norm_l2);

    src.delete();
}