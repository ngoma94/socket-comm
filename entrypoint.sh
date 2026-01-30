#!/bin/bash
set -e

echo "Container ID: $(hostname)"
echo ""
echo "Starting server..."
/build-native/server /tmp/socket &
SERVER_PID=$!

sleep 1

echo ""
echo "Running client tests..."
echo ""

/build-native/client /tmp/socket VERSION
/build-native/client /tmp/socket PING
/build-native/client /tmp/socket UNKNOWN

# Keep container alive while server runs
wait $SERVER_PID
