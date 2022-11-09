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

#include "modules/core/mat/interface/mat.h"
#include "modules/core/base/include/utils.h"
#include "modules/core/mat/include/mat_dot_common.h"

#ifdef HAVE_NEON
#include "modules/core/mat/include/mat_dot_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

double Mat::dot(Mat& m) const {
    if (m.empty()) {
        m = Mat(_width, _height, _type);
    }

    const int m_w = m.width();
    const int m_h = m.height();
    const int m_c = m.channels();

    if ((m_w != _width) || (m_h != _height) || (m_c != _channels)) {
        LOG_ERR("the size of dot-product operands should be the same!");
        return 0;
    }

#ifdef HAVE_NEON
    return dot_neon(*this, m);
#else
    return dot_common(*this, m);
#endif
}

G_FCV_NAMESPACE1_END()