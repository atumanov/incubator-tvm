export TVM_HOME=$(readlink -f ~/git/SRTML/tvm)
export PYTHONPATH=$TVM_HOME/python:$TVM_HOME/topi/python:${PYTHONPATH}
export TVM_PATH=$TVM_HOME
export VTA_HW_PATH=$TVM_PATH/3rdparty/vta-hw
export PYTHONPATH=$TVM_HOME/vta/python:${PYTHONPATH}
