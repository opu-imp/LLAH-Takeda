#!/bin/bash
cd /Users/kawagoe/Develop/LLAH-Takeda
echo "=== BUILD ===" > test/debug_run.log
make >> test/debug_run.log 2>&1
echo "BUILD_EXIT:$?" >> test/debug_run.log
echo "=== RUN ===" >> test/debug_run.log
perl -e 'alarm 30; exec @ARGV' ./llahdoc -h test/db_data2/ test/query_images/query_doc005.jpg >> test/debug_run.log 2>&1
echo "RUN_EXIT:$?" >> test/debug_run.log
echo "=== DONE ===" >> test/debug_run.log
