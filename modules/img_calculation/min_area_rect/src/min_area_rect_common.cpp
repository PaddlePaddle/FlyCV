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

#include "modules/img_calculation/min_area_rect/include/min_area_rect_common.h"

#include <cmath>
#include <algorithm>
#include <cfloat>

#include "modules/core/base/include/utils.h"
#include "modules/core/base/interface/log.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

inline Point2f rotate_90_counterclockwise(const Point2f& input) {
    float x = -input.y();
    float y = input.x();
    return Point2f(x, y);
}

inline Point2f rotate_90_clockwise(const Point2f& input) {
    float x = input.y();
    float y = -input.x();
    return Point2f(x, y);
}

inline Point2f rotate_180(const Point2f& input) {
    float x = -input.x();
    float y = -input.y();
    return Point2f(x, y);
}

inline bool check_first_to_the_right_of_second(
        const Point2f& first,
        const Point2f& second) {
    Point2f temp = rotate_90_clockwise(first);
    return temp.x() * second.x() + temp.y() * second.y() < 0;
}
/**
 * @brief Find the rectangle with the smallest area containing the vertices of the input convex polygon by
 * classical rotating caliper algorithm (reference from Shamos Michael in 1978)
 * @param[in] points Vertices of the input convex polygon
 * @param[in] n Number of vertices above
 * @param[out] box Pointer to stroe result
 */
static void rotating_calipers(
        const Point2f* points,
        int n,
        RotatedRect* box) {
    if (!box || !points) {
        LOG_ERR("Invalid pointer when rotating_calipers!");
    }
    // Every point has its arrow to next point.
    std::vector<Point2f> arrow(n);
    std::vector<float> arrow_length(n);
    // float* inv_vect_length = abuf.data();
    // Point2f* vect = (Point2f*)(inv_vect_length + n);
    int bottom = 0;
    int right = 0;
    int top = 0;
    int left = 0;
    int diagonal_pts[4] = { -1, -1, -1, -1 };
    Point2f rotated_arrow[4];

    Point2f base_edge(0, 0);

    Point2f pt0 = points[0];
    float left_x = pt0.x();
    float right_x = pt0.x();
    float top_y = pt0.y();
    float bottom_y = pt0.y();

    for(int i = 0; i < n; i++) {
        if (pt0.x() < left_x) {
            left_x = pt0.x();
            left = i;
        }
        if (pt0.x() > right_x) {
            right_x = pt0.x();
            right = i;
        }
        if (pt0.y() > top_y) {
            top_y = pt0.y();
            top = i;
        }
        if (pt0.y() < bottom_y) {
            bottom_y = pt0.y();
            bottom = i;
        }
        Point2f next_pt = (i + 1 < n) ? points[i + 1] : points[0];
        // Note : the type of dx dy below must be double! use float will bring deviation
        double dx = next_pt.x() - pt0.x();
        double dy = next_pt.y() - pt0.y();
        arrow[i] = Point2f(float(dx), float(dy));
        arrow_length[i] = float(1.f / std::sqrt(dx * dx + dy * dy));
        pt0 = next_pt;
    }
    // init position
    diagonal_pts[0] = bottom;
    diagonal_pts[1] = right;
    diagonal_pts[2] = top;
    diagonal_pts[3] = left;

    float minarea = FLT_MAX;
    // buffer
    int buf_index_left_pt = 0;
    int buf_index_bottom_pt = 0;
    Point2f buf_base_edge(0., 0.);
    float buf_width = 0.;
    float buf_height = 0.;
    // iterate over each edge
    for(int k = 0; k < n; k++) {
        rotated_arrow[0] = arrow[diagonal_pts[0]];
        rotated_arrow[1] = rotate_90_clockwise(arrow[diagonal_pts[1]]);
        rotated_arrow[2] = rotate_180(arrow[diagonal_pts[2]]);
        rotated_arrow[3] = rotate_90_counterclockwise(arrow[diagonal_pts[3]]);

        int main_element = 0;
        for (int i = 1; i < 4; i++) {
            if (check_first_to_the_right_of_second(rotated_arrow[i], rotated_arrow[main_element])) {
                main_element = i;
            }
        }

        // get next base
        int pindex = diagonal_pts[main_element];
        float lead_x = arrow[pindex].x() * arrow_length[pindex];
        float lead_y = arrow[pindex].y() * arrow_length[pindex];
        switch (main_element) {
        case 0:
            base_edge = Point2f(lead_x, lead_y);
            break;
        case 1:
            base_edge = Point2f(lead_y, -lead_x);
            break;
        case 2:
            base_edge = Point2f(-lead_x, -lead_y);
            break;
        case 3:
            base_edge = Point2f(-lead_y, lead_x);
            break;
        default:
            LOG_ERR("main_element should be 0, 1, 2 or 3");
            break;
        }
        /* change base point of main edge */
        diagonal_pts[main_element] += 1;
        if (diagonal_pts[main_element] == n) {
            diagonal_pts[main_element] = 0;
        }
        // calculate the width of the rotated min rectangle.
        // formula : width = vertor[left -> right] . vector[base_edge]
        // formula : height = vertor[bottom -> top] . vector[base_edge rotate 90 degree counterclockwise]
        Point2f vector_left_to_right(
                points[diagonal_pts[1]].x() - points[diagonal_pts[3]].x(),
                points[diagonal_pts[1]].y() - points[diagonal_pts[3]].y());
        float width = dot_product<float>(vector_left_to_right, base_edge);

        Point2f vector_bottom_to_top(
                points[diagonal_pts[2]].x() - points[diagonal_pts[0]].x(),
                points[diagonal_pts[2]].y() - points[diagonal_pts[0]].y());
        Point2f vertical_base_edge = rotate_90_counterclockwise(base_edge);
        float height = dot_product<float>(vector_bottom_to_top, vertical_base_edge);
        // calculate rectangle aera
        float area = width * height;
        if (area <= minarea) {
            minarea = area;
            // update buffer
            buf_index_bottom_pt = diagonal_pts[0];
            buf_index_left_pt = diagonal_pts[3];
            buf_base_edge = base_edge;
            buf_width = width;
            buf_height = height;
        }
    }
    // another edge that is vertical to base edge (counterclockwise direction)
    Point2f buf_next_edge = rotate_90_counterclockwise(buf_base_edge);

    float C1 = dot_product<float>(buf_base_edge, points[buf_index_left_pt]);
    float C2 = dot_product<float>(buf_next_edge, points[buf_index_bottom_pt]);

    float px = C1 * buf_next_edge.y() - C2 * buf_base_edge.y();
    float py = buf_base_edge.x() * C2 - buf_next_edge.x() * C1;

    Point2f vector_a(buf_base_edge.x() * buf_width, buf_base_edge.y() * buf_width);
    Point2f vector_b(buf_next_edge.x() * buf_height, buf_next_edge.y() * buf_height);

    box->set_center_x(px + (vector_a.x() + vector_b.x()) * 0.5);
    box->set_center_y(py + (vector_a.y() + vector_b.y()) * 0.5);

    box->set_width(buf_width);
    box->set_height(buf_height);
    box->set_angle(atan2f(buf_base_edge.y(), buf_base_edge.x()));
    return;
}

/**
 * @brief Calculate the hull points of input points with specified range,
 * mainly refer to Sklansky's algothrm with the implement the OpenCV
 * @param[in] pts Input points address
 * @param[in] start Begin index of input points
 * @param[in] end End index of input points
 * @param[out] stack Final output stack index of hull points
 * @param[in] nsign The direction of points judgement, only support 1 or -1
 * @param[in] sign2 The direction of three points position
 * @return int32_t
 */
int32_t sklansky_1982(
        Point** pts,
        int start,
        int end,
        int* stack,
        int nsign,
        int sign2) {
    int step = end > start ? 1 : -1;
    int prev = start;
    int cur = prev + step;
    int next = cur + step;
    int stack_size = 3;
    if (start == end
            || (pts[start]->x() == pts[end]->x() && pts[start]->y() == pts[end]->y())) {
        stack[0] = start;
        return 1;
    }
    /*
           stack        index
        |  next  |  stack_size - 1
        |  cur   |  stack_size - 2
        |  pre   |  stack_size - 3
        |  ...   |  ...
     */
    stack[0] = prev;
    stack[1] = cur;
    stack[2] = next;
    end += step;
    while (next != end) {
        /* Vector a : pre_point -> current_point  */
        /* Vector b : current_point -> next_point  */
        Point a(pts[cur]->x() - pts[prev]->x(), pts[cur]->y() - pts[prev]->y());
        Point b(pts[next]->x() - pts[cur]->x(), pts[next]->y() - pts[cur]->y());

        if (sign(b.y()) != nsign) {
            /* judge the direction by the cross product of a and b */
            int64_t convex_flag = a.y() * b.x() - a.x() * b.y();
            if (sign(convex_flag) == sign2 && (a.x() != 0 || a.y() != 0)) {
                prev = cur;
                cur = next;
                next += step;
                stack[stack_size] = next;
                ++stack_size;
            } else {
                if (prev == start) {
                    cur = next;
                    stack[1] = cur;
                    next += step;
                    stack[2] = next;
                } else {
                    stack[stack_size - 2] = next;
                    cur = prev;
                    prev = stack[stack_size - 4];
                    --stack_size;
                }
            }
        } else {
            next +=step;
            stack[stack_size - 1] = next;
        }
    }

    return --stack_size;
}

struct ComparePointCoordinates {
    /**
     * @brief Get the point with the smallest abscissa
     * @param[in] first First input point
     * @param[in] second Second input point
     * @return true
     * @return false
     */
    bool operator()(const Point* first, const Point* second) const {
        if (first->x() != second->x()) {
            return first->x() < second->x();
        } else if (first->y() != second->y()) {
            return first->y() < second->y();
        } else {
            return false;
        }
    }
};

void convex_hull(
        std::vector<Point>& pts,
        std::vector<Point>& hull,
        bool clock_wise) {
    int num_pts = static_cast<int>(pts.size());
    if (num_pts == 0) {
        hull.clear();
        return;
    }

    std::vector<Point*> ptrs_of_points(num_pts);
    for (int i = 0; i < num_pts; i++) {
        ptrs_of_points[i] = &pts[i];
    }

    std::vector<int> stack(num_pts + 2);
    std::vector<int> hull_buf(num_pts);

    std::sort(ptrs_of_points.begin(), ptrs_of_points.begin() + num_pts, ComparePointCoordinates());

    int min_y_index = 0;
    int max_y_index = 0;

    for (int i = 1; i < num_pts; i++) {
        if (ptrs_of_points[i]->y() < ptrs_of_points[min_y_index]->y()) {
            min_y_index = i;
        }
        if (ptrs_of_points[i]->y() > ptrs_of_points[max_y_index]->y()) {
            max_y_index = i;
        }
    }

    int num_output = 0;
    if (ptrs_of_points[0]->x() == ptrs_of_points[num_pts - 1]->x()
            && ptrs_of_points[0]->y() == ptrs_of_points[num_pts - 1]->y()) {
        hull_buf[num_output++] = 0;
    } else {
        int* left_top_stack = stack.data();
        int left_top_count = sklansky_1982(ptrs_of_points.data(),
                0, max_y_index, left_top_stack, -1, 1);
        int* right_top_stack = left_top_stack + left_top_count;
        int right_top_count = sklansky_1982(ptrs_of_points.data(),
                num_pts - 1, max_y_index, right_top_stack, -1, -1);

        if (!clock_wise) {
            std::swap(left_top_stack, right_top_stack);
            std::swap(left_top_count, right_top_count);
        }

        for (int i = 0; i < left_top_count - 1; i++) {
            hull_buf[num_output++] = int(ptrs_of_points[left_top_stack[i]] - pts.data());
        }

        for (int i = right_top_count - 1; i > 0; i--) {
            hull_buf[num_output++] = int(ptrs_of_points[right_top_stack[i]] - pts.data());
        }

        int stop_index = right_top_count > 2 ?
                right_top_stack[1] : left_top_count > 2 ? left_top_stack[left_top_count - 2] : -1;

        int* left_bottom_stack = stack.data();
        int left_bottom_count = sklansky_1982(ptrs_of_points.data(),
                0, min_y_index, left_bottom_stack, 1, -1);
        int* right_bottom_stack = left_bottom_stack + left_bottom_count;
        int right_bottom_count = sklansky_1982(ptrs_of_points.data(),
                num_pts - 1, min_y_index, right_bottom_stack, 1, 1);

        if (clock_wise) {
            std::swap(right_bottom_stack, left_bottom_stack);
            std::swap(right_bottom_count, left_bottom_count);
        }

        if (stop_index >= 0) {
            int check_index = left_bottom_count > 2 ?
                    left_bottom_stack[1] : left_bottom_count + right_bottom_count > 2 ?
                    right_bottom_stack[2 - left_bottom_count] : -1;

            if (check_index == stop_index
                    || (check_index >= 0
                        && ptrs_of_points[check_index]->x() == ptrs_of_points[stop_index]->x()
                        && ptrs_of_points[check_index]->y() == ptrs_of_points[stop_index]->y())) {
                left_bottom_count = left_bottom_count < 2 ? left_bottom_count : 2;
                right_bottom_count = right_bottom_count < 2 ? right_bottom_count : 2;
            }
        }

        for (int i = 0; i < left_bottom_count - 1; i++) {
            hull_buf[num_output++] = int(ptrs_of_points[left_bottom_stack[i]] - pts.data());
        }

        for (int i = right_bottom_count - 1; i > 0; i--) {
            hull_buf[num_output++] = int(ptrs_of_points[right_bottom_stack[i]] - pts.data());
        }

        if (num_output >= 3) {
            int min_index = 0;
            int max_index = 0;
            int lt = 0;
            for (int i = 1; i < num_output; i++) {
                int index = hull_buf[i];
                lt += hull_buf[i - 1] < index;
                if (lt > 1 && lt <= i - 2) {
                    break;
                }
                if (index < hull_buf[min_index]) {
                    min_index = i;
                }
                if (index > hull_buf[max_index]) {
                    max_index = i;
                }
            }
            int mmdist = std::abs(max_index - min_index);
            if ((mmdist == 1 || mmdist == num_output - 1) && (lt <= 1 || lt >= num_output - 2)) {
                int ascending = (max_index + 1) % num_output == min_index;
                int i0 = ascending ? min_index : max_index;
                int j = i0;
                if (i0 > 0) {
                    int i = 0;
                    for (; i < num_output; i++) {
                        stack[i] = hull_buf[j];
                        int current_index = stack[i];
                        int next_j = j + 1 < num_output ? j + 1 : 0;
                        int next_index = hull_buf[next_j];
                        if (i < num_output - 1 && (ascending != (current_index < next_index))) {
                            break;
                        }
                        j = next_j;
                    }
                    if (num_output == i) {
                        memcpy(hull_buf.data(), stack.data(), num_output * sizeof(int));
                    }
                }
            }
        }
    }

    hull.resize(num_output);
    for (int i = 0; i < num_output; i++) {
        hull[i] = pts[hull_buf[i]];
    }

    return;
}

RotatedRect min_area_rect_common(std::vector<Point>& pts) {

    // First find the convex hull of the contour point set
    std::vector<Point> hull;
    convex_hull(pts, hull, false);

    // Then find the smallest circumscribed rectangle of the convex hull
    size_t num_points = hull.size();
    std::vector<Point2f> f_hull(num_points);
    for (size_t i = 0; i < num_points; i++) {
        f_hull[i] = Point2f(float(hull[i].x()), float(hull[i].y()));
    }
    RotatedRect box;
    if (num_points > 2) {
        rotating_calipers(f_hull.data(), static_cast<int>(num_points), &box);
    } else if (num_points == 2) {
        box.set_center_x((f_hull[0].x() + f_hull[1].x()) / 2.f);
        box.set_center_y((f_hull[0].y() + f_hull[1].y()) / 2.f);
        float dx = f_hull[1].x() - f_hull[0].x();
        float dy = f_hull[1].y() - f_hull[0].y();
        box.set_width(sqrtf(dx * dx + dy * dy));
        box.set_height(0);
        box.set_angle(atan2f(dy, dx));
    } else if (num_points == 1) {
        box.set_center(f_hull[0]);
    } else {
        // Do nothing
    }
    box.set_angle(static_cast<float>(box.angle() * 180 / FCV_PI));

    return box;
}

G_FCV_NAMESPACE1_END()
