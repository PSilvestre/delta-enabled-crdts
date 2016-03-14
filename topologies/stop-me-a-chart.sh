#!/bin/bash
# file: stop-me-a-chart.sh
kill -TERM `cat charts/.SimpleHttpServerPID`
