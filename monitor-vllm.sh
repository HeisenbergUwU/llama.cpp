#!/usr/bin/env bash
#
# 监控 vLLM 容器 + GPU 状态 + 日志
# 用法: ./monitor-vllm.sh [container_name] [log_file] [port]
# 默认: container=vllm-248, log_file=/tmp/vllm-248.log, port=4444

set -euo pipefail

CONTAINER_NAME="${1:-vllm-248}"
LOG_FILE="${2:-/tmp/vllm-${CONTAINER_NAME}.log}"
PORT="${3:-4444}"
INTERVAL=2

GPU_AVAILABLE=true
GPU_DATA=""

check_gpu() {
    if command -v nvidia-smi &>/dev/null; then
        GPU_AVAILABLE=true
    else
        GPU_AVAILABLE=false
        echo "[WARN] nvidia-smi 不可用，跳过 GPU 监控"
        echo "        (当前系统: $(uname -s))"
    fi
}

get_gpu_data() {
    if [ "$GPU_AVAILABLE" = true ]; then
        nvidia-smi --query-gpu=index,temperature.gpu,memory.used,memory.total,utilization.gpu,power.draw --format=csv,noheader,nounits -i 0 2>/dev/null || GPU_DATA=""
    else
        GPU_DATA=""
    fi
}

get_throughput() {
    local tokens=0
    local prompts=0
    local successes=0
    local failures=0
    local avg_time=0
    local avg_ttft=0
    local avg_tpmt=0

    if curl -s --connect-timeout 2 "http://localhost:${PORT}/metrics" 2>/dev/null | grep -q 'vllm:'; then
        tokens=$(curl -s --connect-timeout 2 "http://localhost:${PORT}/metrics" 2>/dev/null \
            | grep 'vllm:prompt_tokens_total' \
            | tail -1 \
            | awk '{print $2}' \
            | cut -d'.' -f1 || echo "0")
        successes=$(curl -s --connect-timeout 2 "http://localhost:${PORT}/metrics" 2>/dev/null \
            | grep 'vllm:request_success_total' \
            | tail -1 \
            | awk '{print $2}' \
            | cut -d'.' -f1 || echo "0")
        avg_ttft=$(curl -s --connect-timeout 2 "http://localhost:${PORT}/metrics" 2>/dev/null \
            | grep 'vllm:time_to_first_token_seconds_mean' \
            | tail -1 \
            | awk '{print $2}' || echo "N/A")
        avg_tpmt=$(curl -s --connect-timeout 2 "http://localhost:${PORT}/metrics" 2>/dev/null \
            | grep 'vllm:time_per_output_token_seconds_mean' \
            | tail -1 \
            | awk '{print $2}' || echo "N/A")
    else
        echo -n "0 0 0 0 N/A N/A"
        return
    fi

    echo -n "${tokens:-0} ${prompts:-0} ${successes:-0} ${failures:-0} ${avg_ttft:-N/A} ${avg_tpmt:-N/A}"
}

container_running() {
    docker ps --format "{{.Names}}" 2>/dev/null | grep -qx "$CONTAINER_NAME"
}

print_header() {
    clear
    echo "=================================================================="
    echo "  vLLM 监控: ${CONTAINER_NAME}"
    echo "  端口: ${PORT} | 刷新: ${INTERVAL}s | $(date '+%Y-%m-%d %H:%M:%S')"
    echo "  日志: ${LOG_FILE}"
    echo "=================================================================="
}

print_gpu_info() {
    if [ "$GPU_AVAILABLE" = true ] && [ -n "$GPU_DATA" ]; then
        local gpu_info=$(echo "$GPU_DATA" | tr ',' ' ' | awk '{
            printf "GPU #%s | 温度: %s°C | 显存: %s/%s MB (%.1f%%) | 利用率: %s%% | 功耗: %.1f W\n",
                $1, $2, $3, $4, ($3/$4)*100, $5, $6
        }')
        printf "  %-65s\n" "$gpu_info"
    else
        printf "  %-65s\n" "GPU: 不可用 (需要 nvidia-smi)"
    fi
}

print_stats() {
    local tokens prompts successes failures avg_ttft avg_tpmt
    read -r tokens prompts successes failures avg_ttft avg_tpmt <<< "$(get_throughput)"

    printf "  Token 总量: %'s | 成功请求: %'s\n" "$tokens" "$successes"
    printf "  Avg TTFT:  %s s | Avg TPOT: %s s\n" \
        "$(echo "$avg_ttft" | awk '{printf "%.4f", $1}')" \
        "$(echo "$avg_tpmt" | awk '{printf "%.4f", $1}')"
}

print_container_status() {
    if container_running; then
        local status=$(docker inspect --format='{{.State.Status}}' "$CONTAINER_NAME" 2>/dev/null || echo "unknown")
        local since=$(docker inspect --format='{{.State.StartedAt}}' "$CONTAINER_NAME" 2>/dev/null | cut -d. -f1 || echo "N/A")
        printf "  容器状态: %s | 启动时间: %s\n" "$status" "$since"
    else
        printf "  容器状态: %s 未运行\n" "$CONTAINER_NAME"
    fi
}

show_tail() {
    if [ -f "$LOG_FILE" ]; then
        echo ""
        echo "  --- 最近 5 条日志 ---"
        tail -n 5 "$LOG_FILE" 2>/dev/null | sed 's/^/  /'
    else
        echo ""
        echo "  --- 日志文件不存在: ${LOG_FILE} ---"
        echo "  提示: 先用 'docker logs ${CONTAINER_NAME} > ${LOG_FILE} 2>&1 &' 导出日志"
    fi
}

cleanup() {
    echo ""
    echo "监控已退出。"
    exit 0
}

trap cleanup SIGINT SIGTERM

# 主循环
check_gpu
print_header
print_gpu_info
print_container_status
print_stats
show_tail

echo ""
echo "按 Ctrl+C 退出"
echo ""

while true; do
    sleep "$INTERVAL" &
    WAIT_PID=$!
    wait $WAIT_PID
    print_gpu_info
    print_container_status
    print_stats
done
