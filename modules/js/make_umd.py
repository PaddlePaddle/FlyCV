# Copyright (c) 2023 FlyCV Authors. All Rights Reserved.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

###############################################################################
#
#  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
#
#  By downloading, copying, installing or using the software you agree to this license.
#  If you do not agree to this license, do not download, install,
#  copy or use the software.
#
#
#                           License Agreement
#                For Open Source Computer Vision Library
#
# Copyright (C) 2013, OpenCV Foundation, all rights reserved.
# Third party copyrights are property of their respective owners.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#   * Redistribution's of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
#   * Redistribution's in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
#   * The name of the copyright holders may not be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
# This software is provided by the copyright holders and contributors "as is" and
# any express or implied warranties, including, but not limited to, the implied
# warranties of merchantability and fitness for a particular purpose are disclaimed.
# In no event shall the Intel Corporation or contributors be liable for any direct,
# indirect, incidental, special, exemplary, or consequential damages
# (including, but not limited to, procurement of substitute goods or services;
# loss of use, data, or profits; or business interruption) however caused
# and on any theory of liability, whether in contract, strict liability,
# or tort (including negligence or otherwise) arising in any way out of
# the use of this software, even if advised of the possibility of such damage.
#

###############################################################################
# AUTHOR: Sajjad Taheri, University of California, Irvine. sajjadt[at]uci[dot]edu
#
#                             LICENSE AGREEMENT
# Copyright (c) 2015, 2015 The Regents of the University of California (Regents)
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the
#    names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
###############################################################################

import os, sys, re, json, shutil
from subprocess import Popen, PIPE, STDOUT

PY3 = sys.version_info >= (3, 0)

def make_umd(flycvjs, cvjs):
    with open(flycvjs, 'r+b') as src:
        content = src.read()
    if PY3:  # content is bytes
        content = content.decode('utf-8')
    with open(cvjs, 'w+b') as dst:
        # inspired by https://github.com/umdjs/umd/blob/95563fd6b46f06bda0af143ff67292e7f6ede6b7/templates/returnExportsGlobal.js
        dst.write(("""
(function (root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define(function () {
      return (root.fcv = factory());
    });
  } else if (typeof module === 'object' && module.exports) {
    // Node. Does not work with strict CommonJS, but
    // only CommonJS-like environments that support module.exports,
    // like Node.
    module.exports = factory();
  } else if (typeof window === 'object') {
    // Browser globals
    root.fcv = factory();
  } else if (typeof importScripts === 'function') {
    // Web worker
    root.fcv = factory();
  } else {
    // Other shells, e.g. d8
    root.fcv = factory();
  }
}(this, function () {
  %s
  if (typeof Module === 'undefined')
    Module = {};
  return fcv(Module);
}));
        """ % (content)).lstrip().encode('utf-8'))


if __name__ == "__main__":
    if len(sys.argv) > 2:
        flycvjs = sys.argv[1]
        cvjs = sys.argv[2]
        if not os.path.isfile(flycvjs):
            print('flycv.js file not found! Have you compiled the flycvjs module?')
            exit()
        make_umd(flycvjs, cvjs);
