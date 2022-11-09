__kernel void bgr_resize(__global unsigned char *source_image,
                         __global unsigned char *dest_image,
                         int src_width, int src_height,
                         int dst_width, int dst_height) {
  // dest_image(pos_x,pos_y)
  int pos_x = get_global_id(0);
  int pos_y = get_global_id(1);

  if (pos_x >= dst_width || pos_y >= dst_height) {
      return;
  }

  int pos_dst = mul24(3, mad24(pos_y, dst_width, pos_x));

  float scale_w = (float)src_width / dst_width;
  float scale_h = (float)src_height / dst_height;

  float src_x = (pos_x + 0.5f) * scale_w - 0.5f;
  float src_y = (pos_y + 0.5f) * scale_h - 0.5f;

  // q1(0,0)  q2(1,0)  q3(0,1)  q4(1,1)
  int src_x_q00 = (int)src_x;
  int src_x_q10 = ceil(src_x);
  int src_y_q01 = (int)src_y;
  int src_y_q11 = ceil(src_y);

  int pos_src_q00 = mul24(3, mad24(src_y_q01, src_width, src_x_q00));
  int pos_src_q10 = mul24(3, mad24(src_y_q01, src_width, src_x_q10));
  int pos_src_q01 = mul24(3, mad24(src_y_q11, src_width, src_x_q00));
  int pos_src_q11 = mul24(3, mad24(src_y_q11, src_width, src_x_q10));

  // 𝑓(𝑖+𝑢, 𝑗+𝑣)=(1−𝑢)(1−𝑣)𝑓(𝑖,𝑗)+(1−𝑢)𝑣𝑓(𝑖,𝑗+1)+𝑢(1−𝑣)𝑓(𝑖+1,𝑗)+𝑢𝑣(𝑓(𝑖+1,𝑗+1)
  float mul_f1u_f1v = (src_x - src_x_q00) * (src_y - src_y_q01);
  float mul_fu_f1v = (src_x_q10 - src_x) * (src_y - src_y_q01) ;
  float mul_f1u_fv = (src_x - src_x_q00) * (src_y_q11 - src_y);
  float mul_fu_fv = (src_x_q10 - src_x) * (src_y_q11 - src_y);

  int B = (source_image[pos_src_q00] * mul_f1u_f1v)
  + (source_image[pos_src_q10] * mul_fu_f1v)
  + (source_image[pos_src_q01] * mul_f1u_fv)
  + (source_image[pos_src_q11] * mul_fu_fv);

  int G = (source_image[pos_src_q00 + 1] * mul_f1u_f1v)
  + (source_image[pos_src_q10 + 1] * mul_fu_f1v)
  + (source_image[pos_src_q01 + 1] * mul_f1u_fv)
  + (source_image[pos_src_q11 + 1] * mul_fu_fv);

  int R = (source_image[pos_src_q00 + 2] * mul_f1u_f1v)
  + (source_image[pos_src_q10 + 2] * mul_fu_f1v)
  + (source_image[pos_src_q01 + 2] * mul_f1u_fv)
  + (source_image[pos_src_q11 + 2] * mul_fu_fv);

  B = clamp(B, 0, 255);
  G = clamp(G, 0, 255);
  R = clamp(R, 0, 255);

  dest_image[pos_dst] = B;
  dest_image[pos_dst + 1] = G;
  dest_image[pos_dst + 2] = R;
}