// Copyright (c) 2021 FlyCV Authors. All Rights Reserved.
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

#include "modules/img_calculation/find_homography/interface/find_homography.h"

#include <limits>
#include <cmath>
#include <stdlib.h>

#include "modules/img_calculation/matrix_mul/interface/matrix_mul.h"
#include "float.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

Mat find_homography(
        std::vector<Point2f> src_points,
        std::vector<Point2f> dst_points,
        int method) {
    if (method != 0) {
        LOG_ERR("Unsupported method type!");
        return Mat();
    }
    if (src_points.empty() || dst_points.empty()) {
        LOG_ERR("find_homography src or dst points is empty!");
        return Mat();
    }

    Mat src, dst, h_matrix;

    double ltl[9][9], ltw[9][1], ltv[9][9];
    memset(ltl, 0, sizeof(double) * 9 * 9);
    memset(ltw, 0, sizeof(double) * 9);
    memset(ltv, 0, sizeof(double) * 9 * 9);
    const Point2f *src_point_ptr = src_points.data();
    const Point2f *dst_point_ptr = dst_points.data();
    Mat mat_ltl(9, 9, FCVImageType::GRAY_F64, ltl);
    Mat mat_w(9, 1, FCVImageType::GRAY_F64, ltw);
    Mat mat_v(9, 9, FCVImageType::GRAY_F64, ltv);
    Mat mat_h0(3, 3, FCVImageType::GRAY_F64, ltv[8]);
    Mat mat_h_temp(3, 3, FCVImageType::GRAY_F64, ltv[7]);
    Point2d src_c(0, 0), dst_c(0, 0), src_s(0, 0), dst_s(0, 0);

    for (int i = 0; i < 4; i++) {
        dst_c.set_x(dst_c.x() + dst_point_ptr[i].x());
        dst_c.set_y(dst_c.y() + dst_point_ptr[i].y());
        src_c.set_x(src_c.x() + src_point_ptr[i].x());
        src_c.set_y(src_c.y() + src_point_ptr[i].y());
    }

    dst_c.set_x(dst_c.x() / 4);
    dst_c.set_y(dst_c.y() / 4);
    src_c.set_x(src_c.x() / 4);
    src_c.set_y(src_c.y() / 4);

    for (int i = 0; i < 4; i++) {
        dst_s.set_x(dst_s.x() + std::fabs(dst_point_ptr[i].x() - dst_c.x()));
        dst_s.set_y(dst_s.y() + std::fabs(dst_point_ptr[i].y() - dst_c.y()));
        src_s.set_x(src_s.x() + std::fabs(src_point_ptr[i].x() - src_c.x()));
        src_s.set_y(src_s.y() + std::fabs(src_point_ptr[i].y() - src_c.y()));
    }

    dst_s.set_x(4 / dst_s.x());
    dst_s.set_y(4 / dst_s.y());
    src_s.set_x(4 / src_s.x());
    src_s.set_y(4 / src_s.y());

    double inv_hnorm[9] = {1. / dst_s.x(), 0, dst_c.x(), 0, 1. / dst_s.y(), dst_c.y(), 0, 0, 1};
    double hnorm2[9] = {src_s.x(), 0, -src_c.x() * src_s.x(),
            0, src_s.y(), -src_c.y() * src_s.y(), 0, 0, 1};

    Mat mat_inv_hnorm(3, 3, FCVImageType::GRAY_F64, inv_hnorm);
    Mat mat_hnorm2(3, 3, FCVImageType::GRAY_F64, hnorm2);

    for (int i = 0; i < 4; i++) {
        double dst_x = (dst_point_ptr[i].x() - dst_c.x()) * dst_s.x(),
               dst_y = (dst_point_ptr[i].y() - dst_c.y()) * dst_s.y();
        double src_x = (src_point_ptr[i].x() - src_c.x()) * src_s.x(),
               src_y = (src_point_ptr[i].y() - src_c.y()) * src_s.y();
        double l_x[] = {src_x, src_y, 1, 0, 0, 0, -dst_x * src_x, -dst_x * src_y, -dst_x};
        double l_y[] = {0, 0, 0, src_x, src_y, 1, -dst_y * src_x, -dst_y * src_y, -dst_y};
        int j = 0, k = 0;
        for (j = 0; j < 9; j++) {
            for (k = j; k < 9; k++) {
                ltl[j][k] += l_x[j] * l_x[k] + l_y[j] * l_y[k];
            }
        }
    }

    int ltl_step = mat_ltl.stride();
    int esz = 8;
    int rows = mat_ltl.height();
    unsigned char *mat_ltl_ptr = (unsigned char *)mat_ltl.data();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < i; j++) {
            memcpy(mat_ltl_ptr + (i * ltl_step + j * esz),
                   mat_ltl_ptr + (j * ltl_step + i * esz), esz);
        }
    }

    FCVImageType type = mat_ltl.type();
    int raw_step = (rows * esz + 15) & -16;
    static unsigned char *origin_ptr;
    static unsigned long len = 0;

    if (origin_ptr == nullptr
            || len != (unsigned long)(rows * raw_step + rows * 5 * esz + 32)) {
        if (origin_ptr != nullptr) {
            free(origin_ptr);
        }
        len = static_cast<unsigned long>(rows * raw_step + rows * 5 * esz + 32);
        origin_ptr = (unsigned char *)malloc(len);
    }

    unsigned char *ptr = origin_ptr;
    Mat a_mat(rows, rows, type, ptr, raw_step);
    Mat w_mat(1, rows, type, ptr + raw_step * rows);
    ptr += raw_step * rows + esz * rows;
    mat_ltl.copy_to(a_mat);

    double *a_ptr = (double *)a_mat.data();
    double *w_ptr = (double *)w_mat.data();
    double *v_ptr = (double *)mat_v.data();
    int a_stride = a_mat.stride();
    int v_stride = mat_v.stride();

    const double eps = std::numeric_limits<double>::epsilon();
    int idx_i = 0, idx_j = 0, idx_k = 0, idx_m = 0;

    a_stride /= sizeof(a_ptr[0]);
    if (v_ptr) {
        v_stride /= sizeof(v_ptr[0]);
        for (idx_i = 0; idx_i < rows; idx_i++) {
            for (idx_j = 0; idx_j < rows; idx_j++){
                v_ptr[idx_i * v_stride + idx_j] = (double)0;
            }
            v_ptr[idx_i * v_stride + idx_i] = (double)1;
        }
    }

    int iters = 0;
    int max_iters = rows * rows * 60;
    int *ind_r = (int *)(((size_t)ptr + sizeof(int) - 1) & -sizeof(int));
    int *ind_c = ind_r + rows;
    double mv = (double)0;

    for (idx_k = 0; idx_k < rows; idx_k++) {
        w_ptr[idx_k] = a_ptr[(a_stride + 1) * idx_k];
        if (idx_k < rows - 1) {
            for (idx_m = idx_k + 1, mv = std::abs(a_ptr[a_stride * idx_k + idx_m]),
                    idx_i = idx_k + 2; idx_i < rows; idx_i++) {
                double val = std::abs(a_ptr[a_stride * idx_k + idx_i]);
                if (mv < val){
                    mv = val, idx_m = idx_i;
                }
            }
            ind_r[idx_k] = idx_m;
        }
        if (idx_k > 0) {
            for (idx_m = 0, mv = std::abs(a_ptr[idx_k]), idx_i = 1;
                 idx_i < idx_k; idx_i++) {
                double val = std::abs(a_ptr[a_stride * idx_i + idx_k]);
                if (mv < val){
                    mv = val, idx_m = idx_i;
                }
            }
            ind_c[idx_k] = idx_m;
        }
    }

    if (rows > 1) {
        for (iters = 0; iters < max_iters; iters += 1 + (iters / 14)) {
            for (idx_k = 0, mv = std::abs(a_ptr[ind_r[0]]), idx_i = 1;
                    idx_i < rows - 1; idx_i++) {
                double val = std::abs(a_ptr[a_stride * idx_i + ind_r[idx_i]]);
                if (mv < val) {
                    mv = val, idx_k = idx_i;
                }
            }
            int l = ind_r[idx_k];
            for (idx_i = 1; idx_i < rows; idx_i++) {
                double val = std::abs(a_ptr[a_stride * ind_c[idx_i] + idx_i]);
                if (mv < val) {
                    mv = val, idx_k = ind_c[idx_i], l = idx_i;
                }
            }

            double p = a_ptr[a_stride * idx_k + l];
            if (std::abs(p) <= eps){
                break;
            }
            double y = (double)((w_ptr[l] - w_ptr[idx_k]) * 0.5);
            double t = std::abs(y) + hypot(p, y);
            double s = hypot(p, t);
            double c = t / s;
            s = p / s;
            t = (p / t) * p;
            if (y < 0){
                s = -s, t = -t;
            }
            a_ptr[a_stride * idx_k + l] = 0;

            w_ptr[idx_k] -= t;
            w_ptr[l] += t;

            double a0 = 0, b0 = 0;
            double *v0 = nullptr, *v1 = nullptr;
            for (idx_i = 0; idx_i < idx_k; idx_i++) {
                v0 = &a_ptr[a_stride * idx_i + idx_k],
                v1 = &a_ptr[a_stride * idx_i + l];
                a0 = *v0, b0 = *v1;
                *v0 = a0 * c - b0 * s, *v1 = a0 * s + b0 * c;
            }
            for (idx_i = idx_k + 1; idx_i < l; idx_i++) {
                v0 = &a_ptr[a_stride * idx_k + idx_i],
                v1 = &a_ptr[a_stride * idx_i + l];
                a0 = *v0, b0 = *v1;
                *v0 = a0 * c - b0 * s, *v1 = a0 * s + b0 * c;
            }
            for (idx_i = l + 1; idx_i < rows; idx_i++) {
                v0 = &a_ptr[a_stride * idx_k + idx_i],
                v1 = &a_ptr[a_stride * l + idx_i];
                a0 = *v0, b0 = *v1;
                *v0 = a0 * c - b0 * s, *v1 = a0 * s + b0 * c;
            }

            if (v_ptr) {
                for (idx_i = 0; idx_i < rows; idx_i++) {
                    v0 = &v_ptr[v_stride * idx_k + idx_i],
                    v1 = &v_ptr[v_stride * l + idx_i];
                    a0 = *v0, b0 = *v1;
                    *v0 = a0 * c - b0 * s, *v1 = a0 * s + b0 * c;
                }
            }

            for (idx_j = 0; idx_j < 2; idx_j++) {
                int idx = idx_j == 0 ? idx_k : l;
                if (idx < rows - 1) {
                    for (idx_m = idx + 1,
                        mv = std::abs(a_ptr[a_stride * idx + idx_m]),
                        idx_i = idx + 2;
                         idx_i < rows; idx_i++) {
                        double val = std::abs(a_ptr[a_stride * idx + idx_i]);
                        if (mv < val) {
                            mv = val, idx_m = idx_i;
                        }
                    }
                    ind_r[idx] = idx_m;
                }
                if (idx > 0) {
                    for (idx_m = 0, mv = std::abs(a_ptr[idx]), idx_i = 1;
                         idx_i < idx; idx_i++) {
                        double val = std::abs(a_ptr[a_stride * idx_i + idx]);
                        if (mv < val) {
                            mv = val, idx_m = idx_i;
                        }
                    }
                    ind_c[idx] = idx_m;
                }
            }
        }
    }

    for (idx_k = 0; idx_k < rows - 1; idx_k++) {
        idx_m = idx_k;
        for (idx_i = idx_k + 1; idx_i < rows; idx_i++) {
            if (w_ptr[idx_m] < w_ptr[idx_i]) {
                idx_m = idx_i;
            }
        }
        if (idx_k != idx_m) {
            std::swap(w_ptr[idx_m], w_ptr[idx_k]);
            if (v_ptr) {
                for (idx_i = 0; idx_i < rows; idx_i++) {
                    std::swap(v_ptr[v_stride * idx_m + idx_i],
                              v_ptr[v_stride * idx_k + idx_i]);
                }
            }
        }
    }

    mat_h_temp = matrix_mul(mat_inv_hnorm, mat_h0);
    mat_h0 = matrix_mul(mat_h_temp, mat_hnorm2);
    mat_h0.convert_to(h_matrix, mat_h0.type(), 1. / mat_h0.at<double>(2, 2));

    return h_matrix;
}

G_FCV_NAMESPACE1_END()
