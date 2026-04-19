#!/bin/bash

# a testing script that creates two virtual terminals and have them
# communicating using uart_conf

BLUE='\033[1;34m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
RED='\033[1;31m'
NC='\033[0m'

BINARY="./uart_conf.o"
PORT0="/tmp/ttyV0"
PORT1="/tmp/ttyV1"
PID_FILE="socat.pid"

# 1. Cleanup Function
cleanup() {
    echo -e "\n${YELLOW}[Cleaning up...]${NC}"
    if [ -f "$PID_FILE" ]; then
        kill $(cat "$PID_FILE") 2>/dev/null
        rm -f "$PID_FILE"
    fi
    rm -f "$PORT0" "$PORT1"
}
trap cleanup EXIT

# 2. Start Virtual UART
echo -e "${BLUE}[1/3] Starting virtual UART bridge...${NC}"
socat -d -d pty,link="$PORT0",raw,echo=0 pty,link="$PORT1",raw,echo=0 &
echo $! > "$PID_FILE"

# Wait for symlinks to appear
sleep 1

# 3. Prime the Mock Firmware
echo -e "${BLUE}[2/3] Priming mock firmware response...${NC}"
(sleep 1; echo -e "Acknowledge\n") > "$PORT1" &

# 4. Run the Validator
echo -e "${BLUE}[3/3] Running validator against $PORT0...${NC}"
if $BINARY "$PORT0"; then
    echo -e "${GREEN}✔  Validation Passed${NC}"
else
    echo -e "${RED}✘ Validation Failed${NC}"
fi
