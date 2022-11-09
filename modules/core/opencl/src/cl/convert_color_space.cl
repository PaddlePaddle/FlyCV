__kernel void yuv_to_bgr(__global unsigned char *source_image,
                         __global unsigned char *dest_image,
                         int src_width,
                         int src_height) {
  int pos_x = get_global_id(0);
  int pos_y = get_global_id(1);

  int map_size = src_width * src_height,
      pos_src = pos_y / 2 * src_width + pos_x,
      pos_dst = 3 * (pos_y * src_width + pos_x);

  int Y = source_image[mad24(pos_y, src_width, pos_x)];
  int U = source_image[map_size + pos_src / 2];
  int V = source_image[map_size * 5 / 4 + pos_src / 2];

  int B = Y + ((455 * (U - 128)) >> 8);
  int G = Y - (((88 * (U - 128) + 184 * (V - 128))) >> 8);
  int R = Y + ((360 * (V - 128)) >> 8);

  B = clamp(B, 0, 255);
  G = clamp(G, 0, 255);
  R = clamp(R, 0, 255);

  dest_image[pos_dst] = B;
  dest_image[pos_dst + 1] = G;
  dest_image[pos_dst + 2] = R;
}

__kernel void nv12_to_bgr(__global unsigned char *source_image,
                          __global unsigned char *dest_image, int src_width,
                          int src_height) {
  int pos_x = get_global_id(0);
  int pos_y = get_global_id(1);

  int map_size = mul24(src_width, src_height),
      pos_dst = mul24(3, mad24(pos_y, src_width, pos_x));

  int Y = source_image[mad24(pos_y, src_width, pos_x)];
  int U = source_image[map_size + pos_y / 2 * src_width + pos_x / 2 * 2];
  int V = source_image[map_size + pos_y / 2 * src_width + pos_x / 2 * 2 + 1];

  int B = Y + ((455 * (U - 128)) >> 8);
  int G = Y - (((88 * (U - 128) + 184 * (V - 128))) >> 8);
  int R = Y + ((360 * (V - 128)) >> 8);

  R = clamp(R, 0, 255);
  B = clamp(B, 0, 255);
  G = clamp(G, 0, 255);

  dest_image[pos_dst] = B;
  dest_image[pos_dst + 1] = G;
  dest_image[pos_dst + 2] = R;
}

__kernel void nv21_to_bgr(__global unsigned char *source_image,
                          __global unsigned char *dest_image, int src_width,
                          int src_height) {
  int pos_x = get_global_id(0);
  int pos_y = get_global_id(1);
  int map_size = mul24(src_width, src_height),
      pos_dst = mul24(3, mad24(pos_y, src_width, pos_x));

  int Y = source_image[mad24(pos_y, src_width, pos_x)];
  int U = source_image[map_size + pos_y / 2 * src_width + pos_x / 2 * 2 + 1];
  int V = source_image[map_size + pos_y / 2 * src_width + pos_x / 2 * 2];

  int B = Y + ((455 * (U - 128)) >> 8);
  int G = Y - (((88 * (U - 128) + 184 * (V - 128))) >> 8);
  int R = Y + ((360 * (V - 128)) >> 8);

  R = clamp(R, 0, 255);
  B = clamp(B, 0, 255);
  G = clamp(G, 0, 255);

  dest_image[pos_dst] = B;
  dest_image[pos_dst + 1] = G;
  dest_image[pos_dst + 2] = R;
}