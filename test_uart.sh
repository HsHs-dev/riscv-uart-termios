#!/bin/bash

# a testing script that creates two virtual terminals and have them
# communicating using `socat`

BLUE='\033[1;34m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
RED='\033[1;31m'
NC='\033[0m'

BINARY="./uart_conf.o"
PORT0="/tmp/ttyV0"
PORT1="/tmp/ttyV1"
PID_FILE="socat.pid"
RESULT_FILE="/tmp/uart_test_result"

# cleanup
cleanup() {
    echo -e "\n${YELLOW}[Cleaning up..]${NC}"
    if [ -f "$PID_FILE" ]; then
        kill $(cat "$PID_FILE") 2>/dev/null
        rm -f "$PID_FILE"
    fi
    rm -f "$PORT0" "$PORT1" "$RESULT_FILE"
}
trap cleanup EXIT

# start virtual uart
echo -e "${BLUE}[1/3] Starting virtual UART bridge..${NC}"
socat -d -d pty,link="$PORT0",raw,echo=0 pty,link="$PORT1",raw,echo=0 &
echo $! > "$PID_FILE"

# wait for symlinks to appear
sleep 1

# Initialize the result file which will have the state of the test
echo "timeout" > "$RESULT_FILE"

# Start a background listener to verify what the C program writes
echo -e "${BLUE}[2/3] Starting listener on $PORT1..${NC}"
(
    # read with a 2-second timeout so the script doesn't hang 
    if read -t 2 -r received < "$PORT1"; then
        if [ "$received" = "hello from the uart_conf program" ]; then
            echo "pass" > "$RESULT_FILE"
            echo -e "Acknowledge\n" > "$PORT1"
        else
            echo "wrong_data: $received" > "$RESULT_FILE"
        fi
    else
        echo "timeout" > "$RESULT_FILE"
    fi
) &
LISTENER_PID=$!

# run the checker
echo -e "${BLUE}[3/3] Running checker against $PORT0..${NC}"

# Run the binary and save the returned value
$BINARY "$PORT0"
EXIT_CODE=$?

# Wait for the background listener to finish
wait $LISTENER_PID 2>/dev/null

# Read the state from our subshell listener
STATE=$(cat "$RESULT_FILE")

# FINAL EVALUATION
if [ "$STATE" == "timeout" ]; then
    echo -e "${RED}Failed ❌ (Timeout: C program did not write to UART)${NC}"
elif [ "$STATE" != "pass" ]; then
    echo -e "${RED}Failed ❌ (Received unexpected data: $STATE)${NC}"
elif [ $EXIT_CODE -ne 0 ]; then
    echo -e "${RED}Failed ❌ (C program reported an internal error or timeout)${NC}"
else
    echo -e "${GREEN}Passed ✅ (TX and RX verified)${NC}"
fi
