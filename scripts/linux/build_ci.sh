#!/bin/bash
set -o pipefail

if [ $# -eq 1 ];then
    seqn=$1
else
    seqn=1
fi

current_dir=$(
  cd "$(dirname "$0")" || exit
  pwd
)
project_dir=$(
  cd "$(dirname "$0")"/../.. || exit
  pwd
)

echo "project dir: ${project_dir}"

function red_print() {
  local what=$*
  echo -e "\e[1;31m${what} \e[0m"
}

function green_print() {
  local what=$*
  echo -e "\e[1;32m${what} \e[0m"
}

function yellow_print() {
  local what=$*
  echo -e "\e[1;33m${what} \e[0m"
}

time=$(date "+%Y%m%d%H")

# param 1
function ci_build() {
  if [ -d ${project_dir}/linux-build ]; then
    rm -rf ${project_dir}/linux-build
  fi

  mkdir -p ${project_dir}/linux-build
  cd ${project_dir}/linux-build || exit
  set -x
  cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=./ \
    ${1}
  set +x
  if [ $? -ne 0 ]; then
    return 1
  fi

  core_num=$(grep -c "processor" /proc/cpuinfo)
  use_core=$(( core_num / 2 ))
  make -j${use_core}
  if [ $? -ne 0 ]; then
    return 1
  fi

  make install
  if [ $? -ne 0 ]; then
    return 1
  fi
  return 0
}

function ci_read {
  if [ $# -ne 1 ]; then
    temp=$*
    addition=${temp#*txt}
    #    green_print ${addition}
  fi
  cat ${1} | while read line; do
    green_print "begin build:  $addition $line"
    ci_build "$addition $line" 2>&1 | tee -a ${project_dir}/${time}-build-linux.log
    if [ $? -ne 0 ]; then
      red_print "build fail: $line"
      echo "build fail: $addition $line" >> ${project_dir}/${time}-error-linux.log
      return 1
    fi
    green_print "build success: $addition $line \n\n"
  done
}

# process
COMMON_FILE=${project_dir}/scripts/ci/ci_common.sh
if [ -f "${COMMON_FILE}" ];then
    source "${COMMON_FILE}"
fi
