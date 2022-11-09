// Copyright (c) 2022 FlyCV Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without
// modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright
// notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote
// products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is"
// and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are
// disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any
// direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "modules/img_calculation/connected_components/include/connected_components_common.h"

#include <algorithm>

#include "modules/core/basic_math/interface/basic_math.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#define up2_cur   img_row_2[c]
#define up1_cur   img_row_1[c]
#define cur_cur   img_row[c]
#define down1_cur img_row1[c]

#define down2_left1  c - 1 >= 0 && img_row_2[c - 1]
#define down2_right1 c + 1 < w && img_row_2[c + 1]

#define up1_left2 c - 2 >= 0 && img_row_1[c - 2]
#define up1_left1 c - 1 >= 0 && img_row_1[c - 1]
#define up1_right1 c + 1 < w && img_row_1[c + 1]
#define up1_right2 c + 2 < w && img_row_1[c + 2]

#define cur_left2 c - 2 >= 0 && img_row[c - 2]
#define cur_left1 c - 1 >= 0 && img_row[c - 1]
#define cur_right1 c + 1 < w && img_row[c + 1]

#define down1_left1 c - 1 >= 0 && img_row1[c - 1]
#define down1_right1 c + 1 < w && img_row1[c + 1]

#define SET_LABEL_VALUE(A, X)  X = (A > 0) ? i_label : 0

static void convert_to_s32(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();

    if ((src_w != dst.width()) || (src_h != dst.height())) {
        LOG_ERR("the size of src and labels must be the same!");
        return;
    }

    int src_stride = src.stride();
    int dst_stride = dst.stride() >> 2;

    const unsigned char* src_data = (const unsigned char*)src.data();
    int* dst_data = (int*)dst.data();

    int i = 0, j = 0;
    for (i = 0; i < src_h; ++i) {
        for (j = 0; j < src_w; ++j) {
            dst_data[j] = dst_data[i];
        }
        src_data += src_stride;
        dst_data += dst_stride;
    }
}

int twopass_label4(const Mat& src_img, Mat& lable_img) {
    convert_to_s32(src_img, lable_img);

    int label = 1 ;  // start by 2
    std::vector<int> label_set ;
    label_set.push_back(0) ;   // background: 0

    int* label_img_data = (int*)lable_img.data();
    int rows = lable_img.height();
    int cols = lable_img.width();
    int stride = lable_img.stride() >> 2;
    std::vector<int> vzero(rows, 0);

    // 1. first pass
    for (int i = 0; i < rows; i++) {
        int* data_preRow = i == 0 ? vzero.data() : (label_img_data + (stride * (i - 1)));
        int* data_cur_row = label_img_data + stride * i;
        for (int j = 0; j < cols; j++) {
            if (data_cur_row[j] == 1) {
                std::vector<int> neighbor_labels;
                neighbor_labels.reserve(2);
                int leftpixel = (j == 0) ? 0 : data_cur_row[j - 1] ;
                int uppixel = data_preRow[j] ;
                if (leftpixel > 0) {
                    neighbor_labels.push_back(leftpixel);
                }
                if (uppixel > 0) {
                    neighbor_labels.push_back(uppixel);
                }
                // set a new label
                if (neighbor_labels.empty()) {
                    label_set.push_back(label); // assign to a new label
                    data_cur_row[j] = label;
                    label++;
                    continue;
                } else {
                    // 0 0 1 0 1      x x 2 x 3
                    // 0 1 1 1 1  ->  x 4 2 2 2
                    // 1 1 1 0 1      5 4 2 x 2
                    std::sort(neighbor_labels.begin(), neighbor_labels.end());
                    int smallest_label = neighbor_labels[0];

                    data_cur_row[j] = smallest_label;

                    // save equivalence label set
                    for (size_t k = 1; k < neighbor_labels.size(); k++) {
                        //if left and up pixel > or < right, set them to the smaller
                        int temp_label = neighbor_labels[k];
                        int old_smallest_label = label_set[temp_label];

                        if (old_smallest_label > smallest_label) {
                            label_set[old_smallest_label] = smallest_label;
                            //label_set[temp_label] = smallest_label;
                        } else if (old_smallest_label < smallest_label) {
                            label_set[smallest_label] = old_smallest_label;
                        }
                    }
                }
            }
        }
    }

    // update equivalent labels
    // assigned with the smallest label in each equivalent label set
    //0 0 1 0 1      x x 2 x 3     x x 2 x 3     x x 2 x 3
    //0 1 1 1 1  ->  x 4 2 2 2 ->  x 2 2 2 2 ->  x 2 2 2 2
    //1 1 1 0 1      5 4 2 x 2     5 2 2 x 2     2 2 2 x 2
    int label_size = static_cast<int>(label_set.size());
    for (int i = 1; i < label_size; i++) {
        int curLabel = label_set[i];
        int preLabel = label_set[curLabel];

        while (preLabel != curLabel) {
            curLabel = preLabel;
            preLabel = label_set[preLabel];
        }
        label_set[i] = curLabel;
    }

    int k = 1;
    for (int i = 1; i < label; i++) {
        int curLabel = label_set[i];
        if (curLabel < i) {
            label_set[i] = label_set[curLabel];
        } else {
            label_set[i] = k;
            k++;
        }
    }

    //2. second pass
    for (int i = 0; i < rows; i++) {
        int* data = label_img_data + i * stride;
        for (int j = 0; j < cols; j++) {
            int pixelLabel = data[j];
            data[j] = label_set[pixelLabel];
        }
    }

    return k;
}

//find the root of the tree of node i
inline static int find_root(const int *p, int i){
    int root = i;
    while (p[root] < root) {
        root = p[root];
    }
    return root;
}

//make all nodes in the path of node i point to root
inline static void set_root(int *p, int i, int root){
    while (p[i] < i) {
        int j = p[i];
        p[i] = root;
        i = j;
    }
    p[i] = root;
}

//unite the two trees containing nodes i and j and return the new root
inline static int set_merge(int *p, int i, int j){
    int root = find_root(p, i);
    if (i != j) {
        int rootj = find_root(p, j);
        if (root > rootj){
            root = rootj;
        }
        set_root(p, j, root);
    }
    set_root(p, i, root);
    return root;
}

/**
    Only for 8-connectivity
    Based on "Optimized  Block-based Connected Components Labeling with Decision Trees"
    mainly refer to the implement of OpenCV
**/
static int label_grana(const Mat& src, Mat& labels) {
    const unsigned char* src_data = (unsigned char*)src.data();
    int* labels_data = (int*)labels.data();
    const int w = src.width();
    const int h = src.height();
    const int labels_w = labels.width();
    const int labels_h = labels.height();
    const int src_stride = src.stride();
    const int dst_stride = labels.stride() >> 2;
    std::vector<unsigned char> img_vzero(w * 3, 0);
    std::vector<int> label_vzero(labels_w, 0);

    const int point_sum = w * h;
    int* p = (int*)(malloc)(point_sum * sizeof(int));
    memset(p, 0, point_sum * sizeof(int));
    int label_id = 1;
    int r = 0, c = 0;

    // First scan work with 2x2 blocks
    for (r = 0; r < h; r += 2) {
        // Get rows pointer
        const unsigned char* img_row = src_data + (r * src_stride);
        const unsigned char* img_row1  = (r + 1) > h ?
                img_vzero.data() : (img_row + src_stride);
        const unsigned char* img_row_1 = (r < 1) ?
                (img_vzero.data() + w) : (img_row - src_stride);
        const unsigned char* img_row_2 = (r < 2) ?
                (img_vzero.data() + w + w) : (img_row_1 - src_stride);

        int* labels_row = labels_data + (r * dst_stride);
        int* labels_row_2 = (r < 2) ?
                label_vzero.data() : (labels_row - dst_stride - dst_stride);

        for (c = 0; c < w; c += 2) {
            /* This is a decision tree which allows to choose which action to perform,
            checking as few conditions as possible */
            if (cur_cur) {
                if (cur_left1) {
                    if (up1_right1) {
                        if (up1_cur) {
                            labels_row[c] = labels_row[c - 2];
                            continue;
                        } else {
                            if (up2_cur) {
                                if (up1_left1) {
                                    labels_row[c] = labels_row[c - 2];
                                    continue;
                                } else {
                                    if (up1_left2) {
                                        if (down2_left1) {
                                            labels_row[c] = labels_row[c - 2];
                                            continue;
                                        } else {
                                            labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                            continue;
                                        }
                                    } else {
                                        labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                        continue;
                                    }
                                }
                            } else {
                                labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                continue;
                            }
                        }
                    } else {
                        if (cur_right1) {
                            if (up1_right2) {
                                if (down2_right1) {
                                    if (up1_cur) {
                                        labels_row[c] = labels_row[c - 2];
                                        continue;
                                    } else {
                                        if (up2_cur) {
                                            if (up1_left1) {
                                                labels_row[c] = labels_row[c - 2];
                                                continue;
                                            } else {
                                                if (up1_left2) {
                                                    if (down2_left1) {
                                                        labels_row[c] = labels_row[c - 2];
                                                        continue;
                                                    } else {
                                                        labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                        continue;
                                                    }
                                                } else {
                                                    labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                    continue;
                                                }
                                            }
                                        } else {
                                            labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                            continue;
                                        }
                                    }
                                } else {
                                    labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                    continue;
                                    }
                                } else {
                                    labels_row[c] = labels_row[c - 2];
                                    continue;
                                }
                        } else {
                            labels_row[c] = labels_row[c - 2];
                            continue;
                        }
                    }
                } else {
                    if (down1_left1) {
                        if (up1_right1) {
                            if (cur_left2) {
                                if (up1_left1) {
                                    if (up1_cur) {
                                        labels_row[c] = labels_row[c - 2];
                                        continue;
                                    } else {
                                        if (up2_cur) {
                                            labels_row[c] = labels_row[c - 2];
                                            continue;
                                        } else {
                                            labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                            continue;
                                        }
                                    }
                                } else {
                                    if (up1_left2) {
                                        if (down2_left1) {
                                            if (up1_cur) {
                                                labels_row[c] = labels_row[c - 2];
                                                continue;
                                            } else {
                                                if (up2_cur) {
                                                    labels_row[c] = labels_row[c - 2];
                                                    continue;
                                                } else {
                                                    labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                    continue;
                                                }
                                            }
                                        } else {
                                            labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                            continue;
                                        }
                                    } else {
                                        labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                        continue;
                                    }
                                }
                            } else {
                                if (up1_cur) {
                                    labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                    continue;
                                } else {
                                    if (up1_left1) {
                                        if (up2_cur) {
                                            labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                            continue;
                                        } else {
                                            labels_row[c] = set_merge(p, set_merge(p, labels_row_2[c - 2], labels_row_2[c]), labels_row[c - 2]);
                                            continue;
                                        }
                                    }
                                    else {
                                        labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                        continue;
                                    }
                                }
                            }
                        } else {
                            if (cur_right1) {
                                if (up1_right2) {
                                    if (cur_left2) {
                                        if (up1_left1) {
                                            if (down2_right1) {
                                                if (up1_cur) {
                                                    labels_row[c] = labels_row[c - 2];
                                                    continue;
                                                } else {
                                                    if (up2_cur) {
                                                        labels_row[c] = labels_row[c - 2];
                                                        continue;
                                                    } else {
                                                        labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                        continue;
                                                    }
                                                }
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                continue;
                                            }
                                        } else {
                                            if (down2_right1) {
                                                if (up1_left2) {
                                                    if (down2_left1) {
                                                        if (up1_cur) {
                                                            labels_row[c] = labels_row[c - 2];
                                                            continue;
                                                        } else {
                                                            if (up2_cur) {
                                                                labels_row[c] = labels_row[c - 2];
                                                                continue;
                                                            } else {
                                                                labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                                continue;
                                                            }
                                                        }
                                                    } else {
                                                        labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                        continue;
                                                    }
                                                } else {
                                                    labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                    continue;
                                                }
                                            } else {
                                                if (up1_cur) {
                                                    if (up1_left2) {
                                                        if (down2_left1) {
                                                            labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                            continue;
                                                        } else {
                                                            labels_row[c] = set_merge(p, set_merge(p, labels_row_2[c], labels_row_2[c + 2]), labels_row[c - 2]);
                                                            continue;
                                                        }
                                                    } else {
                                                        labels_row[c] = set_merge(p, set_merge(p, labels_row_2[c], labels_row_2[c + 2]), labels_row[c - 2]);
                                                        continue;
                                                    }
                                                } else {
                                                    labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                    continue;
                                                }
                                            }
                                        }
                                    } else {
                                        if (up1_cur) {
                                            if (down2_right1) {
                                                labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                continue;
                                            } else {
                                                labels_row[c] = set_merge(p, set_merge(p, labels_row_2[c], labels_row_2[c + 2]), labels_row[c - 2]);
                                                continue;
                                            }
                                        } else {
                                            if (up1_left1) {
                                                if (down2_right1) {
                                                    if (up2_cur) {
                                                        labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                        continue;
                                                    }
                                                    else {
                                                        labels_row[c] = set_merge(p, set_merge(p, labels_row_2[c - 2], labels_row_2[c + 2]), labels_row[c - 2]);
                                                        continue;
                                                    }
                                                } else {
                                                    labels_row[c] = set_merge(p, set_merge(p, labels_row_2[c - 2], labels_row_2[c + 2]), labels_row[c - 2]);
                                                    continue;
                                                }
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                continue;
                                            }
                                        }
                                    }
                                } else {
                                    if (up1_left1) {
                                        if (cur_left2) {
                                            labels_row[c] = labels_row[c - 2];
                                            continue;
                                        } else {
                                            labels_row[c] = set_merge(p, labels_row_2[c - 2], labels_row[c - 2]);
                                            continue;
                                        }
                                    } else {
                                        if (up1_cur) {
                                            if (cur_left2) {
                                                if (up1_left2) {
                                                    if (down2_left1) {
                                                        labels_row[c] = labels_row[c - 2];
                                                        continue;
                                                    } else {
                                                        labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                        continue;
                                                    }
                                                } else {
                                                    labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                    continue;
                                                }
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                continue;
                                            }
                                        } else {
                                            labels_row[c] = labels_row[c - 2];
                                            continue;
                                        }
                                    }
                                }
                            } else {
                                if (up1_left1) {
                                    if (cur_left2) {
                                        labels_row[c] = labels_row[c - 2];
                                        continue;
                                    } else {
                                        labels_row[c] = set_merge(p, labels_row_2[c - 2], labels_row[c - 2]);
                                        continue;
                                    }
                                } else {
                                    if (up1_cur) {
                                        if (cur_left2) {
                                            if (up1_left2) {
                                                if (down2_left1) {
                                                    labels_row[c] = labels_row[c - 2];
                                                    continue;
                                                } else {
                                                    labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                    continue;
                                                }
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                continue;
                                            }
                                        } else {
                                            labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                            continue;
                                        }
                                    } else {
                                        labels_row[c] = labels_row[c - 2];
                                        continue;
                                    }
                                }
                            }
                        }
                    } else {
                        if (up1_right1) {
                            if (up1_cur) {
                                labels_row[c] = labels_row_2[c];
                                continue;
                            } else {
                                if (up1_left1) {
                                    if (up2_cur) {
                                        labels_row[c] = labels_row_2[c];
                                        continue;
                                    } else {
                                        labels_row[c] = set_merge(p, labels_row_2[c - 2], labels_row_2[c]);
                                        continue;
                                    }
                                } else {
                                    labels_row[c] = labels_row_2[c];
                                    continue;
                                }
                            }
                        } else {
                            if (cur_right1) {
                                if (up1_right2) {
                                    if (up1_cur) {
                                        if (down2_right1) {
                                            labels_row[c] = labels_row_2[c + 2];
                                            continue;
                                        } else {
                                            labels_row[c] = set_merge(p, labels_row_2[c], labels_row_2[c + 2]);
                                            continue;
                                        }
                                    } else {
                                        if (up1_left1) {
                                            if (down2_right1) {
                                                if (up2_cur) {
                                                    labels_row[c] = labels_row_2[c + 2];
                                                    continue;
                                                } else {
                                                    labels_row[c] = set_merge(p, labels_row_2[c - 2], labels_row_2[c + 2]);
                                                    continue;
                                                }
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c - 2], labels_row_2[c + 2]);
                                                continue;
                                            }
                                        } else {
                                            labels_row[c] = labels_row_2[c + 2];
                                            continue;
                                        }
                                    }
                                } else {
                                    if (up1_cur) {
                                        labels_row[c] = labels_row_2[c];
                                        continue;
                                    } else {
                                        if (up1_left1) {
                                            labels_row[c] = labels_row_2[c - 2];
                                            continue;
                                        } else {
                                            labels_row[c] = label_id;
                                            p[label_id] = label_id;
                                            label_id = label_id + 1;
                                            continue;
                                        }
                                    }
                                }
                            } else {
                                if (up1_cur) {
                                    labels_row[c] = labels_row_2[c];
                                    continue;
                                } else {
                                    if (up1_left1) {
                                        labels_row[c] = labels_row_2[c - 2];
                                        continue;
                                    } else {
                                        labels_row[c] = label_id;
                                        p[label_id] = label_id;
                                        label_id = label_id + 1;
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                if (down1_cur) {
                    if (cur_right1) {
                        if (cur_left1) {
                            if (up1_right1) {
                                if (up1_cur) {
                                    labels_row[c] = labels_row[c - 2];
                                    continue;
                                } else {
                                    if (up2_cur) {
                                        if (up1_left1) {
                                            labels_row[c] = labels_row[c - 2];
                                            continue;
                                        } else {
                                            if (up1_left2) {
                                                if (down2_left1) {
                                                    labels_row[c] = labels_row[c - 2];
                                                    continue;
                                                } else {
                                                    labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                    continue;
                                                }
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                continue;
                                            }
                                        }
                                    } else {
                                        labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                        continue;
                                    }
                                }
                            } else {
                                if (up1_right2) {
                                    if (down2_right1) {
                                        if (up1_cur) {
                                            labels_row[c] = labels_row[c - 2];
                                            continue;
                                        } else {
                                            if (up2_cur) {
                                                if (up1_left1) {
                                                    labels_row[c] = labels_row[c - 2];
                                                    continue;
                                                } else {
                                                    if (up1_left2) {
                                                        if (down2_left1) {
                                                            labels_row[c] = labels_row[c - 2];
                                                            continue;
                                                        } else {
                                                            labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                            continue;
                                                        }
                                                    } else {
                                                        labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                        continue;
                                                    }
                                                }
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                continue;
                                            }
                                        }
                                    } else {
                                        labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                        continue;
                                    }
                                } else {

                                    labels_row[c] = labels_row[c - 2];
                                    continue;
                                }
                            }
                        } else {
                            if (down1_left1) {
                                if (up1_right1) {
                                    if (cur_left2) {
                                        if (up1_left1) {
                                            if (up1_cur) {
                                                labels_row[c] = labels_row[c - 2];
                                                continue;
                                            } else {
                                                if (up2_cur) {
                                                    labels_row[c] = labels_row[c - 2];
                                                    continue;
                                                } else {
                                                    labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                    continue;
                                                }
                                            }
                                        } else {
                                            if (up1_left2) {
                                                if (down2_left1) {
                                                    if (up1_cur) {
                                                        labels_row[c] = labels_row[c - 2];
                                                        continue;
                                                    } else {
                                                        if (up2_cur) {
                                                            labels_row[c] = labels_row[c - 2];
                                                            continue;
                                                        } else {
                                                            labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                            continue;
                                                        }
                                                    }
                                                } else {
                                                    labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                    continue;
                                                }
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                continue;
                                            }
                                        }
                                    } else {
                                        labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                        continue;
                                    }
                                } else {
                                    if (up1_right2) {
                                        if (down2_right1) {
                                            if (cur_left2) {
                                                if (up1_left1) {
                                                    if (up1_cur) {
                                                        labels_row[c] = labels_row[c - 2];
                                                        continue;
                                                    } else {
                                                        if (up2_cur) {
                                                            labels_row[c] = labels_row[c - 2];
                                                            continue;
                                                        } else {
                                                            labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                            continue;
                                                        }
                                                    }
                                                } else {
                                                    if (up1_left2) {
                                                        if (down2_left1) {
                                                            if (up1_cur) {
                                                                labels_row[c] = labels_row[c - 2];
                                                                continue;
                                                            } else {
                                                                if (up2_cur) {
                                                                    labels_row[c] = labels_row[c - 2];
                                                                    continue;
                                                                } else {
                                                                    labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                                    continue;
                                                                }
                                                            }
                                                        } else {
                                                            labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                            continue;
                                                        }
                                                    } else {
                                                        labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                        continue;
                                                    }
                                                }
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                continue;
                                            }
                                        } else {
                                            if (up1_cur) {
                                                if (cur_left2) {
                                                    if (up1_left1) {
                                                        labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                        continue;
                                                    } else {
                                                        if (up1_left2) {
                                                            if (down2_left1) {
                                                                labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                                continue;
                                                            } else {
                                                                labels_row[c] = set_merge(p, set_merge(p, labels_row_2[c], labels_row_2[c + 2]), labels_row[c - 2]);
                                                                continue;
                                                            }
                                                        } else {
                                                            labels_row[c] = set_merge(p, set_merge(p, labels_row_2[c], labels_row_2[c + 2]), labels_row[c - 2]);
                                                            continue;
                                                        }
                                                    }
                                                } else {
                                                    labels_row[c] = set_merge(p, set_merge(p, labels_row_2[c], labels_row_2[c + 2]), labels_row[c - 2]);
                                                    continue;
                                                }
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c + 2], labels_row[c - 2]);
                                                continue;
                                            }
                                        }
                                    } else {
                                        if (up1_cur) {
                                            if (cur_left2) {
                                                if (up1_left1) {
                                                    labels_row[c] = labels_row[c - 2];
                                                    continue;
                                                } else {
                                                    if (up1_left2) {
                                                        if (down2_left1) {
                                                            labels_row[c] = labels_row[c - 2];
                                                            continue;
                                                        } else {
                                                            labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                            continue;
                                                        }
                                                    } else {
                                                        labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                        continue;
                                                    }
                                                }
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c], labels_row[c - 2]);
                                                continue;
                                            }
                                        } else {
                                            labels_row[c] = labels_row[c - 2];
                                            continue;
                                        }
                                    }
                                }
                            } else {
                                if (up1_right1) {
                                    labels_row[c] = labels_row_2[c];
                                    continue;
                                } else {
                                    if (up1_right2) {
                                        if (up1_cur) {
                                            if (down2_right1) {
                                                labels_row[c] = labels_row_2[c + 2];
                                                continue;
                                            } else {
                                                labels_row[c] = set_merge(p, labels_row_2[c], labels_row_2[c + 2]);
                                                continue;
                                            }
                                        } else {
                                            labels_row[c] = labels_row_2[c + 2];
                                            continue;
                                        }
                                    } else {
                                        if (up1_cur) {
                                            labels_row[c] = labels_row_2[c];
                                            continue;
                                        } else {
                                            labels_row[c] = label_id;
                                            p[label_id] = label_id;
                                            label_id = label_id + 1;
                                            continue;
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        if (down1_left1) {
                            labels_row[c] = labels_row[c - 2];
                            continue;
                        } else {
                            if (cur_left1) {

                                labels_row[c] = labels_row[c - 2];
                                continue;
                            } else {
                                labels_row[c] = label_id;
                                p[label_id] = label_id;
                                label_id = label_id + 1;
                                continue;
                            }
                        }
                    }
                } else {
                    if (cur_right1) {
                        if (up1_right1) {
                            labels_row[c] = labels_row_2[c];
                            continue;
                        } else {
                            if (up1_right2) {
                                if (up1_cur) {
                                    if (down2_right1) {
                                        labels_row[c] = labels_row_2[c + 2];
                                        continue;
                                    } else {
                                        labels_row[c] = set_merge(p, labels_row_2[c], labels_row_2[c + 2]);
                                        continue;
                                    }
                                } else {
                                    labels_row[c] = labels_row_2[c + 2];
                                    continue;
                                }
                            } else {
                                if (up1_cur) {
                                    labels_row[c] = labels_row_2[c];
                                    continue;
                                } else {
                                    labels_row[c] = label_id;
                                    p[label_id] = label_id;
                                    label_id = label_id + 1;
                                    continue;
                                }
                            }
                        }
                    } else {
                        if (down1_right1) {
                            labels_row[c] = label_id;
                            p[label_id] = label_id;
                            label_id = label_id + 1;
                            continue;
                        } else {
                            labels_row[c] = 0;
                            continue;
                        }
                    }
                }
            }
        }
    }

    // Second scan
    int label_count = 1;
    for (int i = 1; i < label_id; i++) {
        int cur_label = p[i];
        if (cur_label < i) {
            p[i] = p[cur_label];
        } else {
            p[i] = label_count;
            label_count++;
        }
    }

    if (labels_h & 1) {
        int labels_w_align2 = labels_w & (~1);
        for (r = 0; r < labels_h; r += 2) {
            const unsigned char* img_row = src_data + (r * src_stride);
            const unsigned char* img_row1 = img_row + src_stride;
            int* labels_row = labels_data + (r * dst_stride);
            int* labels_row_fol = (r + 1 < labels_h) ? (labels_row + dst_stride) : label_vzero.data();

            for (c = 0; c < labels_w_align2; c += 2) {
                int i_label = labels_row[c];
                if (i_label > 0) {
                    i_label = p[i_label];
                    SET_LABEL_VALUE(img_row[c], labels_row[c]);
                    SET_LABEL_VALUE(img_row1[c], labels_row_fol[c]);
                    SET_LABEL_VALUE(img_row[c + 1], labels_row[c + 1]);
                    SET_LABEL_VALUE(img_row1[c + 1], labels_row_fol[c + 1]);
                } else {
                    labels_row[c] = 0;
                    labels_row_fol[c] = 0;
                    labels_row[c + 1] = 0;
                    labels_row_fol[c + 1] = 0;
                }
            }

            for (; c < labels_w; c++) {
                int i_label = labels_row[c];
                if (i_label > 0) {
                    i_label = p[i_label];
                    SET_LABEL_VALUE(img_row[c], labels_row[c]);
                    SET_LABEL_VALUE(img_row1[c], labels_row_fol[c]);
                } else {
                    labels_row[c] = 0;
                    labels_row_fol[c] = 0;
                }
            }
        }
    } else {
        //rows is even
        int labels_w_align2 = labels_w & (~1);
        for (r = 0; r < labels_h; r += 2) {
            const unsigned char* img_row = src_data + (r * src_stride);
            const unsigned char* img_row1 = img_row + src_stride;
            int* labels_row = labels_data + (r * dst_stride);
            int* labels_row_fol = labels_row + dst_stride;

            c = 0;
            for (c = 0; c < labels_w_align2; c += 2) {
                int i_label = labels_row[c];
                if (i_label > 0) {
                    i_label = p[i_label];
                    SET_LABEL_VALUE(img_row[c], labels_row[c]);
                    SET_LABEL_VALUE(img_row1[c], labels_row_fol[c]);
                    SET_LABEL_VALUE(img_row[c + 1], labels_row[c + 1]);
                    SET_LABEL_VALUE(img_row1[c + 1], labels_row_fol[c + 1]);
                } else {
                    labels_row[c] = 0;
                    labels_row_fol[c] = 0;
                    labels_row[c + 1] = 0;
                    labels_row_fol[c + 1] = 0;
                }
            }

            for (; c < labels_w; c++) {
                int i_label = labels_row[c];
                if (i_label > 0) {
                    i_label = p[i_label];
                    SET_LABEL_VALUE(img_row[c], labels_row[c]);
                    SET_LABEL_VALUE(img_row1[c], labels_row_fol[c]);
                } else {
                    labels_row[c] = 0;
                    labels_row_fol[c] = 0;
                }
            }
        }
    }

    if (p != nullptr) {
        free(p);
        p = nullptr;
    }

    return label_count;
}

int connected_components_common(
        const Mat& src,
        Mat& labels,
        int connectivity,
        FCVImageType ltype) {

       if (src.type() != FCVImageType::GRAY_U8) {
            LOG_ERR("the type of src in connected_components must be u8!");
            return -1;
        }

       if (ltype != FCVImageType::GRAY_S32) {
            LOG_ERR("the ltype of connected_components must be s32!");
            return -1;
        }

        if (labels.empty()) {
            labels = Mat(src.width(), src.height(), ltype);
        }

        int count = 0;

       if(connectivity == 8) {
            count = label_grana(src, labels);
       } else if(connectivity == 4){
            //count = twopass_label4(src, labels);
       } else {
            LOG_ERR("the connectivity of connected_components is not supported yet!");
            return -1;
       }

       return count;
}

G_FCV_NAMESPACE1_END()
