#!/bin/sh
if [ xmaple = xmaple ]; then
  echo "Skipping multiplication test (needs Maple, which is not installed)"
  exit 0
fi
cat /dev/null > integration/status.txt
maple -q integration/decomposeTest.mpl #| tee 2>&1 | tee -a integration/status.txt | egrep "total errors" | tr -d '"' | awk {'exit $1'}
