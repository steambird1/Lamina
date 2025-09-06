#!/bin/bash

# 获取当前脚本所在目录（scripts/目录）的绝对路径
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# 计算项目根目录（scripts/的父目录）
PROJECT_ROOT=$(cd "${SCRIPT_DIR}/.." &>/dev/null && pwd)

# 调试目录仍为根目录下的cmake-build-debug
DEBUG_DIR="${PROJECT_ROOT}/cmake-build-debug"

# 1. 检查 Debug 目录是否存在
if [ ! -d "$DEBUG_DIR" ]; then
    echo "错误：Debug 目录不存在！路径：$DEBUG_DIR"
    echo "请确认 CMake Debug 编译成功，且路径配置正确。"
    exit 1
fi

# 2. 检查路径是否已在 PATH 中
echo "正在检查路径是否已在 PATH 中..."
if echo "$PATH" | grep -q "$DEBUG_DIR"; then
    echo "路径已存在于 PATH 中，无需重复添加：$DEBUG_DIR"
    exit 0
fi

# 3. 选择生效方式
echo "请选择 PATH 添加方式："
echo "1. 临时生效（仅当前终端，重启后失效）"
echo "2. 永久生效（所有新终端，需重启终端或 source 配置）"
read -p "输入选项（1/2）：" choice

case $choice in
    1)
        # 临时生效
        export PATH="$PATH:$DEBUG_DIR"
        echo "临时生效添加成功！当前终端可直接输入：myapp help"
        ;;
    2)
        # 永久生效
        if [ -n "$ZSH_VERSION" ]; then
            CONFIG_FILE="$HOME/.zshrc"
        elif [ -n "$BASH_VERSION" ]; then
            CONFIG_FILE="$HOME/.bashrc"
        else
            echo "错误：未识别的 shell（仅支持 bash/zsh），请手动添加路径到你的 shell 配置文件。"
            exit 1
        fi

        # 写入配置文件
        echo "export PATH=\"\$PATH:$DEBUG_DIR\"" >> "$CONFIG_FILE"
        echo "永久生效添加成功！配置文件：$CONFIG_FILE"
        echo "立即生效请执行：source $CONFIG_FILE"
        echo "后续新终端可直接输入：myapp help"
        ;;
    *)
        echo "错误：无效选项，请输入 1 或 2。"
        exit 1
        ;;
esac

exit 0