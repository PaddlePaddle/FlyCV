__kernel void matrix_multiply_f32(__global unsigned char *source_image0,
                         __global unsigned char *source_image1,
                         __global unsigned char *dest_image,
                         int src_width1) {

  // dest_image(pos_x,pos_y)
  int pos_x = get_global_id(0);
  int pos_y = get_global_id(1);

  int pos_dst = mul24(4, mad24(pos_y, src_width1, pos_x));

  // c[m][n] = a[m][k] * b[k][n]
  // c[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j];
  int pos_src_i0 = mul24(4, pos_x);
  int pos_src_i1 = mul24(4, mad24(1, src_width1, pos_x));
  int pos_src_0j = mul24(4, mad24(src_width1, pos_y, 0));
  int pos_src_1j = mul24(4, mad24(src_width1, pos_y, 1));

  int C0 = source_image0[pos_src_i0] * source_image1[pos_src_0j]
        + source_image0[pos_src_i1] * source_image1[pos_src_1j];

  int C1 = source_image0[pos_src_i0 + 1] * source_image1[pos_src_0j + 1]
        + source_image0[pos_src_i1 + 1] * source_image1[pos_src_1j + 1];

  int C2 = source_image0[pos_src_i0 + 2] * source_image1[pos_src_0j + 2]
        + source_image0[pos_src_i1 + 2] * source_image1[pos_src_1j + 2];

  int C3 = source_image0[pos_src_i0 + 3] * source_image1[pos_src_0j + 3]
        + source_image0[pos_src_i1 + 3] * source_image1[pos_src_1j + 3];

  C0 = clamp(C0, 0, 255);
  C1 = clamp(C1, 0, 255);
  C2 = clamp(C2, 0, 255);
  C3 = clamp(C3, 0, 255);

  dest_image[pos_dst] = C0;
  dest_image[pos_dst + 1] = C1;
  dest_image[pos_dst + 2] = C2;
  dest_image[pos_dst + 3] = C3;
}