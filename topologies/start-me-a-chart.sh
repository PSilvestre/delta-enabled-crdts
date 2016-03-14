#!/bin/bash
# file: start-me-a-chart.sh
cd charts/
google-chrome http://localhost:3032 &
python -m SimpleHTTPServer 3032 &
echo $! > .SimpleHttpServerPID
