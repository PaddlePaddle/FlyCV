__kernel void bgr_warp_affine(__global unsigned char *source_image,
                         __global unsigned char *dest_image,
                         __global float *m_data,
                         int src_width, int src_height) {
  // dest_image(pos_x,pos_y)
  int pos_x = get_global_id(0);
  int pos_y = get_global_id(1);

  float src_x = pos_x * m_data[0] + pos_y * m_data[1] + m_data[2];
  float src_y = pos_x * m_data[3] + pos_y * m_data[4] + m_data[5];

  // q1(0,0)  q2(1,0)  q3(0,1)  q4(1,1)

  int src_x_00 = (int)src_x;
  float ux0 = src_x - src_x_00;
  float ux1 = 1.0f - ux0;

  int src_y_01 = (int)src_y;
  float vy0 = src_y - src_y_01;
  float vy1 = 1.0f - vy0;

  // 𝑓(𝑖+𝑢, 𝑗+𝑣)=(1−𝑢)(1−𝑣)𝑓(𝑖,𝑗)+(1−𝑢)𝑣𝑓(𝑖,𝑗+1)+𝑢(1−𝑣)𝑓(𝑖+1,𝑗)+𝑢𝑣(𝑓(𝑖+1,𝑗+1)
  float mul_f1u_f1v = (src_x - src_x_q00) * (src_y - src_y_q01);
  float mul_fu_f1v = (src_x_q10 - src_x) * (src_y - src_y_q01) ;
  float mul_f1u_fv = (src_x - src_x_q00) * (src_y_q11 - src_y);
  float mul_fu_fv = (src_x_q10 - src_x) * (src_y_q11 - src_y);

 }